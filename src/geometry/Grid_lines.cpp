#include "../include/MyPolygon.h"

void Grid_lines::init_grid_lines(int dimy){
    horizontal = new uint16_t[dimy + 1];
}

int Grid_lines::get_num_nodes(int y){
    return horizontal[y + 1] - horizontal[y];
}

Grid_lines::~Grid_lines(){
    delete horizontal;
}