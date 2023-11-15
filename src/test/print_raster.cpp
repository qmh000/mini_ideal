#include "../include/MyPolygon.h"
#include "../include/query_context.h"


int main(){
	query_context ctx;
	// vector<MyPolygon *> source = load_binary_file("/home/qmh/mini_ideal/src/has_child.idl",ctx);

	// for(MyPolygon *p:source){
	// 	if(p->boundary->num_vertices>10000){
	// 		p->rasterization(100);
	// 		// p->print();
	// 		p->get_rastor()->print();
	// 		sleep(1000);
	// 		return 0;
	// 	}
	// }

	
	MyPolygon *source = load_binary_file_single("/home/qmh/mini_ideal/src/has_child.idl",ctx,0);

	cout << source->boundary->num_vertices << endl;

	source->rasterization(100);
	source->get_rastor()->print();

	return 0;
}