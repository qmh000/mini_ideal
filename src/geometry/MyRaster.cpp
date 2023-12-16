#include "../include/MyPolygon.h"

MyRaster::MyRaster(VertexSequence *vst, int epp){
	assert(epp>0);
	vs = vst;
	mbr = vs->getMBR();

	double multi = abs((mbr->high[1]-mbr->low[1])/(mbr->high[0]-mbr->low[0]));
	dimx = std::pow((vs->num_vertices/epp)/multi,0.5);
	dimy = dimx*multi;

	if(dimx==0){
		dimx = 1;
	}
	if(dimy==0){
		dimy = 1;
	}

	step_x = (mbr->high[0]-mbr->low[0])/dimx;
	step_y = (mbr->high[1]-mbr->low[1])/dimy;

	if(step_x<0.00001){
		step_x = 0.00001;
		dimx = (mbr->high[0]-mbr->low[0])/step_x+1;
	}
	if(step_y<0.00001){
		step_y = 0.00001;
		dimy = (mbr->high[1]-mbr->low[1])/step_y+1;
	}
}

// original
// void MyRaster::init_pixels(){
// 	assert(mbr);
// 	const double start_x = mbr->low[0];
// 	const double start_y = mbr->low[1];
// 	for(double i=0;i<=dimx;i++){
// 		vector<Pixel *> v;
// 		for(double j=0;j<=dimy;j++){
// 			Pixel *m = new Pixel();
// 			m->id[0] = i;
// 			m->id[1] = j;
// 			m->low[0] = i*step_x+start_x;
// 			m->high[0] = (i+1.0)*step_x+start_x;
// 			m->low[1] = j*step_y+start_y;
// 			m->high[1] = (j+1.0)*step_y+start_y;
// 			v.push_back(m);
// 		}
// 		pixels.push_back(v);
// 	};
// }

// modified
void MyRaster::init_pixels(){
	assert(mbr);
	pixels = new Pixels((dimx+1)*(dimy+1));
	pixels->init_status((dimx+1)*(dimy+1) / 4 + 1);
	// pixels->init_status((dimx+1)*(dimy+1));
	horizontal.init_grid_lines(dimy + 1);
	vertical.init_grid_lines(dimx + 1);
}

void MyRaster::evaluate_edges(){
	// modified(add)
	multimap<int, vector<double>> horizontal_intersect_info;
	multimap<int, vector<double>> vertical_intersect_info;
	multimap<int, vector<cross_info>> edges_info;

	// normalize
	assert(mbr);
	const double start_x = mbr->low[0];
	const double start_y = mbr->low[1];

	for(int i=0;i<vs->num_vertices-1;i++){
		double x1 = vs->p[i].x;
		double y1 = vs->p[i].y;
		double x2 = vs->p[i+1].x;
		double y2 = vs->p[i+1].y;

		int cur_startx = (x1-start_x)/step_x;
		int cur_endx = (x2-start_x)/step_x;
		int cur_starty = (y1-start_y)/step_y;
		int cur_endy = (y2-start_y)/step_y;

		if(cur_startx==dimx+1){
			cur_startx--;
		}
		if(cur_endx==dimx+1){
			cur_endx--;
		}

		int minx = min(cur_startx,cur_endx);
		int maxx = max(cur_startx,cur_endx);

		if(cur_starty==dimy+1){
			cur_starty--;
		}
		if(cur_endy==dimy+1){
			cur_endy--;
		}
		// todo should not happen for normal cases
		if(cur_startx>dimx||cur_endx>dimx||cur_starty>dimy||cur_endy>dimy){
			cout<<"xrange\t"<<cur_startx<<" "<<cur_endx<<endl;
			cout<<"yrange\t"<<cur_starty<<" "<<cur_endy<<endl;
			printf("xrange_val\t%f %f\n",(x1-start_x)/step_x, (x2-start_x)/step_x);
			printf("yrange_val\t%f %f\n",(y1-start_y)/step_y, (y2-start_y)/step_y);
			assert(false);
		}
		assert(cur_startx<=dimx);
		assert(cur_endx<=dimx);
		assert(cur_starty<=dimy);
		assert(cur_endy<=dimy);

		// original
		// pixels[cur_startx][cur_starty]->status = BORDER;
		// pixels[cur_endx][cur_endy]->status = BORDER;

		//modified

		// pixels->set_status(get_id(cur_startx, cur_starty), BORDER);
		// pixels->set_status(get_id(cur_endx, cur_endy), BORDER);

		//in the same pixel
		if(cur_startx==cur_endx&&cur_starty==cur_endy){
			continue;
		}

		if(y1==y2){
			//left to right
			if(cur_startx<cur_endx){
				for(int x=cur_startx;x<cur_endx;x++){
					// original
					// pixels[x][cur_starty]->leave(y1,RIGHT,i);
					// pixels[x+1][cur_starty]->enter(y1,LEFT,i);

					// modified
					if(!vertical_intersect_info.count(x + 1)){
						vector<double> v;
						v.push_back(y1);
						vertical_intersect_info.insert({x + 1, v});
					}else{
						auto iter = vertical_intersect_info.find(x + 1);
						iter.operator*().second.push_back(y1);
					}

					if(!edges_info.count(get_id(x, cur_starty))){
						vector<cross_info> v;
						v.push_back(cross_info(LEAVE, i));
						edges_info.insert({get_id(x, cur_starty), v});
					}else 
						edges_info.find(get_id(x, cur_starty)).operator*().second.push_back(cross_info(LEAVE, i));
					if(!edges_info.count(get_id(x + 1, cur_starty))){
						vector<cross_info> v;
						v.push_back(cross_info(ENTER, i));
						edges_info.insert({get_id(x + 1, cur_starty), v});
					}else 
						edges_info.find(get_id(x + 1, cur_starty)).operator*().second.push_back(cross_info(ENTER, i));
					pixels->set_status(get_id(x, cur_starty), BORDER);
					pixels->set_status(get_id(x+1, cur_starty), BORDER);
				}
			}else { // right to left
				for(int x=cur_startx;x>cur_endx;x--){
					// original
					// pixels[x][cur_starty]->leave(y1, LEFT,i);
					// pixels[x-1][cur_starty]->enter(y1, RIGHT,i);

					// modified
					if(!vertical_intersect_info.count(x)){
						vector<double> v;
						v.push_back(y1);
						vertical_intersect_info.insert({x, v});
					}else{
						auto iter = vertical_intersect_info.find(x);
						iter.operator*().second.push_back(y1);
					}

					if(!edges_info.count(get_id(x, cur_starty))){
						vector<cross_info> v;
						v.push_back(cross_info(LEAVE, i));
						edges_info.insert({get_id(x, cur_starty), v});
					}else 
						edges_info.find(get_id(x, cur_starty)).operator*().second.push_back(cross_info(LEAVE, i));
					if(!edges_info.count(get_id(x - 1, cur_starty))){
						vector<cross_info> v;
						v.push_back(cross_info(ENTER, i));
						edges_info.insert({get_id(x - 1, cur_starty), v});
					}else 
						edges_info.find(get_id(x - 1, cur_starty)).operator*().second.push_back(cross_info(ENTER, i));
					pixels->set_status(get_id(x, cur_starty), BORDER);
					pixels->set_status(get_id(x-1, cur_starty), BORDER);
				}
			}
		}else if(x1==x2){
			//bottom up
			if(cur_starty<cur_endy){
				for(int y=cur_starty;y<cur_endy;y++){
					// original
					// pixels[cur_startx][y]->leave(x1, TOP,i);
					// pixels[cur_startx][y+1]->enter(x1, BOTTOM,i);

					// modified
					if(!horizontal_intersect_info.count(y + 1)){
						vector<double> v;
						v.push_back(x1);
						horizontal_intersect_info.insert({y + 1, v});
					}else{
						auto iter = horizontal_intersect_info.find(y + 1);
						iter.operator*().second.push_back(x1);
					}
					if(!edges_info.count(get_id(cur_startx, y))){
						vector<cross_info> v;
						v.push_back(cross_info(LEAVE, i));
						edges_info.insert({get_id(cur_startx, y), v});
					}else 
						edges_info.find(get_id(cur_startx, y)).operator*().second.push_back(cross_info(LEAVE, i));
					if(!edges_info.count(get_id(cur_startx, y + 1))){
						vector<cross_info> v;
						v.push_back(cross_info(ENTER, i));
						edges_info.insert({get_id(cur_startx, y + 1), v});
					}else 
						edges_info.find(get_id(cur_startx, y + 1)).operator*().second.push_back(cross_info(ENTER, i));

					pixels->set_status(get_id(cur_startx, y), BORDER);
					pixels->set_status(get_id(cur_startx, y+1), BORDER);
				}
			}else { //border[bottom] down
				for(int y=cur_starty;y>cur_endy;y--){
					// original
					// pixels[cur_startx][y]->leave(x1, BOTTOM,i);
					// pixels[cur_startx][y-1]->enter(x1, TOP,i);

					// modified
					if(!horizontal_intersect_info.count(y)){
						vector<double> v;
						v.push_back(x1);
						horizontal_intersect_info.insert({y, v});
					}else{
						auto iter = horizontal_intersect_info.find(y);
						iter.operator*().second.push_back(x1);
					}

					if(!edges_info.count(get_id(cur_startx, y))){
						vector<cross_info> v;
						v.push_back(cross_info(LEAVE, i));
						edges_info.insert({get_id(cur_startx, y), v});
					}else 
						edges_info.find(get_id(cur_startx, y)).operator*().second.push_back(cross_info(LEAVE, i));
					if(!edges_info.count(get_id(cur_startx, y - 1))){
						vector<cross_info> v;
						v.push_back(cross_info(ENTER, i));
						edges_info.insert({get_id(cur_startx, y - 1), v});
					}else 
						edges_info.find(get_id(cur_startx, y - 1)).operator*().second.push_back(cross_info(ENTER, i));
					
					pixels->set_status(get_id(cur_startx, y), BORDER);
					pixels->set_status(get_id(cur_startx, y-1), BORDER);
				}
			}
		}else{
			// solve the line function
			double a = (y1-y2)/(x1-x2);
			double b = (x1*y2-x2*y1)/(x1-x2);

			int x = cur_startx;
			int y = cur_starty;
			while(x!=cur_endx||y!=cur_endy){
				bool passed = false;
				double yval = 0;
				double xval = 0;
				int cur_x = 0;
				int cur_y = 0;
				//check horizontally
				if(x!=cur_endx){
					if(cur_startx<cur_endx){
						xval = ((double)x+1)*step_x+start_x;
					}else{
						xval = (double)x*step_x+start_x;
					}
					yval = xval*a+b;
					cur_y = (yval-start_y)/step_y;
					// printf("y %f %d\n",(yval-start_y)/step_y,cur_y);
					if(cur_y>max(cur_endy, cur_starty)){
						cur_y=max(cur_endy, cur_starty);
					}
					if(cur_y<min(cur_endy, cur_starty)){
						cur_y=min(cur_endy, cur_starty);
					}
					if(cur_y==y){
						passed = true;
						// left to right
						if(cur_startx<cur_endx){
							// original
							// pixels[x++][y]->leave(yval,RIGHT,i);
							// pixels[x][y]->enter(yval,LEFT,i);

							// modified
							if(!vertical_intersect_info.count(x + 1)){
								vector<double> v;
								v.push_back(yval);
								vertical_intersect_info.insert({x + 1, v});
							}else{
								auto iter = vertical_intersect_info.find(x + 1);
								iter.operator*().second.push_back(yval);
							}

							pixels->set_status(get_id(x, y), BORDER);
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(LEAVE, i));
								edges_info.insert({get_id(x ++, y), v});
							}else 
								edges_info.find(get_id(x ++, y)).operator*().second.push_back(cross_info(LEAVE, i));
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(ENTER, i));
								edges_info.insert({get_id(x, y), v});
							}else 
								edges_info.find(get_id(x, y)).operator*().second.push_back(cross_info(ENTER, i));
							
							pixels->set_status(get_id(x, y), BORDER);
						}else{//right to left
							// original
							// pixels[x--][y]->leave(yval,LEFT,i);
							// pixels[x][y]->enter(yval,RIGHT,i);

							// modified
							if(!vertical_intersect_info.count(x)){
								vector<double> v;
								v.push_back(yval);
								vertical_intersect_info.insert({x, v});
							}else{
								auto iter = vertical_intersect_info.find(x);
								iter.operator*().second.push_back(yval);
							}

							pixels->set_status(get_id(x, y), BORDER);
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(LEAVE, i));
								edges_info.insert({get_id(x --, y), v});
							}else 
								edges_info.find(get_id(x --, y)).operator*().second.push_back(cross_info(LEAVE, i));
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(ENTER, i));
								edges_info.insert({get_id(x, y), v});
							}else 
								edges_info.find(get_id(x, y)).operator*().second.push_back(cross_info(ENTER, i));
							pixels->set_status(get_id(x, y), BORDER);
						}
					}
				}
				//check vertically
				if(y!=cur_endy){
					if(cur_starty<cur_endy){
						yval = (y+1)*step_y+start_y;
					}else{
						yval = y*step_y+start_y;
					}
					xval = (yval-b)/a;
					int cur_x = (xval-start_x)/step_x;
					//printf("x %f %d\n",(xval-start_x)/step_x,cur_x);
					if(cur_x>max(cur_endx, cur_startx)){
						cur_x=max(cur_endx, cur_startx);
					}
					if(cur_x<min(cur_endx, cur_startx)){
						cur_x=min(cur_endx, cur_startx);
					}
					if(cur_x==x){
						passed = true;
						if(cur_starty<cur_endy){// bottom up
							// original
							// pixels[x][y++]->leave(xval, TOP,i);
							// pixels[x][y]->enter(xval, BOTTOM,i);

							// modified
							if(!horizontal_intersect_info.count(y + 1)){
								vector<double> v;
								v.push_back(xval);
								horizontal_intersect_info.insert({y + 1, v});
							}else{
								auto iter = horizontal_intersect_info.find(y + 1);
								iter.operator*().second.push_back(xval);
							}

							pixels->set_status(get_id(x, y), BORDER);
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(LEAVE, i));
								edges_info.insert({get_id(x, y ++), v});
							}else 
								edges_info.find(get_id(x, y ++)).operator*().second.push_back(cross_info(LEAVE, i));
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(ENTER, i));
								edges_info.insert({get_id(x, y), v});
							}else 
								edges_info.find(get_id(x, y)).operator*().second.push_back(cross_info(ENTER, i));
							
							pixels->set_status(get_id(x, y), BORDER);				
						}else{// top down
							// original
							// pixels[x][y--]->leave(xval, BOTTOM,i);
							// pixels[x][y]->enter(xval, TOP,i);

							// modified
							if(!horizontal_intersect_info.count(y)){
								vector<double> v;
								v.push_back(xval);
								horizontal_intersect_info.insert({y, v});
							}else{
								auto iter = horizontal_intersect_info.find(y);
								iter.operator*().second.push_back(xval);
							}

							pixels->set_status(get_id(x, y), BORDER);
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(LEAVE, i));
								edges_info.insert({get_id(x, y --), v});
							}else 
								edges_info.find(get_id(x, y --)).operator*().second.push_back(cross_info(LEAVE, i));
							if(!edges_info.count(get_id(x, y))){
								vector<cross_info> v;
								v.push_back(cross_info(ENTER, i));
								edges_info.insert({get_id(x, y), v});
							}else 
								edges_info.find(get_id(x, y)).operator*().second.push_back(cross_info(ENTER, i));
							
							pixels->set_status(get_id(x, y), BORDER);
						}
					}
				}
				assert(passed);
			}
		}
	}


	// original
	// for(vector<Pixel *> &rows:pixels){
	// 	for(Pixel *p:rows){
	// 		for(int i=0;i<4;i++){
	// 			if(p->intersection_nodes[i].size()>0){
	// 				p->status = BORDER;
	// 				break;
	// 			}
	// 		}
	// 	}
	// }

	// modified
	// for(auto info : edges_info){
	// 	auto pix = info.first;
	// 	pixels->set_status(pix, BORDER);
	// }

	// 初始化edge_sequences和intersection nodes list;
	process_crosses(edges_info);
	process_intersection(horizontal_intersect_info, "horizontal");
	process_intersection(vertical_intersect_info, "vertical");
	pixels->process_pixels_null(dimx, dimy);

	// for(int i = 0; i <= dimx; i ++){
	// 	auto id = get_id(i, dimy);
	// 	pixels->set_status(id, OUT);
	// }

	// for(int i = 0; i <= dimy; i ++){
	// 	auto id = get_id(dimx, i);
	// 	pixels->set_status(id, OUT);
	// }

	
}

// original
// void MyRaster::scanline_reandering(){
// 	for(int y = 1;y < dimy;y ++){
// 		bool isin = false;
// 		for(int x = 0; x < dimx; x ++){
// 			if(pixels[x][y]->status!=BORDER){
// 				if(isin){
// 					pixels[x][y]->status = IN;
// 				}
// 				continue;
// 			}
// 			if(pixels[x][y]->intersection_nodes[BOTTOM].size()%2==1){
// 				isin = !isin;
// 			}
// 		}
// 	}
// }

// modified
void MyRaster::scanline_reandering(){
	const double start_x = mbr->low[0];
	const double start_y = mbr->low[1];

	for(int y = 1; y < dimy; y ++){
		bool isin = false;
		for(int x = 0; x < dimx; x ++){
			if(pixels->show_status(get_id(x, y)) != BORDER){
				if(isin){
					pixels->set_status(get_id(x, y), IN);
				}else{
					pixels->set_status(get_id(x, y), OUT);
				}
				continue;
			}
			int pass = 0;
			uint16_t i = horizontal.offset[y], j = horizontal.offset[y + 1];
			while(i < j && horizontal.intersection_nodes[i] <= start_x + step_x * (x + 1)){
				pass ++;
				i ++;
			}
			if(pass % 2 == 1) isin = true;
			else isin = false;

		}
	}
}


void MyRaster::rasterization(){

	//1. create space for the pixels
	init_pixels();

	//2. edge crossing to identify BORDER pixels
	evaluate_edges();

	//3. determine the status of rest pixels with scanline rendering
	scanline_reandering();
}

void MyRaster::print(){
	MyMultiPolygon *inpolys = new MyMultiPolygon();
	MyMultiPolygon *borderpolys = new MyMultiPolygon();
	MyMultiPolygon *outpolys = new MyMultiPolygon();

	for(int i=0;i<=dimx;i++){
		for(int j=0;j<=dimy;j++){
			box* bx = get_pixel_box(i, j);
			MyPolygon *m = MyPolygon::gen_box(*bx);
			delete bx;
			if(pixels->show_status(get_id(i, j)) == BORDER){
				borderpolys->insert_polygon(m);
			}else if(pixels->show_status(get_id(i, j)) == IN){
				inpolys->insert_polygon(m);
			}else if(pixels->show_status(get_id(i, j)) == OUT){
				outpolys->insert_polygon(m);
			}
		}
	}

	cout<<"border:" << borderpolys->num_polygons() <<endl;
	borderpolys->print();
	cout<<"in:"<< inpolys->num_polygons() << endl;
	inpolys->print();
	cout<<"out:"<< outpolys->num_polygons() << endl;
	outpolys->print();
	cout << endl;
	// allpolys->print();


	delete borderpolys;
	delete inpolys;
	delete outpolys;
}

int MyRaster::count_intersection_nodes(Point &p){
	// here we assume the point inside one of the pixel
	// Pixel *pix = get_pixel(p);
	int pix_id = get_pixel_id(p);
	// assert(pix->status==BORDER);
	assert(pixels->show_status(pix_id) == BORDER);
	int count = 0;
	// for(int i=0;i<=pix->id[1];i++){
	// 	for(double &node:pixels[pix->id[0]][i]->intersection_nodes[RIGHT]){
	// 		if(node<=p.y){
	// 			count++;
	// 		}
	// 	}
	// }
	int x = get_x(pix_id) + 1;
	uint16_t i = vertical.offset[x], j;
	if(x < dimx) j = vertical.offset[x + 1];
	else j = vertical.num_crosses;
	while(i < j && vertical.intersection_nodes[i] <= p.y){
		count ++;
		i ++;
	}
	return count;
}

box* MyRaster::get_pixel_box(int x, int y){
	const double start_x = mbr->low[0];
	const double start_y = mbr->low[1];

	double lowx = start_x + x * step_x;
	double lowy = start_y + y * step_y;
	double highx = start_x + (x + 1) * step_x;
	double highy = start_y + (y + 1) * step_y;

	box* ret = new box(lowx, lowy, highx, highy);
	return ret;
}

int MyRaster::get_offset_x(double xval){
	assert(mbr);
	assert(step_x>0.000000000001 && "the width per pixel must not be 0");
	int x = double_to_int((xval-mbr->low[0])/step_x);
	return min(max(x, 0), dimx);
}
// the range must be [0, dimy]
int MyRaster::get_offset_y(double yval){
	assert(mbr);
	assert(step_y>0.000000000001 && "the hight per pixel must not be 0");
	int y = double_to_int((yval-mbr->low[1])/step_y);
	return min(max(y, 0), dimy);
}

double MyRaster::get_double_x(int x){
	return mbr->low[0] + x * step_x;
}

double MyRaster::get_double_y(int y){
	return mbr->low[1] + y * step_y;
}

size_t MyRaster::get_num_crosses(){
	// *2是为了match原始IDEAL
	return (horizontal.num_crosses + vertical.num_crosses) * 2;
}

int MyRaster::get_num_border_edge(){
	// int num = 0;
	// for(vector<Pixel *> &rows:pixels){
	// 	for(Pixel *p:rows){
	// 		num += p->num_edges_covered();
	// 	}
	// }
	return vs->num_vertices;
}

uint16_t MyRaster::get_num_sequences(int id){
	if(pixels->show_status(id) != BORDER) return 0;
	return pixels->pointer[id + 1] - pixels->pointer[id];
}

size_t MyRaster::get_num_pixels(){
	return (dimx+1)*(dimy+1);
}

size_t MyRaster::get_num_pixels(PartitionStatus status){
	size_t num = 0;
	 
	// for(vector<Pixel *> &rows:pixels){
	// 	for(Pixel *p:rows){
	// 		if(p->status==status){
	// 			num++;
	// 		}
	// 	}
	// }

	for(int i = 0; i < get_num_pixels(); i ++){
		if(pixels->show_status(i) == status) 
			num ++;
	}
	return num;
}

int MyRaster::get_id(int x, int y){
	assert(x>=0&&x<=dimx);
	assert(y>=0&&y<=dimy);
	return y * (dimx+1) + x;
}

// from id to pixel x
int MyRaster::get_x(int id){
	return id % (dimx+1);
}

// from id to pixel y
int MyRaster::get_y(int id){
	assert((id / (dimx+1)) <= dimy);
	return id / (dimx+1);
}

int MyRaster::get_x(long long id){
	return id % (dimx+1);
}

// from id to pixel y
int MyRaster::get_y(long long id){
	assert((id / (dimx+1)) <= dimy);
	return id / (dimx+1);
}

int MyRaster::get_pixel_id(Point &p){
	int xoff = get_offset_x(p.x);
	int yoff = get_offset_y(p.y);
	assert(xoff <= dimx);
	assert(yoff <= dimy);
	return get_id(xoff, yoff);
}

// similar to the expand_radius function, get the possible minimum distance between point p and
// the target pixels which will be evaluated in this step, will be used as a stop sign
double MyRaster::get_possible_min(Point &p, int center, int step, bool geography){
	int core_x_low = get_x(center);
	int core_x_high = get_x(center);
	int core_y_low = get_y(center);
	int core_y_high = get_y(center);

	vector<int> needprocess;

	int ymin = max(0,core_y_low-step);
	int ymax = min(dimy,core_y_high+step);

	double mindist = DBL_MAX;
	//left scan
	if(core_x_low-step>=0){
		double x = get_pixel_box(core_x_low-step,ymin)->high[0];
		double y1 = get_pixel_box(core_x_low-step,ymin)->low[1];
		double y2 = get_pixel_box(core_x_low-step,ymax)->high[1];

		Point p1 = Point(x, y1);
		Point p2 = Point(x, y2);
		double dist = point_to_segment_distance(p, p1, p2, geography);
		mindist = min(dist, mindist);
	}
	//right scan
	if(core_x_high+step<=dimx){
		double x = get_pixel_box(core_x_high+step,ymin)->low[0];
		double y1 = get_pixel_box(core_x_high+step,ymin)->low[1];
		double y2 = get_pixel_box(core_x_high+step,ymax)->high[1];
		Point p1 = Point(x, y1);
		Point p2 = Point(x, y2);
		double dist = point_to_segment_distance(p, p1, p2, geography);
		mindist = min(dist, mindist);
	}

	// skip the first if there is left scan
	int xmin = max(0,core_x_low-step+(core_x_low-step>=0));
	// skip the last if there is right scan
	int xmax = min(dimx,core_x_high+step-(core_x_high+step<=dimx));
	//bottom scan
	if(core_y_low-step>=0){
		double y = get_pixel_box(xmin,core_y_low-step)->high[1];
		double x1 = get_pixel_box(xmin,core_y_low-step)->low[0];
		double x2 = get_pixel_box(xmax,core_y_low-step)->high[0];
		Point p1 = Point(x1, y);
		Point p2 = Point(x2, y);
		double dist = point_to_segment_distance(p, p1, p2, geography);
		mindist = min(dist, mindist);
	}
	//top scan
	if(core_y_high+step<=dimy){
		double y = get_pixel_box(xmin,core_y_low+step)->low[1];
		double x1 = get_pixel_box(xmin,core_y_low+step)->low[0];
		double x2 = get_pixel_box(xmax,core_y_low+step)->high[0];
		Point p1 = Point(x1, y);
		Point p2 = Point(x2, y);
		double dist = point_to_segment_distance(p, p1, p2, geography);
		mindist = min(dist, mindist);
	}
	return mindist;
}


vector<int> MyRaster::expand_radius(int center, int step){

	// int lowx = center->id[0];
	// int highx = center->id[0];
	// int lowy = center->id[1];
	// int highy = center->id[1];

	int lowx = get_x(center);
	int highx = get_x(center);
	int lowy = get_y(center);
	int highy = get_y(center);

	return expand_radius(lowx,highx,lowy,highy,step);
}

vector<int> MyRaster::expand_radius(int core_x_low, int core_x_high, int core_y_low, int core_y_high, int step){

	vector<int> needprocess;
	int ymin = max(0,core_y_low-step);
	int ymax = min(dimy, core_y_high+step);

	//left scan
	if(core_x_low-step>=0){
		for(int y=ymin;y<=ymax;y++){
			needprocess.push_back(get_id(core_x_low-step,y));
		}
	}
	//right scan
	if(core_x_high+step<=dimx){
		for(int y=ymin;y<=ymax;y++){
			needprocess.push_back(get_id(core_x_high+step,y));
		}
	}

	// skip the first if there is left scan
	int xmin = max(0,core_x_low-step+(core_x_low-step>=0));
	// skip the last if there is right scan
	int xmax = min(dimx,core_x_high+step-(core_x_high+step<=dimx));
	//bottom scan
	if(core_y_low-step>=0){
		for(int x=xmin;x<=xmax;x++){
			needprocess.push_back(get_id(x,core_y_low-step));
		}
	}
	//top scan
	if(core_y_high+step<=dimy){
		for(int x=xmin;x<=xmax;x++){
			needprocess.push_back(get_id(x,core_y_high+step));
		}
	}

	return needprocess;
}

MyRaster::~MyRaster(){
	// original
	// for(vector<Pixel *> &rows:pixels){
	// 	for(Pixel *p:rows){
	// 		delete p;
	// 	}
	// 	rows.clear();
	// }
	// pixels.clear();
	
	// modified
	// delete pixels;
}

vector<int> MyRaster::retrieve_pixels(box *target){

	vector<int> ret;
	int start_x = get_offset_x(target->low[0]);
	int start_y = get_offset_y(target->low[1]);
	int end_x = get_offset_x(target->high[0]);
	int end_y = get_offset_y(target->high[1]);

	//log("%d %d %d %d %d %d",dimx,dimy,start_x,end_x,start_y,end_y);
	for(int i=start_x;i<=end_x;i++){
		for(int j=start_y;j<=end_y;j++){
			ret.push_back(get_id(i , j));
		}
	}
	return ret;
}


void MyRaster::process_crosses(multimap<int, vector<cross_info>> edges_info){
	int num_edge_seqs = 0;
	// 可以写成lambda表达式
	for(auto ei : edges_info){
		num_edge_seqs += ei.second.size();
	}
	pixels->init_edge_sequences(num_edge_seqs);

	int idx = 0;
	for(auto info : edges_info){
		auto pix = info.first;
		auto crosses = info.second; 
		if(crosses.size() == 0) return;
		
		if(crosses.size() % 2 == 1){
			crosses.push_back(cross_info((cross_type)!crosses[crosses.size()-1].type, crosses[crosses.size()-1].edge_id));
		}
		
		assert(crosses.size()%2==0);

		// 根据crosses.size()，初始化
		int start = 0;
		int end = crosses.size() - 1;
		pixels->add_edge_offset(pix, idx);

		if(crosses[0].type == LEAVE){
			assert(crosses[end].type == ENTER);
			pixels->add_edge(idx ++, 0, crosses[0].edge_id);
			pixels->add_edge(idx ++, crosses[end].edge_id, vs->num_vertices - 2);
			start ++;
			end --;
		}

		for(int i = start; i <= end; i++){
			assert(crosses[i].type == ENTER);
			//special case, an ENTER has no pair LEAVE,
			//happens when one edge crosses the pair
			if(i == end || crosses[i + 1].type == ENTER){
				pixels->add_edge(idx ++, crosses[i].edge_id, crosses[i].edge_id);
			}else{
				pixels->add_edge(idx ++, crosses[i].edge_id, crosses[i+1].edge_id);
				i++;
			}
		}
	}
}

void MyRaster::process_intersection(multimap<int, vector<double>> intersection_info, string direction){
	int num_nodes = 0;
	for(auto i : intersection_info){
		num_nodes += i.second.size();
	}
	if(direction == "horizontal"){
		horizontal.init_intersection_node(num_nodes);
		horizontal.num_crosses = num_nodes;
		int idx = 0;
		for(auto info : intersection_info){
			auto h = info.first;
			auto nodes = info.second;
			
			sort(nodes.begin(), nodes.end());

			horizontal.offset[h] = idx;

			for(auto node : nodes){
				horizontal.add_node(idx, node);
				idx ++;
			}
		}
		horizontal.offset[dimy] = idx;
	}else{
		vertical.init_intersection_node(num_nodes);
		vertical.num_crosses = num_nodes;

		int idx = 0;
		for(auto info : intersection_info){
			auto h = info.first;
			auto nodes = info.second;
			
			sort(nodes.begin(), nodes.end());

			vertical.offset[h] = idx;

			for(auto node : nodes){
				vertical.add_node(idx, node);
				idx ++;
			}
		}
		vertical.offset[dimx] = idx;		
	}

}


















// original ideal

Pixel *MyRaster::get_pixel(Point &p){
	int xoff = get_offset_x(p.x);
	int yoff = get_offset_y(p.y);
	assert(xoff<=dimx);
	assert(yoff<=dimy);
	return ideal_pixels[xoff][yoff];
}

int MyRaster::ideal_count_intersection_nodes(Point &p){
	// here we assume the point inside one of the pixel
	Pixel *pix = get_pixel(p);
	assert(pix->status==BORDER);
	int count = 0;
	for(int i=0;i<=pix->id[1];i++){
		for(double &node:ideal_pixels[pix->id[0]][i]->intersection_nodes[RIGHT]){
			if(node<=p.y){
				count++;
			}
		}
	}
	return count;
}


void MyRaster::ideal_init_pixels(){
	assert(mbr);
	const double start_x = mbr->low[0];
	const double start_y = mbr->low[1];
	for(double i=0;i<=dimx;i++){
		vector<Pixel *> v;
		for(double j=0;j<=dimy;j++){
			Pixel *m = new Pixel();
			m->id[0] = i;
			m->id[1] = j;
			m->low[0] = i*step_x+start_x;
			m->high[0] = (i+1.0)*step_x+start_x;
			m->low[1] = j*step_y+start_y;
			m->high[1] = (j+1.0)*step_y+start_y;
			v.push_back(m);
		}
		ideal_pixels.push_back(v);
	};
}

void MyRaster::ideal_evaluate_edges(){
	// normalize
	assert(mbr);
	const double start_x = mbr->low[0];
	const double start_y = mbr->low[1];

	for(int i=0;i<vs->num_vertices-1;i++){
		double x1 = vs->p[i].x;
		double y1 = vs->p[i].y;
		double x2 = vs->p[i+1].x;
		double y2 = vs->p[i+1].y;

		int cur_startx = (x1-start_x)/step_x;
		int cur_endx = (x2-start_x)/step_x;
		int cur_starty = (y1-start_y)/step_y;
		int cur_endy = (y2-start_y)/step_y;

		if(cur_startx==dimx+1){
			cur_startx--;
		}
		if(cur_endx==dimx+1){
			cur_endx--;
		}

		int minx = min(cur_startx,cur_endx);
		int maxx = max(cur_startx,cur_endx);

		if(cur_starty==dimy+1){
			cur_starty--;
		}
		if(cur_endy==dimy+1){
			cur_endy--;
		}
		// todo should not happen for normal cases
		if(cur_startx>dimx||cur_endx>dimx||cur_starty>dimy||cur_endy>dimy){
			cout<<"xrange\t"<<cur_startx<<" "<<cur_endx<<endl;
			cout<<"yrange\t"<<cur_starty<<" "<<cur_endy<<endl;
			printf("xrange_val\t%f %f\n",(x1-start_x)/step_x, (x2-start_x)/step_x);
			printf("yrange_val\t%f %f\n",(y1-start_y)/step_y, (y2-start_y)/step_y);
			assert(false);
		}
		assert(cur_startx<=dimx);
		assert(cur_endx<=dimx);
		assert(cur_starty<=dimy);
		assert(cur_endy<=dimy);

		// pixels[cur_startx][cur_starty]->status = BORDER;
		// pixels[cur_endx][cur_endy]->status = BORDER;

		//in the same pixel
		if(cur_startx==cur_endx&&cur_starty==cur_endy){
			continue;
		}

		if(y1==y2){
			//left to right
			if(cur_startx<cur_endx){
				for(int x=cur_startx;x<cur_endx;x++){
					ideal_pixels[x][cur_starty]->leave(y1,RIGHT,i);
					ideal_pixels[x+1][cur_starty]->enter(y1,LEFT,i);
					ideal_pixels[x][cur_starty]->status = BORDER;
					ideal_pixels[x+1][cur_starty]->status = BORDER;
				}
			}else { // right to left
				for(int x=cur_startx;x>cur_endx;x--){
					ideal_pixels[x][cur_starty]->leave(y1, LEFT,i);
					ideal_pixels[x-1][cur_starty]->enter(y1, RIGHT,i);
					ideal_pixels[x][cur_starty]->status = BORDER;
					ideal_pixels[x-1][cur_starty]->status = BORDER;
				}
			}
		}else if(x1==x2){
			//bottom up
			if(cur_starty<cur_endy){
				for(int y=cur_starty;y<cur_endy;y++){
					ideal_pixels[cur_startx][y]->leave(x1, TOP,i);
					ideal_pixels[cur_startx][y+1]->enter(x1, BOTTOM,i);
					ideal_pixels[cur_startx][y]->status = BORDER;
					ideal_pixels[cur_startx][y+1]->status = BORDER;
				}
			}else { //border[bottom] down
				for(int y=cur_starty;y>cur_endy;y--){
					ideal_pixels[cur_startx][y]->leave(x1, BOTTOM,i);
					ideal_pixels[cur_startx][y-1]->enter(x1, TOP,i);
					ideal_pixels[cur_startx][y]->status = BORDER;
					ideal_pixels[cur_startx][y-1]->status = BORDER;
				}
			}
		}else{
			// solve the line function
			double a = (y1-y2)/(x1-x2);
			double b = (x1*y2-x2*y1)/(x1-x2);

			int x = cur_startx;
			int y = cur_starty;
			while(x!=cur_endx||y!=cur_endy){
				bool passed = false;
				double yval = 0;
				double xval = 0;
				int cur_x = 0;
				int cur_y = 0;
				//check horizontally
				if(x!=cur_endx){
					if(cur_startx<cur_endx){
						xval = ((double)x+1)*step_x+start_x;
					}else{
						xval = (double)x*step_x+start_x;
					}
					yval = xval*a+b;
					cur_y = (yval-start_y)/step_y;
					//printf("y %f %d\n",(yval-start_y)/step_y,cur_y);
					if(cur_y>max(cur_endy, cur_starty)){
						cur_y=max(cur_endy, cur_starty);
					}
					if(cur_y<min(cur_endy, cur_starty)){
						cur_y=min(cur_endy, cur_starty);
					}
					if(cur_y==y){
						passed = true;
						// left to right
						if(cur_startx<cur_endx){
							ideal_pixels[x][y]->status = BORDER;
							ideal_pixels[x++][y]->leave(yval,RIGHT,i);
							ideal_pixels[x][y]->enter(yval,LEFT,i);
							ideal_pixels[x][y]->status = BORDER;
						}else{//right to left
							ideal_pixels[x][y]->status = BORDER;
							ideal_pixels[x--][y]->leave(yval,LEFT,i);
							ideal_pixels[x][y]->enter(yval,RIGHT,i);
							ideal_pixels[x][y]->status = BORDER;
						}
					}
				}
				//check vertically
				if(y!=cur_endy){
					if(cur_starty<cur_endy){
						yval = (y+1)*step_y+start_y;
					}else{
						yval = y*step_y+start_y;
					}
					xval = (yval-b)/a;
					int cur_x = (xval-start_x)/step_x;
					//printf("x %f %d\n",(xval-start_x)/step_x,cur_x);
					if(cur_x>max(cur_endx, cur_startx)){
						cur_x=max(cur_endx, cur_startx);
					}
					if(cur_x<min(cur_endx, cur_startx)){
						cur_x=min(cur_endx, cur_startx);
					}
					if(cur_x==x){
						passed = true;
						if(cur_starty<cur_endy){// bottom up
							ideal_pixels[x][y]->status = BORDER;
							ideal_pixels[x][y++]->leave(xval, TOP,i);
							ideal_pixels[x][y]->enter(xval, BOTTOM,i);
							ideal_pixels[x][y]->status = BORDER;
						}else{// top down
							ideal_pixels[x][y]->status = BORDER;
							ideal_pixels[x][y--]->leave(xval, BOTTOM,i);
							ideal_pixels[x][y]->enter(xval, TOP,i);
							ideal_pixels[x][y]->status = BORDER;
						}
					}
				}
				// for debugging, should never happen
				if(!passed){
					vs->print();
					cout<<"dim\t"<<dimx<<" "<<dimy<<endl;
					printf("val\t%f %f\n",(xval-start_x)/step_x, (yval-start_y)/step_y);
					cout<<"curxy\t"<<x<<" "<<y<<endl;
					cout<<"calxy\t"<<cur_x<<" "<<cur_y<<endl;
					cout<<"xrange\t"<<cur_startx<<" "<<cur_endx<<endl;
					cout<<"yrange\t"<<cur_starty<<" "<<cur_endy<<endl;
					printf("xrange_val\t%f %f\n",(x1-start_x)/step_x, (x2-start_x)/step_x);
					printf("yrange_val\t%f %f\n",(y1-start_y)/step_y, (y2-start_y)/step_y);
				}
				assert(passed);
			}
		}
	}

	// for(vector<Pixel *> &rows:pixels){
	// 	for(Pixel *p:rows){
	// 		for(int i=0;i<4;i++){
	// 			if(p->intersection_nodes[i].size()>0){
	// 				p->status = BORDER;
	// 				break;
	// 			}
	// 		}
	// 	}
	// }


	for(vector<Pixel *> &rows:ideal_pixels){
		for(Pixel *pix:rows){
			pix->ideal_process_crosses(vs->num_vertices);
		}
	}
}

void MyRaster::ideal_scanline_reandering(){
	for(int y=1;y<dimy;y++){
		bool isin = false;
		for(int x=0;x<dimx;x++){
			if(ideal_pixels[x][y]->status!=BORDER){
				if(isin){
					ideal_pixels[x][y]->status = IN;
				}
				continue;
			}
			if(ideal_pixels[x][y]->intersection_nodes[BOTTOM].size()%2==1){
				isin = !isin;
			}
		}
	}
}

void MyRaster::ideal_rasterization(){

	//1. create space for the pixels
	ideal_init_pixels();

	//2. edge crossing to identify BORDER pixels
	ideal_evaluate_edges();

	//3. determine the status of rest pixels with scanline rendering
	ideal_scanline_reandering();
}

void MyRaster::ideal_print(){
	MyMultiPolygon *inpolys = new MyMultiPolygon();
	MyMultiPolygon *borderpolys = new MyMultiPolygon();
	MyMultiPolygon *outpolys = new MyMultiPolygon();

	for(int i=0;i<=dimx;i++){
		for(int j=0;j<=dimy;j++){
			MyPolygon *m = MyPolygon::gen_box(*ideal_pixels[i][j]);
			if(ideal_pixels[i][j]->status==BORDER){
				borderpolys->insert_polygon(m);
			}else if(ideal_pixels[i][j]->status==IN){
				inpolys->insert_polygon(m);
			}else if(ideal_pixels[i][j]->status==OUT){
				outpolys->insert_polygon(m);
			}
		}
	}

	cout<<"border:"<<endl;
	borderpolys->print();
	cout<<"in:"<<endl;
	inpolys->print();
	cout<<"out:"<<endl;
	outpolys->print();


	delete borderpolys;
	delete inpolys;
	delete outpolys;
}
