#include "../include/MyPolygon.h"

double MyPolygon::distance(Point &p, query_context *ctx, bool profile){

#ifdef USE_GPU
	if(ctx->gpu){
		return point_to_segment_sequence_distance_gpu(p, boundary->p, boundary->num_vertices,ctx->geography);
	}
#endif

	// distance is 0 if contained by the polygon
	double mindist = getMBB()->max_distance(p, ctx->geography);

	struct timeval query_start = get_cur_time();
	bool contained = contain(p, ctx, profile);
	if(profile){
		ctx->contain_check.execution_time += get_time_elapsed(query_start,true);
		ctx->contain_check.counter++;
	}

	if(contained){
		return 0;
	}
	if(profile){
		ctx->object_checked.counter++;
	}

    //initialize the starting pixel
    // Pixel *closest = raster->get_closest_pixel(p);
    int closest = raster->get_pixel_id(p);
    int step = 0;
    vector<int> needprocess;

    while(true){
        struct timeval start = get_cur_time();
        if(step==0){
            needprocess.push_back(closest);
        }else{
            needprocess = raster->expand_radius(closest, step);
        }
        // should never happen
        // all the boxes are scanned
        if(needprocess.size()==0){
            assert(false&&"should not evaluated all boxes");
            if(profile){
                ctx->refine_count++;
            }
            return boundary->distance(p, ctx->geography);
        }
        if(profile)	{
            ctx->pixel_evaluated.counter += needprocess.size();
            ctx->pixel_evaluated.execution_time += get_time_elapsed(start, true);
        }

        auto pixs = raster->get_pixels();

        for(auto cur:needprocess){
            //printf("checking pixel %d %d %d\n",cur->id[0],cur->id[1],cur->status);
            if(pixs->show_status(cur) == BORDER){
                start = get_cur_time();
                if(profile)
                {
                    ctx->border_evaluated.counter++;
                }
                // no need to check the edges of this pixel
                if(profile){
                    ctx->border_evaluated.execution_time += get_time_elapsed(start);
                }

                box* cur_box = raster->get_pixel_box(raster->get_x(cur), raster->get_y(cur)); 
                double mbr_dist = cur_box->distance(p, ctx->geography);
                delete cur_box;
                // skip the pixels that is further than the current minimum
                if(mbr_dist >= mindist){
                    continue;
                }

                // the vector model need be checked.
                start = get_cur_time();
                if(profile){
                    ctx->border_checked.counter++;
                }

                // for(edge_range &rg:cur->edge_ranges){
                //     for (int i = rg.vstart; i <= rg.vend; i++) {
                //         ctx->edge_checked.counter ++;
                //         double dist = point_to_segment_distance(p, *get_point(i), *get_point(i+1),ctx->geography);
                //         mindist = min(mindist, dist);
                //         if(ctx->within(mindist)){
                //             ctx->edge_checked.execution_time += get_time_elapsed(start);
                //             return mindist;
                //         }
                //     }
                // }

                // for(int i = 0; i < raster->get_num_sequences(p); i ++){
				// 	auto r = pixs->edge_sequences[pixs->pointer[p] + i];
				// 	for(int j = 0; j <= raster->get_num_sequences(p2); j ++){
				// 		auto r2 = pixs->edge_sequences[pixs->pointer[p2] + j];
				// 		if(segment_intersect_batch(boundary->p+r.first, target->boundary->p+r2.first, r.second, r2.second, ctx->edge_checked.counter)){
				// 			ctx->edge_checked.execution_time += get_time_elapsed(start,true);
				// 			return false;
				// 		}
				// 	}
				// }

                for(int i = 0; i < raster->get_num_sequences(cur); i ++){
                    auto rg = pixs->edge_sequences[pixs->pointer[cur] + i];
                    for(int j = 0; j < rg.second; j ++){
                        auto r = rg.first + j;
                        ctx->edge_checked.counter ++;
                        double dist = point_to_segment_distance(p, *get_point(r), *get_point(r+1), ctx->geography);
                        mindist = min(mindist, dist);
                    }
                }
                ctx->edge_checked.execution_time += get_time_elapsed(start);
            }
        }
        //printf("point to polygon distance - step:%d #pixels:%ld radius:%f mindist:%f\n",step,needprocess.size(),mbrdist+step*step_size,mindist);
        needprocess.clear();

        // for within query, return if the current minimum is close enough
        if(ctx->within(mindist)){
            return mindist;
        }
        step++;
        double minrasterdist = raster->get_possible_min(p, closest, step, ctx->geography);
        //cout<<step<<" "<<mindist<<" "<<minrasterdist<<endl;

        // close enough
        if(mindist < minrasterdist){
            break;
        }
    }
    if(profile){
        ctx->refine_count++;
    }
    // IDEAL return
    return mindist;
}