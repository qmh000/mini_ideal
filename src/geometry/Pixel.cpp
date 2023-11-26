#include "../include/Pixel.h"

// void Pixel::enter(double val, Direction d, int vnum){
// 	intersection_nodes[d].push_back(val);
// 	crosses.push_back(cross_info(ENTER,vnum));
// }

// void Pixel::leave(double val, Direction d, int vnum){
// 	intersection_nodes[d].push_back(val);
// 	crosses.push_back(cross_info(LEAVE,vnum));
// }

// void Pixel::process_crosses(int num_edges){
// 	if(crosses.size()==0){
// 		return;
// 	}

// 	//very very very very rare cases
// 	if(crosses.size()%2==1){
// 		crosses.push_back(cross_info((cross_type)!crosses[crosses.size()-1].type,crosses[crosses.size()-1].edge_id));
// 	}

// 	assert(crosses.size()%2==0);
// 	int start = 0;
// 	int end = crosses.size()-1;

// 	//special case for the first edge
// 	if(crosses[0].type==LEAVE){
// 		assert(crosses[end].type==ENTER);
// 		edge_ranges.push_back(edge_range(crosses[end].edge_id,num_edges-2));
// 		edge_ranges.push_back(edge_range(0,crosses[0].edge_id));
// 		start++;
// 		end--;
// 	}

// 	for(int i=start;i<=end;i++){
// 		assert(crosses[i].type==ENTER);
// 		//special case, an ENTER has no pair LEAVE,
// 		//happens when one edge crosses the pair
// 		if(i==end||crosses[i+1].type==ENTER){
// 			edge_ranges.push_back(edge_range(crosses[i].edge_id,crosses[i].edge_id));
// 		}else{
// 			edge_ranges.push_back(edge_range(crosses[i].edge_id,crosses[i+1].edge_id));
// 			i++;
// 		}
// 	}

// 	// confirm the correctness
// 	for(edge_range &r:edge_ranges){
// 		assert(r.vstart<=r.vend&&r.vend<num_edges);
// 	}
// 	crosses.clear();
// }

Pixels::Pixels(int num_pixels){
	status = new uint8_t[num_pixels / 4 + 1];
	// status = new int[num_pixels];
	pointer = new uint16_t[num_pixels];
}

Pixels::~Pixels(){
	delete status;
	delete pointer;
	delete edge_sequences;
}

void Pixels::add_edge_offset(int id, int idx){
	pointer[id] = idx;
}

box::box (double lowx, double lowy, double highx, double highy){
	low[0] = lowx;
	low[1] = lowy;
	high[0] = highx;
	high[1] = highy;
}

void Pixels::set_status(int id, PartitionStatus state){
	int pos = id % 4 * 2;   //乘2是因为每个status占2bit
	if(state == OUT){
		status[id / 4] &= ~((uint8_t)3 << pos);
	}else if(state == IN){
		status[id / 4] |= ((uint8_t)3 << pos);
	}else{
		status[id / 4] &= ~((uint8_t)1 << pos);
		status[id / 4] |= ((uint8_t)1 << (pos + 1));
	}
}

// void Pixels::set_status(int id, PartitionStatus state){
// 	status[id] = state;
// }

PartitionStatus Pixels::show_status(int id){
	uint8_t st = status[id / 4];
	int pos = id % 4 * 2;   //乘2是因为每个status占2bit	
	st &= ((uint8_t)3 << pos);
	st >>= pos;
	if(st == 0) return OUT;
	if(st == 3) return IN;
	return BORDER;
}

// PartitionStatus Pixels::show_status(int id){
// 	int st = status[id];
// 	if(st == 0) return OUT;
// 	if(st == 2) return IN;
// 	return BORDER;
// }

int Pixels::get_num_sequences(int id){
	if(show_status(id) != BORDER) return 0;
	else return pointer[id + 1] - pointer[id];
}

void Pixels::process_pixels_null(int x, int y){
	for(int i = (x+1)*(y+1)-1; i >= 0; i --){
		if(show_status(i) != BORDER){
			pointer[i] = pointer[i + 1]; 
		}
	}
}

void Pixels::add_edge(int idx, int start, int end){
	edge_sequences[idx] = make_pair(start, end - start  + 1);
}

void Pixels::init_edge_sequences(int num_edge_seqs){
	edge_sequences = new pair<uint16_t, uint8_t>[num_edge_seqs];
}

bool box::contain(Point &p){
	return p.x>=low[0]&&
		   p.x<=high[0]&&
		   p.y>=low[1]&&
		   p.y<=high[1];
}