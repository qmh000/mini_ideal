#include "../include/MyPolygon.h"

void Intersection_node::init_intersection_node(int num_nodes){
    node = new double[num_nodes];
}

void Intersection_node::add_node(int idx, double x){
    node[idx] = x;
}

Intersection_node::~Intersection_node(){
    delete node; 
}