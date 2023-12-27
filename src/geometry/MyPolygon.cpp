#include <string.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <sstream>
#include <vector>
#include <thread>
#include <unordered_map>

#include "../include/MyPolygon.h"

using namespace std;

MyPolygon::~MyPolygon(){
	// clear();
	delete raster;
	delete boundary;
	delete mbr;

}

void MyPolygon::rasterization(int vpr){
	assert(vpr>0);
	if(raster){
		return;
	}
	pthread_mutex_lock(&ideal_partition_lock);
	if(raster==NULL){
		raster = new MyRaster(boundary, vpr);
		raster->rasterization();
		// original ideal
		// raster->ideal_rasterization();
	}
	pthread_mutex_unlock(&ideal_partition_lock);
}

size_t MyPolygon::decode(char *source){
	size_t decoded = 0;
	assert(!boundary);
	boundary = new VertexSequence();
	size_t num_holes = ((size_t *)source)[0];
	decoded += sizeof(size_t);
	decoded += boundary->decode(source+decoded);
	for(size_t i=0;i<num_holes;i++){
		VertexSequence *vs = new VertexSequence();
		decoded += vs->decode(source+decoded);
	}
	return decoded;
}

size_t MyPolygon::encode(char *target){
	size_t encoded = 0;
	((size_t *)target)[0] = holes.size();
	encoded += sizeof(size_t); //saved one size_t for number of holes
	encoded += boundary->encode(target+encoded);
	for(VertexSequence *vs:holes){
		encoded += vs->encode(target+encoded);
	}
	return encoded;
}

void VertexSequence::print(bool complete_ring){
	cout<<"(";
	for(int i=0;i<num_vertices;i++){
		if(i!=0){
			cout<<",";
		}
		printf("%f ",p[i].x);
		printf("%f",p[i].y);
	}
	// the last vertex should be the same as the first one for a complete ring
	if(complete_ring){
		if(p[0].x!=p[num_vertices-1].x||p[0].y!=p[num_vertices-1].y){
			cout<<",";
			printf("%f ",p[0].x);
			printf("%f",p[0].y);
		}
	}
	cout<<")";
}

void MyPolygon::print_without_head(bool print_hole, bool complete_ring){
	assert(boundary);
	cout<<"(";

	boundary->print(complete_ring);
	if(print_hole){
		for(VertexSequence *vs:this->holes){
			cout<<", ";
			vs->print(complete_ring);
		}
	}
	cout<<")";
}

void MyPolygon::print(bool print_id, bool print_hole){
	if(print_id){
		cout<<"id:\t"<<this->id<<endl;
	}
	cout<<"POLYGON";
	print_without_head(print_hole);
	cout<<endl;
}

MyPolygon *MyPolygon::gen_box(double min_x,double min_y,double max_x,double max_y){
	MyPolygon *mbr = new MyPolygon();
	mbr->boundary = new VertexSequence(5);
	mbr->boundary->p[0].x = min_x;
	mbr->boundary->p[0].y = min_y;
	mbr->boundary->p[1].x = max_x;
	mbr->boundary->p[1].y = min_y;
	mbr->boundary->p[2].x = max_x;
	mbr->boundary->p[2].y = max_y;
	mbr->boundary->p[3].x = min_x;
	mbr->boundary->p[3].y = max_y;
	mbr->boundary->p[4].x = min_x;
	mbr->boundary->p[4].y = min_y;
	return mbr;
}

MyPolygon *MyPolygon::gen_box(box pix){
	return gen_box(pix.low[0],pix.low[1],pix.high[0],pix.high[1]);
}

void MyPolygon::clear(){
	if(boundary){
		delete boundary;
	}
	for(VertexSequence *p:holes){
		if(p){
			delete p;
		}
	}
	holes.clear();
	if(mbr){
		delete mbr;
	}
	if(raster){
		delete raster;
	}
}






//original ideal
// void MyPolygon::ideal_rasterization(int vpr){
// 	assert(vpr>0);
// 	if(raster){
// 		return;
// 	}
// 	pthread_mutex_lock(&ideal_partition_lock);
// 	if(raster==NULL){
// 		raster = new MyRaster(boundary,vpr);
// 		raster->ideal_rasterization();
// 	}
// 	pthread_mutex_unlock(&ideal_partition_lock);
// }
