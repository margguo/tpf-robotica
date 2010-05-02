
#include "Cobject.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#define EPSILON 30


namespace benchmark{

Cobject::Cobject(int index,int x,int y,int w,int h)
{
	this->index=index;
	this->x=x;
	this->y=y;
	this->w=w;
	this->h=h;
	
}


Cobject::Cobject(int x,int y,int w,int h)
{
	this->x=x;
	this->y=y;
	this->w=w;
	this->h=h;
	
}

Cobject::~Cobject()
{
	this->x=x;
	this->y=y;
	this->w=w;
	this->h=h;
}

void 
Cobject::print(){

 printf("Object in x: %d, y: %d  w: %d, h: %d\n",this->x,this->y,this->w,this->h);
 
}

std::vector<int> Cobject::centroid(){
	std::vector<int> centroid(2);
	centroid[0]=this->x + this->w/2;
	centroid[1]=this->x + this->h/2;
	return centroid;
}
bool Cobject::isSimilar(Cobject * other){
	std::vector<int> centroid1=this->centroid();
	std::vector<int> centroid2=other->centroid();
	
	if( pow(
		( 	pow(fabs(centroid1[0]- centroid2[0]),2) +  
			pow(fabs(centroid1[1]- centroid2[1]),2)
		),0.5)< EPSILON){
		return true;
	}
	
	return false;

/*	if(this->overlap(other))
		return true;
		*/
}

bool Cobject::overlap(Cobject* other){
	
	if(! (this->contains(other->x,other->y) || this->contains(other->x+other->w,other->y)
		|| this->contains(other->x,other->y+other->h) || this->contains(other->x+other->w,other->y+other->h)))
			return false;
	return true;
}

bool Cobject::contains(int x,int y){
	if(x>= this->x && x<= this->x + this->w)
		if(y>=this->y && y< this->y + this->h)
			return true;
	
	return false;
}
bool Cobject::contains(Cobject * other){
	if(this->contains(other->x,other->y) && this->contains(other->x+other->w,other->y)
		&& this->contains(other->x,other->y+other->h) && this->contains(other->x+other->w,other->y+other->h))
			return true;
	return false;
}


}
