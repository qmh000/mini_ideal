#include "../include/Pixel.h"

void Pixel::enter(double val, Direction d, int vnum){
	intersection_nodes[d].push_back(val);
	crosses.push_back(cross_info(ENTER,vnum));
}

void Pixel::leave(double val, Direction d, int vnum){
	intersection_nodes[d].push_back(val);
	crosses.push_back(cross_info(LEAVE,vnum));
}

void Pixel::process_crosses(int num_edges){
	if(crosses.size()==0){
		return;
	}

	//very very very very rare cases
	if(crosses.size()%2==1){
		crosses.push_back(cross_info((cross_type)!crosses[crosses.size()-1].type,crosses[crosses.size()-1].edge_id));
	}

	assert(crosses.size()%2==0);
	int start = 0;
	int end = crosses.size()-1;

	//special case for the first edge
	if(crosses[0].type==LEAVE){
		assert(crosses[end].type==ENTER);
		edge_ranges.push_back(edge_range(crosses[end].edge_id,num_edges-2));
		edge_ranges.push_back(edge_range(0,crosses[0].edge_id));
		start++;
		end--;
	}

	for(int i=start;i<=end;i++){
		assert(crosses[i].type==ENTER);
		//special case, an ENTER has no pair LEAVE,
		//happens when one edge crosses the pair
		if(i==end||crosses[i+1].type==ENTER){
			edge_ranges.push_back(edge_range(crosses[i].edge_id,crosses[i].edge_id));
		}else{
			edge_ranges.push_back(edge_range(crosses[i].edge_id,crosses[i+1].edge_id));
			i++;
		}
	}

	// confirm the correctness
	for(edge_range &r:edge_ranges){
		assert(r.vstart<=r.vend&&r.vend<num_edges);
	}
	crosses.clear();
}

box::box (double lowx, double lowy, double highx, double highy){
	low[0] = lowx;
	low[1] = lowy;
	high[0] = highx;
	high[1] = highy;
}