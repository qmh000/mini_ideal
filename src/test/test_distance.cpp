#include "../include/MyPolygon.h"

int main(){
	query_context ctx;
	MyPolygon * poly = load_binary_file_single("/home/qmh/data/has_child.idl", ctx, 0);
    double x, y;
    cin >> x >> y;
	Point target(x, y);

    poly->rasterization(100);

	double dist = poly->distance(target, &ctx);

	cout << dist << endl;

	return 0;
}