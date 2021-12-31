#pragma once
#include <cmath>
//template <typename T>
class ComplexNum
{
public:
//public functions
	//constructors and destuctors
	ComplexNum();  //constructor, 0+0i
	ComplexNum(double _real, double _img); //constructor, real + img*i
	~ComplexNum(); //destuctor
	//get functions
	double getMag() { return sqrt(real*real + img*img); };
	//operators overloading
	ComplexNum operator=(ComplexNum input);
	ComplexNum operator+(ComplexNum second);
	ComplexNum operator*(ComplexNum second);

	//public variables
	double real, img;
	

};

