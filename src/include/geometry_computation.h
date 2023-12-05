#ifndef SRC_GEOMETRY_GEOMETRY_COMPUTATION_H_
#define SRC_GEOMETRY_GEOMETRY_COMPUTATION_H_

#include <math.h>
#include "Pixel.h"
#include "util.h"

inline int sgn(const double& x) {
	return x >= 0 ? x ? 1 : 0 : -1;
}

inline bool inter1(double a, double b, double c, double d) {

	double tmp;
    if (a > b){
    	tmp = a;
    	a = b;
    	b = tmp;
    }
    if (c > d){
    	tmp = c;
    	c = d;
    	d = tmp;
    }
    return max(a, c) <= min(b, d);
}

inline bool segment_intersect(const Point& a, const Point& b, const Point& c, const Point& d) {
    if (c.cross(a, d) == 0 && c.cross(b, d) == 0)
        return inter1(a.x, b.x, c.x, d.x) && inter1(a.y, b.y, c.y, d.y);
    return sgn(a.cross(b, c)) != sgn(a.cross(b, d)) &&
           sgn(c.cross(d, a)) != sgn(c.cross(d, b));
}

inline bool segment_intersect_batch(Point *p1, Point *p2, int s1, int s2, size_t &checked){
	for(int i=0;i<s1;i++){
		for(int j=0;j<s2;j++){
			checked++;
			if(segment_intersect(p1[i],p1[(i+1)%s1],p2[j],p2[(j+1)%s2])){
				return true;
			}
		}
	}
	return false;
}

#endif /* SRC_GEOMETRY_GEOMETRY_COMPUTATION_H_ */