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
}

void MyRaster::evaluate_edges(){
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
		set_status(cur_startx, cur_starty, BORDER);
		set_status(cur_endx, cur_endy, BORDER);

		//in the same pixel
		if(cur_startx==cur_endx&&cur_starty==cur_endy){
			continue;
		}

		if(y1==y2){
			//left to right
			if(cur_startx<cur_endx){
				for(int x=cur_startx;x<cur_endx;x++){
					pixels[x][cur_starty]->leave(y1,RIGHT,i);
					pixels[x+1][cur_starty]->enter(y1,LEFT,i);
				}
			}else { // right to left
				for(int x=cur_startx;x>cur_endx;x--){
					pixels[x][cur_starty]->leave(y1, LEFT,i);
					pixels[x-1][cur_starty]->enter(y1, RIGHT,i);
				}
			}
		}else if(x1==x2){
			//bottom up
			if(cur_starty<cur_endy){
				for(int y=cur_starty;y<cur_endy;y++){
					pixels[cur_startx][y]->leave(x1, TOP,i);
					pixels[cur_startx][y+1]->enter(x1, BOTTOM,i);
				}
			}else { //border[bottom] down
				for(int y=cur_starty;y>cur_endy;y--){
					pixels[cur_startx][y]->leave(x1, BOTTOM,i);
					pixels[cur_startx][y-1]->enter(x1, TOP,i);
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
							pixels[x++][y]->leave(yval,RIGHT,i);
							pixels[x][y]->enter(yval,LEFT,i);
						}else{//right to left
							pixels[x--][y]->leave(yval,LEFT,i);
							pixels[x][y]->enter(yval,RIGHT,i);
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
							pixels[x][y++]->leave(xval, TOP,i);
							pixels[x][y]->enter(xval, BOTTOM,i);
						}else{// top down
							pixels[x][y--]->leave(xval, BOTTOM,i);
							pixels[x][y]->enter(xval, TOP,i);
						}
					}
				}
				assert(passed);
			}
		}
	}

	for(vector<Pixel *> &rows:pixels){
		for(Pixel *p:rows){
			for(int i=0;i<4;i++){
				if(p->intersection_nodes[i].size()>0){
					p->status = BORDER;
					break;
				}
			}
		}
	}


	for(vector<Pixel *> &rows:pixels){
		for(Pixel *pix:rows){
			pix->process_crosses(vs->num_vertices);
		}
	}
}

void MyRaster::scanline_reandering(){
	for(int y=1;y<dimy;y++){
		bool isin = false;
		for(int x=0;x<dimx;x++){
			if(pixels[x][y]->status!=BORDER){
				if(isin){
					pixels[x][y]->status = IN;
				}
				continue;
			}
			if(pixels[x][y]->intersection_nodes[BOTTOM].size()%2==1){
				isin = !isin;
			}
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
			MyPolygon *m = MyPolygon::gen_box(*pixels[i][j]);
			if(pixels[i][j]->status==BORDER){
				borderpolys->insert_polygon(m);
			}else if(pixels[i][j]->status==IN){
				inpolys->insert_polygon(m);
			}else if(pixels[i][j]->status==OUT){
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

void MyRaster::set_status(int x, int y, PartitionStatus status){
	int id = y * dimx + x;
	uint8_t st = pixels->status[id / 4];
	int pos = id % 4 * 2;   //乘2是因为每个status占2bit
	if(status == OUT){
		st &= ~((uint8_t)3 << pos);
	}else if(status == IN){
		st |= ((uint8_t)3 << pos);
	}else{
		st &= ~((uint8_t)1 << pos);
		st |= ((uint8_t)1 << (pos + 1));
	}
}

PartitionStatus MyRaster::show_status(int x, int y){
	int id = y * dimx + x;
	uint8_t st = pixels->status[id / 4];
	int pos = id % 4 * 2;   //乘2是因为每个status占2bit	
	st &= ((uint8_t)3 << pos);
	st >>= pos;
	if(st == 0) return OUT;
	if(st == 3) return IN;
	return BORDER;
}

MyRaster::~MyRaster(){
	for(vector<Pixel *> &rows:pixels){
		for(Pixel *p:rows){
			delete p;
		}
		rows.clear();
	}
	pixels.clear();
}