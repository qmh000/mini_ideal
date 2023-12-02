/*
 * preprocessing.cpp
 *
 *  Created on: May 6, 2022
 *      Author: teng
 */

#include "../include/MyPolygon.h"

/**
 *
 * the multi-thread functions for preprocessing the data
 *
 *
 * */

void *rasterization_unit(void *args){
	query_context *ctx = (query_context *)args;
	query_context *gctx = ctx->global_ctx;

	vector<MyPolygon *> &polygons = *(vector<MyPolygon *> *)gctx->target;

	//log("thread %d is started",ctx->thread_id);
	int local_count = 0;
	while(ctx->next_batch(10)){
		for(int i=ctx->index;i<ctx->index_end;i++){
			struct timeval start = get_cur_time();
			polygons[i]->rasterization(ctx->vpr);
			double latency = get_time_elapsed(start);
			int num_vertices = polygons[i]->get_num_vertices();
			//ctx->report_latency(num_vertices, latency);
//			if(latency>10000||num_vertices>200000){
//				logt("partition %d vertices (source)",start,num_vertices);
//			}
			ctx->report_progress();
		}
	}
	ctx->merge_global();
	return NULL;
}

void process_rasterization(query_context *gctx){

	log("start rasterizing the referred polygons");
	vector<MyPolygon *> &polygons = *(vector<MyPolygon *> *)gctx->target;
	assert(polygons.size()>0);
	gctx->index = 0;
	size_t former = gctx->target_num;
	gctx->target_num = polygons.size();

	struct timeval start = get_cur_time();
	pthread_t threads[gctx->num_threads];
	query_context ctx[gctx->num_threads];
	for(int i=0;i<gctx->num_threads;i++){
		ctx[i] = *gctx;
		ctx[i].thread_id = i;
		ctx[i].global_ctx = gctx;
	}

	for(int i=0;i<gctx->num_threads;i++){
		pthread_create(&threads[i], NULL, rasterization_unit, (void *)&ctx[i]);
	}

	for(int i = 0; i < gctx->num_threads; i++ ){
		void *status;
		pthread_join(threads[i], &status);
	}

	// //collect partitioning status
	size_t num_partitions = 0;
	size_t num_crosses = 0;
	size_t num_border_partitions = 0;
	size_t num_edges = 0;
	for(MyPolygon *poly:polygons){
		num_partitions += poly->get_rastor()->get_num_pixels();
		num_crosses += poly->get_rastor()->get_num_crosses();
		num_border_partitions += poly->get_rastor()->get_num_pixels(BORDER);
		num_edges += poly->get_rastor()->get_num_border_edge();
	}
	logt("IDEALized %d polygons with (%ld)%ld average pixels %.2f average crosses per pixel %.2f edges per pixel", start,
			polygons.size(),
			num_border_partitions/polygons.size(),
			num_partitions/polygons.size(),
			1.0*num_crosses/num_border_partitions,
			1.0*num_edges/num_border_partitions);

	gctx->index = 0;
	gctx->query_count = 0;
	gctx->target_num = former;
}

// the entry function
void preprocess(query_context *gctx){

	vector<MyPolygon *> target_polygons;
	target_polygons.insert(target_polygons.end(), gctx->source_polygons.begin(), gctx->source_polygons.end());
	target_polygons.insert(target_polygons.end(), gctx->target_polygons.begin(), gctx->target_polygons.end());
	gctx->target = (void *)&target_polygons;
	if(gctx->use_grid){
		process_rasterization(gctx);
	}
	target_polygons.clear();
	gctx->target = NULL;
}


