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

class Grid_lines{
public:
	uint16_t *offset;
	double *intersection_nodes;

	size_t num_crosses = 0;

	Grid_lines(){}
	~Grid_lines();
	void init_grid_lines(int dimy);
	int get_num_nodes(int y);
	void init_intersection_node(int num_nodes);
	void add_node(int idx, double x);

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
	Grid_lines horizontal;
	Grid_lines vertical;

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
	size_t get_num_pixels();
	size_t get_num_pixels(PartitionStatus status);
	size_t get_num_crosses();
	int get_num_border_edge();
	int get_num_sequences(int id);
	int get_offset_x(double xval);
	int get_offset_y(double yval);
	double get_double_x(int x);
	double get_double_y(int y);
	int get_id(int x, int y);
	int get_x(int id);
	int get_y(int id);
	int count_intersection_nodes(Point &p);
	box get_pixel_box(int x, int y);
	Pixels* get_pixels(){return pixels;}
	int get_pixel_id(Point &p){return get_id(get_offset_x(p.x), get_offset_y(p.y));}
	void process_crosses(unordered_map<int, vector<cross_info>> edge_info);
	void process_intersection(map<int, vector<double>> edge_intersection, string direction);

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
	/*
	 * some query functions
	 */
	bool contain(Point &p, query_context *ctx, bool profile = true);
	/*
	 * some utility functions
	 */
	void print(bool print_id=true, bool print_hole=false);
	void print_without_head(bool print_hole = false, bool complete_ring = false);
	/*
	 * for filtering
	 */
	box *getMBB();
	void rasterization(int vertex_per_raster);

	size_t decode(char *source);
    size_t encode(char *target);
    inline int get_num_vertices(){
		if(!boundary){
			return 0;
		}
		return boundary->num_vertices;
	}
	MyRaster *get_rastor(){
		return raster;
	}
	size_t get_num_pixels(){
		if(raster){
			return raster->get_num_pixels();
		}
		return 0;
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

// utility functions
void preprocess(query_context *gctx);

// storage related functions
vector<MyPolygon *> load_binary_file(const char *path, query_context &ctx);
MyPolygon *load_binary_file_single(const char *path, query_context ctx, int idx);
size_t load_points_from_path(const char *path, Point **points);

#endif /* SRC_MYPOLYGON_H_ */