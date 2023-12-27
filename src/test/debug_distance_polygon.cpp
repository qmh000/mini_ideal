#include "../include/MyPolygon.h"

int main (int argc, char **argv){
    int vpr = 10;

	query_context ctx;
    MyPolygon *poly_r = load_binary_file_single("/home/qmh/data/has_child.idl", ctx, 0);
    MyPolygon *poly_t = load_binary_file_single("/home/qmh/data/sampled.target.idl", ctx, 0);

    poly_r->rasterization(vpr);
    poly_t->rasterization(vpr);

    // poly_r->print();
    // poly_r->get_rastor()->print();

    // cout << endl;

    // poly_t->print();
    // poly_t->get_rastor()->print();
    

	auto dist = poly_r->distance(poly_t, &ctx);
	
    cout << dist << endl;
    return 0;
}

