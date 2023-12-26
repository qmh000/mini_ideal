#include "../include/MyPolygon.h"
#include <fstream>
#include "../index/RTree.h"
#include <queue>

RTree<MyPolygon *, double, 2, double> tree;

bool MySearchCallback(MyPolygon *poly, void* arg){
	query_context *ctx = (query_context *)arg;
	MyPolygon *target= (MyPolygon *)ctx->target;

    // query with rasterization
	timeval start = get_cur_time();
	ctx->found += poly->distance(target,ctx);

	if(ctx->collect_latency){
		int nv = target->get_num_vertices();
		if(nv<5000){
			nv = 100*(nv/100);
			ctx->report_latency(nv, get_time_elapsed(start));
		}
	}
	return true;
}

void *query(void *args){
	query_context *ctx = (query_context *)args;
	query_context *gctx = ctx->global_ctx;
	log("thread %d is started",ctx->thread_id);
	ctx->query_count = 0;

	while(ctx->next_batch(1)){
		for(int i=ctx->index;i<ctx->index_end;i++){
			if(!tryluck(gctx->sample_rate)){
				ctx->report_progress();
				continue;
			}
            MyPolygon *poly = gctx->target_polygons[i];
            ctx->target = (void *)poly;
            box *px = poly->getMBB();
			struct timeval query_start = get_cur_time();
			tree.Search(px->low, px->high, MySearchCallback, (void *)ctx);
//			if(gctx->source_polygons[i]->getid()==8){
//				gctx->source_polygons[i]->print(false, false);
//			}
			ctx->object_checked.execution_time += get_time_elapsed(query_start);
			ctx->report_progress();
		}
	}
	ctx->merge_global();
	return NULL;
}



int main(int argc, char** argv) {
	query_context global_ctx;
	global_ctx = get_parameters(argc, argv);
    global_ctx.geography = true;

	timeval start;
    global_ctx.source_polygons = load_binary_file(global_ctx.source_path.c_str(), global_ctx);
	start = get_cur_time();
    for(MyPolygon *p:global_ctx.source_polygons){
		tree.Insert(p->getMBB()->low, p->getMBB()->high, p);
	}
	logt("building R-Tree with %d nodes", start, global_ctx.source_polygons.size());

    global_ctx.target_polygons = load_binary_file(global_ctx.target_path.c_str(), global_ctx);
    global_ctx.target_num = global_ctx.target_polygons.size();
    start = get_cur_time();

    global_ctx.reset_stats();

	preprocess(&global_ctx);
	logt("preprocess the source data", start);

    start = get_cur_time();

    pthread_t threads[global_ctx.num_threads];
	query_context ctx[global_ctx.num_threads];
	for(int i=0;i<global_ctx.num_threads;i++){
		ctx[i] = query_context(global_ctx);
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



