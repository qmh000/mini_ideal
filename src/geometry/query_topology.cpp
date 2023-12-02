#include <float.h>
#include <math.h>
#include <utility>

#include "../include/MyPolygon.h"

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
		uint edge_count = 0;
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
	cout << "Error!" << endl;
	return false;
}

