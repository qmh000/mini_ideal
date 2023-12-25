#include "../include/MyPolygon.h"

int main (int argc, char **argv){
    int vpr = 10;

	query_context ctx;
    MyPolygon *poly_r = load_binary_file_single("/home/qmh/data/has_child.idl", ctx, 0);
    MyPolygon *poly_t = load_binary_file_single("/home/qmh/data/has_child.idl", ctx, 1);

    poly_r->rasterization(vpr);
    poly_t->rasterization(vpr);

	auto dist = poly_r->distance(poly_t, &ctx);
	
    cout << dist << endl;
    return 0;
}

