#include "../include/MyPolygon.h"

box *MyPolygon::getMBB(){
	if(this->mbr){
		return mbr;
	}
	mbr = boundary->getMBR();
	return mbr;
}