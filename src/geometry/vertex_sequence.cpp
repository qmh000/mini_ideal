#include "../include/MyPolygon.h"

VertexSequence::VertexSequence(int nv){
	p = new Point[nv];
	num_vertices = nv;
}

box *VertexSequence::getMBR(){
	box *mbr = new box();
	for(int i=0;i<num_vertices;i++){
		mbr->low[0] = min(mbr->low[0], p[i].x);
		mbr->high[0] = max(mbr->high[0], p[i].x);
		mbr->low[1] = min(mbr->low[1], p[i].y);
		mbr->high[1] = max(mbr->high[1], p[i].y);
	}
	return mbr;
}

size_t VertexSequence::encode(char *dest){
	assert(num_vertices>0);
	size_t encoded = 0;
	((long *)dest)[0] = num_vertices;
	encoded += sizeof(size_t);
	memcpy(dest+encoded,(char *)p,num_vertices*sizeof(Point));
	encoded += num_vertices*sizeof(Point);
	return encoded;
}

size_t VertexSequence::decode(char *source){
	size_t decoded = 0;
	num_vertices = ((size_t *)source)[0];
	assert(num_vertices>0);
	p = new Point[num_vertices];
	decoded += sizeof(size_t);
	memcpy((char *)p,source+decoded,num_vertices*sizeof(Point));
	decoded += num_vertices*sizeof(Point);
	return decoded;
}

double VertexSequence::distance(Point &point, bool geography) {
	return point_to_segment_sequence_distance(point, p, num_vertices, geography);
}

VertexSequence::~VertexSequence(){
	if(p){
		delete []p;
	}
}