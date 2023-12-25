#include "../include/MyPolygon.h"

double MyPolygon::distance(Point &p, query_context *ctx, bool profile){

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

// get the distance from pixel pix to polygon target
double MyPolygon::distance(MyPolygon *target, int pix, query_context *ctx, bool profile){
	assert(target->raster);
	assert(raster);
	struct timeval start = get_cur_time();

    auto pix_x = raster->get_x(pix);
    auto pix_y = raster->get_y(pix);
    auto pix_box = raster->get_pixel_box(pix_x, pix_y);
	double mindist = target->getMBB()->max_distance(*pix_box, ctx->geography);
	double mbrdist = target->getMBB()->distance(*pix_box, ctx->geography);
	double min_mbrdist = mbrdist;
	int step = 0;
	double step_size = target->raster->get_step(ctx->geography);
	auto r_pixs = raster->get_pixels();
    auto t_pixs = target->raster->get_pixels();
    assert(r_pixs->show_status(pix) == BORDER);

	// initialize the seed closest pixels
	vector<int> needprocess = target->raster->get_closest_pixels(pix_box);
	assert(needprocess.size()>0);
	unsigned short lowx = target->raster->get_x(needprocess[0]);
	unsigned short highx = target->raster->get_x(needprocess[0]);
	unsigned short lowy = target->raster->get_y(needprocess[0]);
	unsigned short highy = target->raster->get_y(needprocess[0]);
	for(auto p : needprocess){
		lowx = min(lowx, (unsigned short)target->raster->get_x(p));
		highx = max(highx, (unsigned short)target->raster->get_x(p));
		lowy = min(lowy, (unsigned short)target->raster->get_y(p));
		highy = max(highy, (unsigned short)target->raster->get_y(p));
	}

	ctx->pixel_evaluated.execution_time += get_time_elapsed(start);

	while(true){
		struct timeval start = get_cur_time();

		// for later steps, expand the circle to involve more pixels
		if(step>0){
			needprocess = raster->expand_radius(lowx,highx,lowy,highy,step);
		}
		ctx->pixel_evaluated.counter += needprocess.size();
		ctx->pixel_evaluated.execution_time += get_time_elapsed(start, true);

		// all the boxes are scanned (should never happen)
		if(needprocess.size()==0){
			return mindist;
		}

		for(auto cur : needprocess){
			if(profile){
				ctx->pixel_evaluated.counter++;
			}
			//printf("checking pixel %d %d %d\n",cur->id[0],cur->id[1],cur->status);
			// note that there is no need to check the edges of
			// this pixel if it is too far from the target
            auto cur_x = target->raster->get_x(cur);
            auto cur_y = target->raster->get_y(cur);
			
            if(t_pixs->show_status(cur) == BORDER){
				start = get_cur_time();
				bool toofar = (target->raster->get_pixel_box(cur_x, cur_y)->distance(*pix_box,ctx->geography) >= mindist);
				if(profile){
					ctx->border_evaluated.counter++;
					ctx->border_evaluated.execution_time += get_time_elapsed(start, true);
				}
				if(toofar){
					continue;
				}
				//ctx->border_evaluated.counter++;
				// the vector model need be checked.
				start = get_cur_time();
				if(profile){
					ctx->border_checked.counter++;
				}

				for(int i = 0; i < raster->get_num_sequences(pix); i ++){
					auto pix_er = r_pixs->edge_sequences[r_pixs->pointer[pix] + i];
					for(int j = 0; j < target->raster->get_num_sequences(cur); j ++){
						auto cur_er = t_pixs->edge_sequences[t_pixs->pointer[cur] + j];
						double dist = segment_sequence_distance(target->boundary->p+cur_er.first, boundary->p+pix_er.first, cur_er.second, pix_er.second, ctx->geography);
						if(profile){
							ctx->edge_checked.counter += pix_er.second*cur_er.second;
						}
						mindist = min(dist, mindist);
					}
				}

				// for(edge_range &pix_er:pix->edge_ranges){
				// 	for(edge_range &cur_er:cur->edge_ranges){
				// 		double dist;
				// 		if(ctx->is_within_query()){
				// 			dist = segment_to_segment_within_batch(target->boundary->p+pix_er.vstart,
				// 								boundary->p+cur_er.vstart, pix_er.size(), cur_er.size(),
				// 								ctx->within_distance, ctx->geography, ctx->edge_checked.counter);
				// 		}else{
				// 			dist = segment_sequence_distance(target->boundary->p+pix_er.vstart,
				// 								boundary->p+cur_er.vstart, pix_er.size(), cur_er.size(), ctx->geography);
				// 			if(profile){
				// 				ctx->edge_checked.counter += pix_er.size()*cur_er.size();
				// 			}
				// 		}
				// 		mindist = min(dist, mindist);
				// 		if(ctx->within(mindist)){
				// 			if(profile){
				// 				ctx->edge_checked.execution_time += get_time_elapsed(start, true);
				// 			}
				// 			return mindist;
				// 		}
				// 	}
				// }
				if(profile){
					ctx->edge_checked.execution_time += get_time_elapsed(start, true);
				}
			}
		}
		//log("step:%d #pixels:%ld radius:%f mindist:%f",step, needprocess.size(), mbrdist+step*step_size, mindist);
		needprocess.clear();
		double min_possible = mbrdist+step*step_size;
		// the minimum distance for now is good enough for three reasons:
		// 1. current minimum distance is smaller than any further distance
		// 2. for within query, current minimum is close enough
		// 3. for within query, current minimum could never be smaller than the threshold
		if(mindist <= min_possible){
			return mindist;
		}
		step++;
	}

	return mindist;
}

double MyPolygon::distance(MyPolygon *target, query_context *ctx){

	ctx->object_checked.counter++;

	if(raster){
		timeval start = get_cur_time();

		double mindist = getMBB()->max_distance(*target->getMBB(), ctx->geography);
		const double mbrdist = getMBB()->distance(*target->getMBB(),ctx->geography);
		double min_mbrdist = mbrdist;
		int step = 0;
		double step_size = raster->get_step(ctx->geography);   //step_r
        auto r_pixs = raster->get_pixels();
		auto t_pixs = target->raster->get_pixels();

		vector<int> needprocess = raster->get_closest_pixels(target->getMBB());
		assert(needprocess.size()>0);
		unsigned short lowx = raster->get_x(needprocess[0]);
		unsigned short highx = raster->get_x(needprocess[0]);
		unsigned short lowy = raster->get_y(needprocess[0]);
		unsigned short highy = raster->get_y(needprocess[0]);
		for(auto p : needprocess){
			lowx = min(lowx, (unsigned short)raster->get_x(p));
			highx = max(highx, (unsigned short)raster->get_x(p));
			lowy = min(lowy, (unsigned short)raster->get_y(p));
			highy = max(highy, (unsigned short)raster->get_y(p));
		}
		ctx->pixel_evaluated.execution_time += get_time_elapsed(start);

		while(true){
			struct timeval start = get_cur_time();

			// first of all, expand the circle to involve more pixels
			if(step>0){
				needprocess = raster->expand_radius(lowx,highx,lowy,highy,step);
			}
			ctx->pixel_evaluated.counter += needprocess.size();
			ctx->pixel_evaluated.execution_time += get_time_elapsed(start, true);

			// all the boxes are scanned (should never happen)
			if(needprocess.size()==0){
				return mindist;
			}

			for(auto cur : needprocess){
				//printf("checking pixel %d %d %d\n",cur->id[0],cur->id[1],cur->status);
				// note that there is no need to check the edges of
				// this pixel if it is too far from the target
                auto cur_x = raster->get_x(cur);
                auto cur_y = raster->get_y(cur);
				if(r_pixs->show_status(cur) == BORDER && raster->get_pixel_box(cur_x, cur_y)->distance(*target->getMBB(),ctx->geography) < mindist){
					// the vector model need be checked.
					// do a polygon--pixel distance calculation
					double dist = distance(target, cur, ctx, true);
					mindist = min(dist, mindist);
				}
			}
			//log("step:%d #pixels:%ld radius:%f mindist:%f",step, needprocess.size(), mbrdist+step*step_size, mindist);
			needprocess.clear();
			double min_possible = mbrdist+step*step_size;
			// the minimum distance for now is good enough for three reasons:
			// 1. current minimum distance is smaller than any further distance
			// 2. for within query, current minimum is close enough
			// 3. for within query, current minimum could never be smaller than the threshold
			if(mindist <= min_possible){
				return mindist;
			}
			step++;
		}

		// iterate until the closest pair of edges are found
		assert(false && "happens when there is no boundary pixel, check out the input");
		return DBL_MAX;
	}
}