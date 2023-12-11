#include <float.h>
#include <math.h>
#include <utility>

#include "../include/MyPolygon.h"
#include "../include/geometry_computation.h"

bool MyPolygon::contain(Point &p, query_context *ctx, bool profile){

	// the MBB may not be checked for within query
	if(!mbr->contain(p)){
		return false;
	}
	if(profile){
		ctx->object_checked.counter++;
	}

	struct timeval start = get_cur_time();
	// todo adjust the lower bound of pixel number when the raster model is usable
	if(raster && get_num_pixels()>5){
		start = get_cur_time();
        //Pixel *target = raster->get_pixel(p);
        int target = raster->get_pixel_id(p);
        auto pix = raster->get_pixels();
		if(profile){
			ctx->pixel_evaluated.counter++;
			ctx->pixel_evaluated.execution_time += get_time_elapsed(start);
		}
		if(pix->show_status(target) == IN) {
			return true;
		}
		if(pix->show_status(target) == OUT){
			return false;
		}

		start = get_cur_time();
		bool ret = false;

		// checking the intersection edges in the target pixel
		// uint edge_count = 0;
		// for(edge_range &rg:target->edge_ranges){
		// 	for(int i = rg.vstart; i <= rg.vend; i++) {
		// 		int j = i+1;
		// 		if(((boundary->p[i].y >= p.y) != (boundary->p[j].y >= p.y))){
		// 			double int_x = (boundary->p[j].x - boundary->p[i].x) * (p.y - boundary->p[i].y) / (boundary->p[j].y - boundary->p[i].y) + boundary->p[i].x;
		// 			if(p.x <= int_x && int_x <= target->high[0]){
		// 				ret = !ret;
		// 			}
		// 		}
		// 	}
		// 	edge_count += rg.size();
		// }
		uint edge_count = 0;
        for(uint16_t e = 0; e < raster->get_num_sequences(target); e ++){    
            auto edge = pix->get_edge(pix->pointer[target] + e);
            auto pos = edge.first;
            for(int k = 0; k < edge.second; k ++){
                int i = pos + k;
                int j = i + 1;  //ATTENTION
                if(((boundary->p[i].y >= p.y) != (boundary->p[j].y >= p.y))){
					double int_x = (boundary->p[j].x - boundary->p[i].x) * (p.y - boundary->p[i].y) / (boundary->p[j].y - boundary->p[i].y) + boundary->p[i].x;
					if(p.x <= int_x && int_x <= raster->get_double_x(raster->get_x(target) + 1)){
						ret = !ret;
					}
				}
            }
			edge_count += edge.second;
        }
		if(profile){
			ctx->edge_checked.counter += edge_count;
			ctx->edge_checked.execution_time += get_time_elapsed(start);
		}

		// check the crossing nodes on the right bar
		// swap the state of ret if odd number of intersection
		// nodes encountered at the right side of the border
		struct timeval tstart = get_cur_time();
		int nc = raster->count_intersection_nodes(p);
		if(nc%2==1){
			ret = !ret;
		}
		if(profile){
			ctx->intersection_checked.counter += nc;
			ctx->intersection_checked.execution_time += get_time_elapsed(tstart);

			ctx->border_checked.counter++;
			ctx->border_checked.execution_time += get_time_elapsed(start);
			ctx->refine_count++;
		}

		return ret;
	}
    // else{

	// 	// check the maximum enclosed rectangle (MER)
	// 	if(mer&&mer->contain(p)){
	// 		return true;
	// 	}
	// 	// check the convex hull
	// 	if(convex_hull&&!convex_hull->contain(p)){
	// 		return false;
	// 	}

	// 	// refinement step
	// 	start = get_cur_time();
	// 	bool contained = false;

	// 	// todo for test only, remove in released version
	// 	if(ctx->perform_refine)
	// 	{
	// 		if(rtree){
	// 			contained = contain_rtree(rtree,p,ctx);
	// 		}else{
	// 			contained = contain(p);
	// 		}
	// 	}
	// 	if(profile){
	// 		ctx->refine_count++;
	// 		ctx->edge_checked.counter += get_num_vertices();
	// 		ctx->edge_checked.execution_time += get_time_elapsed(start);
	// 		ctx->border_checked.counter++;
	// 		ctx->border_checked.execution_time += get_time_elapsed(start);
	// 	}
	// 	return contained;
	// }
	return false;
}

bool MyPolygon::contain(MyPolygon *target, query_context *ctx){
	if(!getMBB()->contain(*target->getMBB())){
		//log("mbb do not contain");
		return false;
	}
	ctx->object_checked.counter++;
	struct timeval start = get_cur_time();

	if(raster){
		vector<int> pxs = raster->retrieve_pixels(target->getMBB());
		auto pixs = raster->get_pixels();
		int etn = 0;
		int itn = 0;
		vector<int> bpxs;
		for(auto p : pxs){
			if(pixs->show_status(p) == OUT){
				etn++;
			}else if(pixs->show_status(p) == IN){
				itn++;
			}else{
				bpxs.push_back(p);
			}
		}
		//log("%d %d %d",etn,itn,pxs.size());
		if(etn == pxs.size()){
			return false;
		}
		if(itn == pxs.size()){
			return true;
		}
		ctx->border_checked.counter++;

		start = get_cur_time();
		if(target->raster){
			vector<pair<int, int>> candidates;
			vector<int> bpxs2;
			start = get_cur_time();
			for(auto p : bpxs){
				box *bx =  raster->get_pixel_box(raster->get_x(p), raster->get_y(p));
				bpxs2 = target->raster->retrieve_pixels(bx);
				delete bx;
				for(auto p2 : bpxs2){
					ctx->pixel_evaluated.counter++;
					// an external pixel of the container intersects an internal
					// pixel of the containee, which means the containment must be false
					if(pixs->show_status(p) == OUT && pixs->show_status(p2) == IN){
						ctx->pixel_evaluated.execution_time += get_time_elapsed(start,true);
						return false;
					}
					// evaluate the state
					if(pixs->show_status(p) == BORDER && pixs->show_status(p2) == BORDER){
						candidates.push_back(make_pair(p, p2));
					}
				}
				bpxs2.clear();
			}
			ctx->pixel_evaluated.execution_time += get_time_elapsed(start,true);

			for(auto pa : candidates){
				auto p = pa.first;
				auto p2 = pa.second;
				ctx->border_evaluated.counter++;
				// for(edge_range &r:p->edge_ranges){
				// 	for(edge_range &r2:p2->edge_ranges){
				// 		if(segment_intersect_batch(boundary->p+r.vstart, target->boundary->p+r2.vstart, r.size(), r2.size(), ctx->edge_checked.counter)){
				// 			ctx->edge_checked.execution_time += get_time_elapsed(start,true);
				// 			return false;
				// 		}
				// 	}
				// }
				for(int i = 0; i < raster->get_num_sequences(p); i ++){
					auto r = pixs->edge_sequences[pixs->pointer[p] + i];
					for(int j = 0; j <= raster->get_num_sequences(p2); j ++){
						auto r2 = pixs->edge_sequences[pixs->pointer[p2] + j];
						if(segment_intersect_batch(boundary->p+r.first, target->boundary->p+r2.first, r.second, r2.second, ctx->edge_checked.counter)){
							ctx->edge_checked.execution_time += get_time_elapsed(start,true);
							return false;
						}
					}
				}
				
			}

			ctx->edge_checked.execution_time += get_time_elapsed(start,true);
		}
		bpxs.clear();
	}
	/*
		// further filtering with the mbr of the target
		Point mbb_vertices[5];
		target->mbr->to_array(mbb_vertices);
		// no intersection between this polygon and the mbr of the target polygon
		if(!segment_intersect_batch(boundary->p, mbb_vertices, boundary->num_vertices, 5, ctx->edge_checked.counter)){
			// the target must be the one which is contained (not contain) as its mbr is contained
			if(contain(mbb_vertices[0], ctx)){
				return true;
			}
		}

		// when reach here, we have no choice but evaluate all edge pairs
		ctx->border_checked.counter++;

		// use the internal rtree if it is created
		if(rtree){
			for(int i=0;i<target->get_num_vertices();i++){
				if(!contain_rtree(rtree, *target->get_point(i), ctx)){
					return false;
				}
			}
			return true;
		}

		// otherwise, checking all the edges to make sure no intersection
		if(segment_intersect_batch(boundary->p, target->boundary->p, boundary->num_vertices, target->boundary->num_vertices, ctx->edge_checked.counter)){
			return false;
		}
	}
	*/

	// this is the last step for all the cases, when no intersection segment is identified
	// pick one point from the target and it must be contained by this polygon
	// Point p(target->getx(0),target->gety(0));
	// return contain(p, ctx,false);
}
