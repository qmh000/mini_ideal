#include "../include/MyPolygon.h"

void Grid_lines::init_grid_lines(int len){
    offset = new uint16_t[len];
    memset(offset, 0, sizeof(uint16_t) * (len));
}

int Grid_lines::get_num_nodes(int y){
    return offset[y + 1] - offset[y];
}

void Grid_lines::init_intersection_node(int num_nodes){
    intersection_nodes = new double[num_nodes];
}

void Grid_lines::add_node(int idx, double x){
    intersection_nodes[idx] = x;
}

Grid_lines::~Grid_lines(){
    delete []offset;
    delete []intersection_nodes;
}

