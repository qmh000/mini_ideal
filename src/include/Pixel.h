#ifndef SRC_GEOMETRY_PIXEL_H_
#define SRC_GEOMETRY_PIXEL_H_
#include <float.h>

#include "Point.h"

#define BOX_GLOBAL_MIN 100000.0
#define BOX_GLOBAL_MAX -100000.0
class box{
public:
	double low[2] = {BOX_GLOBAL_MIN,BOX_GLOBAL_MIN};
	double high[2] = {BOX_GLOBAL_MAX,BOX_GLOBAL_MAX};

	box(){}
	box (double lowx, double lowy, double highx, double highy);
};

enum PartitionStatus{
	OUT = 0,
	BORDER = 1,
	IN = 2
};

enum Direction{
	LEFT = 0,
	RIGHT = 1,
	TOP = 2,
	BOTTOM
};

enum cross_type{
	ENTER = 0,
	LEAVE = 1
};

class cross_info{
public:
	cross_type type;
	int edge_id;
	cross_info(cross_type t, int e){
		type = t;
		edge_id = e;
	}
};

// original
// class edge_range{
// public:
// 	int vstart = 0;
// 	int vend = 0;
// 	edge_range(int s, int e){
// 		vstart = s;
// 		vend = e;
// 	}
// 	int size(){
// 		return vend-vstart+1;
// 	}
// };

// modified
class Edge_seqs{
public:
	pair<uint16_t, uint8_t> *pos;

	Edge_seqs(){}
	~Edge_seqs();
	void init_edge_sequences(int num_edge_seqs);
	void add_edge(int id, int start, int end);
};


// original
// class Pixel:public box{
//     vector<cross_info> crosses;
// public:
// 	unsigned short id[2];
// 	PartitionStatus status = OUT;
// 	vector<edge_range> edge_ranges;
// 	vector<double> intersection_nodes[4];

// public:
// 	bool is_boundary(){
// 		return status == BORDER;
// 	}
// 	bool is_internal(){
// 		return status == IN;
// 	}
// 	bool is_external(){
// 		return status == OUT;
// 	}
// 	Pixel(){}
// 	void enter(double val, Direction d, int vnum);
// 	void leave(double val, Direction d, int vnum);
// 	void process_crosses(int num_edges);
// };

// modified
class Pixels{
public:
	uint8_t *status;
	// int *status;
	uint16_t *pointer;
	int num_border = 0;
	Pixels(){}
	Pixels(int num_vertices);
	~Pixels();
	void init_status(int size){memset(status, 0, size * sizeof(int));}
	void set_status(int id, PartitionStatus status);
	PartitionStatus show_status(int id);
	int get_num_pixels();
	int get_num_border();
	int get_num_sequences(int id);
	void add_edge(int id, int idx);
	void process_pixels_null(int x, int y);
	


};




#endif /* SRC_GEOMETRY_PIXEL_H_ */