#include <queue>
#include <fstream>
#include "../include/MyPolygon.h"
#include "../index/RTree.h"

// some shared parameters

RTree<MyPolygon *, double, 2, double> tree;

bool MySearchCallback(MyPolygon *poly, void* arg){
	query_context *ctx = (query_context *)arg;

	struct timeval start = get_cur_time();

	ctx->found += poly->contain(*(Point *)ctx->target, ctx);

	double timepassed = get_time_elapsed(start);
	if(ctx->collect_latency){
		int nv = poly->get_num_vertices();
		if(nv<5000){
			nv = 100*(nv/100);
			ctx->report_latency(nv, timepassed);
		}
	}
	return true;
}

void *query(void *args){
	query_context *ctx = (query_context *)args;
	query_context *gctx = ctx->global_ctx;
	//log("thread %d is started",ctx->thread_id);
	char point_buffer[200];

	while(ctx->next_batch(100)){
		for(int i=ctx->index;i<ctx->index_end;i++){
			if(!tryluck(ctx->sample_rate)){
				ctx->report_progress();
				continue;
			}
			struct timeval start = get_cur_time();
			ctx->target = (void *)&gctx->points[i];
			tree.Search((double *)(gctx->points+i), (double *)(gctx->points+i), MySearchCallback, (void *)ctx);
			ctx->object_checked.execution_time += ::get_time_elapsed(start);
			ctx->report_progress();
		}
	}
	ctx->merge_global();

	return NULL;
}



int main(int argc, char** argv) {

	query_context global_ctx;
	global_ctx = get_parameters(argc, argv);
	global_ctx.query_type = QueryType::contain;

	global_ctx.source_polygons = load_binary_file(global_ctx.source_path.c_str(),global_ctx);

	preprocess(&global_ctx);

	timeval start = get_cur_time();
	for(MyPolygon *p:global_ctx.source_polygons){
		tree.Insert(p->getMBB()->low, p->getMBB()->high, p);
	}
	logt("building R-Tree with %d nodes", start, global_ctx.source_polygons.size());

	// read all the points
	global_ctx.load_points();


	start = get_cur_time();
	pthread_t threads[global_ctx.num_threads];
	query_context ctx[global_ctx.num_threads];
	for(int i=0;i<global_ctx.num_threads;i++){
		ctx[i] = global_ctx;
		ctx[i].thread_id = i;
		ctx[i].global_ctx = &global_ctx;
	}
	
	for(int i=0;i<global_ctx.num_threads;i++){
		pthread_create(&threads[i], NULL, query, (void *)&ctx[i]);
	}

	for(int i = 0; i < global_ctx.num_threads; i++ ){
		void *status;
		pthread_join(threads[i], &status);
	}
	global_ctx.print_stats();
	logt("total query",start);


	return 0;
}

// int main(int argc, char** argv) {
// 	query_context ctx;

// 	MyPolygon *poly = load_binary_file_single("/home/qmh/data/has_child.idl", ctx, 0);

// 	poly->rasterization(100);

// 	cout << poly->get_rastor()->get_num_pixels() << endl;
// 	cout << poly->get_rastor()->get_num_crosses() << endl;
// 	cout <<	poly->get_rastor()->get_num_pixels(BORDER) << endl;
// 	cout << poly->get_rastor()->get_num_border_edge() << endl;

// 	cout << poly->boundary->num_vertices << endl;

// 	return 0;
// }