#include "../include/MyPolygon.h"
#include "../include/query_context.h"


int main(){
	query_context ctx;
	vector<MyPolygon *> source = load_binary_file("/home/qmh/data/all.target.idl",ctx);
	int sum = 0;
	for(int i = 0; i < source.size(); i ++){
		auto p = source[i];
		p->rasterization(100);
		// p->print();
		// p->get_rastor()->print();
		sum += p->get_rastor()->get_num_pixels(BORDER);
	}
    cout << "rasterization finished!" << endl;

	cout << sum << endl;
	
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
