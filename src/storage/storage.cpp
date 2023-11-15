#include "../include/MyPolygon.h"

typedef struct{
	ifstream *infile;
	size_t offset;
	size_t poly_size;
	size_t load(char *buffer){
		infile->seekg(offset, infile->beg);
		infile->read(buffer, poly_size);
		return poly_size;
	}
}load_holder;

const size_t buffer_size = 10*1024*1024;

void *load_unit(void *arg){
	query_context *ctx = (query_context *)arg;
	vector<load_holder *> *jobs = (vector<load_holder *> *)ctx->target;
	vector<MyPolygon *> *global_polygons = (vector<MyPolygon *> *)ctx->target2;

	char *buffer = new char[buffer_size];
	vector<MyPolygon *> polygons;
	while(ctx->next_batch(1)){
		for(int i=ctx->index;i<ctx->index_end;i++){
			load_holder *lh = (*jobs)[i];
			ctx->global_ctx->lock();
			size_t poly_size = lh->load(buffer);
			ctx->global_ctx->unlock();
			size_t off = 0;
			while(off<poly_size){
				MyPolygon *poly = new MyPolygon();
				off += poly->decode(buffer+off);
				if(poly->get_num_vertices() >= 3){
					polygons.push_back(poly);
					poly->getMBB();
				}else{
					delete poly;
				}
			}
			ctx->report_progress(1);
		}
	}

	delete []buffer;
	ctx->global_ctx->lock();
	global_polygons->insert(global_polygons->end(), polygons.begin(), polygons.end());
	ctx->global_ctx->unlock();
	polygons.clear();
	return NULL;
}

vector<MyPolygon *> load_binary_file(const char *path, query_context &global_ctx){
	global_ctx.index = 0;
	global_ctx.index_end = 0;
	vector<MyPolygon *> polygons;
	if(!file_exist(path)){
		log("%s does not exist",path);
		exit(0);
	}
	struct timeval start = get_cur_time();

	ifstream infile;
	infile.open(path, ios::in | ios::binary);
	size_t num_polygons_infile = 0;
	infile.seekg(0, infile.end);
	
	infile.seekg(-sizeof(size_t), infile.end);
	infile.read((char *)&num_polygons_infile, sizeof(size_t));
	assert(num_polygons_infile>0 && "the file should contain at least one polygon");

	PolygonMeta *pmeta = new PolygonMeta[num_polygons_infile];
	infile.seekg(-sizeof(size_t)-sizeof(PolygonMeta)*num_polygons_infile, infile.end);
	infile.read((char *)pmeta, sizeof(PolygonMeta)*num_polygons_infile);
	// the last one is the end
	size_t num_polygons = min(num_polygons_infile, global_ctx.max_num_polygons);

	logt("loading %ld polygon from %s",start, num_polygons,path);
	// organizing tasks
	vector<load_holder *> tasks;
	size_t cur = 0;
	while(cur<num_polygons){
		size_t end = cur+1;
		while(end<num_polygons &&
				pmeta[end].offset - pmeta[cur].offset + pmeta[end].size < buffer_size){
			end++;
		}
		load_holder *lh = new load_holder();
		lh->infile = &infile;
		lh->offset = pmeta[cur].offset;
		if(end<num_polygons){
			lh->poly_size = pmeta[end].offset - pmeta[cur].offset;
		}else{
			lh->poly_size = pmeta[end-1].offset - pmeta[cur].offset + pmeta[end-1].size;
		}
		tasks.push_back(lh);
		cur = end;
	}

	logt("packed %ld tasks", start, tasks.size());

	global_ctx.target_num = tasks.size();
	pthread_t threads[global_ctx.num_threads];
	query_context myctx[global_ctx.num_threads];
	for(int i=0;i<global_ctx.num_threads;i++){
		myctx[i].index = 0;
		myctx[i] = global_ctx;
		myctx[i].thread_id = i;
		myctx[i].global_ctx = &global_ctx;
		myctx[i].target = (void *)&tasks;
		myctx[i].target2 = (void *)&polygons;
	}
	for(int i=0;i<global_ctx.num_threads;i++){
		pthread_create(&threads[i], NULL, load_unit, (void *)&myctx[i]);
	}

	for(int i = 0; i < global_ctx.num_threads; i++ ){
		void *status;
		pthread_join(threads[i], &status);
	}
	infile.close();
	delete []pmeta;
	for(load_holder *lh:tasks){
		delete lh;
	}
	logt("loaded %ld polygons", start, polygons.size());
	return polygons;
}

MyPolygon *load_binary_file_single(const char *path, query_context ctx, int idx)
{
	ifstream infile;
	infile.open(path, ios::in | ios::binary);

	size_t num_polygons_infile;
	infile.seekg(-sizeof(size_t), infile.end);
	infile.read((char *)&num_polygons_infile, sizeof(size_t));
	assert(idx < num_polygons_infile && "the idx must smaller than the polygon number ");

	PolygonMeta pmeta;
	infile.seekg(-sizeof(size_t) - sizeof(PolygonMeta) * (num_polygons_infile - idx), infile.end);
	infile.read((char *)&pmeta, sizeof(PolygonMeta));

	char *buffer = new char[pmeta.size];

	infile.seekg(pmeta.offset, infile.beg);
	infile.read(buffer, pmeta.size);

	MyPolygon *poly = new MyPolygon();
	poly->decode(buffer);

	delete[] buffer;
	infile.close();
	return poly;
}