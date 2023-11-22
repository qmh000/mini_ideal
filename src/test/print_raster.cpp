#include "../include/MyPolygon.h"
#include "../include/query_context.h"


int main(){
	query_context ctx;
	vector<MyPolygon *> source = load_binary_file("/home/qmh/mini_ideal/src/has_child.idl",ctx);
	
	for(int i = 0; i < source.size(); i ++){
		source[i]->rasterization(100);
		source[i]->print();
		source[i]->get_rastor()->print();
	}
    cout << "rasterization finished!" << endl;

	sleep(1000);
	// MyPolygon *p0 = load_binary_file_single("/home/qmh/mini_ideal/src/has_child.idl",ctx1,0);
	// p0->rasterization(100);
	
	// MyPolygon *p1 = load_binary_file_single("/home/qmh/mini_ideal/src/has_child.idl",ctx2,1);
	// // p1->print();
	// p1->rasterization(100);

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
	

	// int input;
	// cin >> input;
	// MyPolygon *source = load_binary_file_single("/home/qmh/mini_ideal/src/has_child.idl",ctx,input);

	// cout << source->boundary->num_vertices << endl;

	// source->rasterization(100);
	// source->print();
	// source->get_rastor()->print();

	return 0;
}
