#include "ComplexNum.h"

//empty constructor
//template<typename T>
ComplexNum::ComplexNum() {
	//set value to 0 + 0i
	real = 0; img = 0;
}

//non empty constructor
//template<typename T>
ComplexNum::ComplexNum(double _real, double _img){
	//set value to _real + _img*i
	real = _real;
	img = _img;
}

//destructor
//template<typename T>
ComplexNum::~ComplexNum() = default;

//template<typename T>
ComplexNum ComplexNum::operator=(ComplexNum input){
	return input;
}

//template<typename T>
ComplexNum ComplexNum::operator+(ComplexNum second){
	return ComplexNum(this->real + second.real, this->img + second.img);
}

ComplexNum ComplexNum::operator*(ComplexNum second){
	ComplexNum C;
	C.real = this->real * second.real - this->img * second.img; //calculate real part
	C.img = this->real * second.img + this->img * second.real; //calculate im part
	return C;
}


