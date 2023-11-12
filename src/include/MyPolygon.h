#ifndef SRC_MYPOLYGON_H_
#define SRC_MYPOLYGON_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <math.h>
#include <stack>
#include <map>
#include <bits/stdc++.h>

#include "util.h"
#include "Pixel.h"
#include "Point.h"
#include "query_context.h"

using namespace std;

typedef struct PolygonMeta_{
	uint size; // size of the polygon in bytes
	uint num_vertices; // number of vertices in the boundary (hole excluded)
	size_t offset; // the offset in the file
	box mbr; // the bounding boxes
} PolygonMeta;

class Intersection_node{
public:
	int idx = 0;
	double *node;

	Intersection_node(){}
	~Intersection_node();
	void init_intersection_node(int num_nodes);
	void add_node(int idx, double x);
};

class Grid_lines{
public:
	uint16_t *horizontal;
	
	Grid_lines(){}
	~Grid_lines();
	void init_grid_lines(int dimy);
	int get_num_nodes(int y);
};

class VertexSequence{
public:
	int num_vertices = 0;
	Point *p = NULL;

	VertexSequence(){};
	VertexSequence(int nv);
	~VertexSequence();
	box *getMBR();
	size_t encode(char *dest);
	size_t decode(char *source);
	void print(bool complete_ring=false);
};

class MyRaster{
	box *mbr = NULL;
	VertexSequence *vs = NULL;
	// original
	// vector<vector<Pixel *>> pixels;
	
	// modified
	Pixels *pixels = NULL;
	Grid_lines grid_lines;
	Edge_seqs edge_sequences;
	Intersection_node intersection_node_lists;


	double step_x = 0.0;
	double step_y = 0.0;
	int dimx = 0;
	int dimy = 0;
	void init_pixels();
	void evaluate_edges();
	void scanline_reandering();
public:
    MyRaster(VertexSequence *vs, int epp);
	void rasterization();
	void print();
	int get_id(int x, int y);
	int get_x(int id);
	int get_y(int id);
	box get_pixel_box(int x, int y);
	void process_crosses(unordered_map<int, vector<cross_info>> edge_info);
	void process_intersection(unordered_map<int, vector<double>> edge_intersection);

	~MyRaster();
};

class MyPolygon{
	size_t id = 0;

    box *mbr = NULL;
    MyRaster *raster = NULL;
    pthread_mutex_t ideal_partition_lock;
public:
    VertexSequence *boundary = NULL;
    vector<VertexSequence *> holes;
    MyPolygon(){
        pthread_mutex_init(&ideal_partition_lock, NULL);
    }
    ~MyPolygon();
    void clear();
    size_t decode(char *source);
    size_t encode(char *target);
    inline int get_num_vertices(){
		if(!boundary){
			return 0;
		}
		return boundary->num_vertices;
	}
    box *getMBB();
    void rasterization(int vertex_per_raster);
	void print(bool print_id=true, bool print_hole=false);
	void print_without_head(bool print_hole = false, bool complete_ring = false);
	MyRaster *get_rastor(){
		return raster;
	}
	static MyPolygon *gen_box(double minx,double miny,double maxx,double maxy);
	static MyPolygon *gen_box(box pix);
};

class MyMultiPolygon{
	vector<MyPolygon *> polygons;
public:
	MyMultiPolygon(){};
	~MyMultiPolygon();
	int num_polygons(){
		return polygons.size();
	}
	void print();

	size_t insert_polygon(MyPolygon *p){
		polygons.push_back(p);
		return polygons.size();
	}
	size_t insert_polygon(vector<MyPolygon *> ps){
		for(MyPolygon *p:ps){
			polygons.push_back(p);
		}
		return polygons.size();
	}
};


vector<MyPolygon *> load_binary_file(const char *path, query_context &ctx);

#endif /* SRC_MYPOLYGON_H_ */