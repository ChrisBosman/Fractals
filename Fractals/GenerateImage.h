#pragma once
#include <SDL.h>
#include <iostream>
//#include <thread>

class GenerateImage
{
public:
	GenerateImage(SDL_Renderer* _renderer, int  _windowWidth, int _windowHeight, double _xBound[2], double _yBound[2], int _fracfunct, uint8_t& _color) {
		fracfunct = _fracfunct;
		windowWidth = _windowWidth;
		windowHeight = _windowHeight;
		yBound = _yBound;
		xBound = _xBound;
		renderer = _renderer;
		color = _color;
	}

	void operator()(int imageHeight, int maxN, int xBegin, int xEnd,std::vector<int>* stabilityValue, double initialR = 0, double initialI = 0, bool juliaSet = false) {
		//loop over eacht pixel
		for (int y = 0; y < imageHeight; y++) { //each column
			for (int x = xBegin; x < xEnd; x++) { //each row
				//Find the coordinates on the complex plane for that pixel
				double cReal = mapToReal(x);
				double cIm = mapToImaginary(y);
				//find how stable the point is
				int n = 0;
				if (juliaSet) {
					n = checkStability(initialR, initialI, maxN, cReal, cIm);
				}
				else
					n = checkStability(cReal, cIm, maxN, initialR, initialI);
				stabilityValue->at((size_t)y+(x-xBegin)*(size_t)imageHeight) = n;
				//setColor(n, maxN); //set renderer draw color
				//SDL_RenderDrawPoint(renderer, x, y);
			}
		}
	}

private:
	//check if points are stable with recursion
	int checkStability(double cReal, double cIm, int N, double zReal = 0, double zIm = 0) {
		int i = 0;
		while (i < N && zReal * zReal + zIm * zIm < 4.0) {
			double tmp;
			switch (fracfunct) {
			case 1:
				//Z^2 + C
				tmp = zReal * zReal - zIm * zIm + cReal;
				zIm = 2.0 * zReal * zIm + cIm;
				break;
			case 2:
				//Z ^ 3 + C
				tmp = zReal * (zReal * zReal - 3 * zIm * zIm) + cReal;
				zIm = zIm * (3 * zReal * zReal - zIm * zIm) + cIm;
				break;
			case 3:
				//Z^4 + C
				tmp = zReal * zReal * zReal * zReal - 6 * zReal * zReal * zIm * zIm + zIm * zIm * zIm * zIm + cReal;
				zIm = 4 * zReal * zIm * (zReal * zReal - zIm * zIm) + cIm;
				break;
			case 4:
				//burning ship
				//Z^2 + C
				tmp = zReal * zReal - zIm * zIm + cReal;
				zIm = abs(2.0 * zReal * zIm) + cIm;
				break;
			case 5:
				//strange arrow
				tmp = zReal * zReal * zReal * zReal - 6 * zReal * zReal * zIm * zIm + zIm * zIm * zIm * zIm + cReal;
				zIm = 2.0 * zReal * zIm + cIm;
				break;
			case 6:
				//strange cross
				tmp = zReal * zReal - zIm * zIm + cReal;
				zIm = 4 * zReal * zIm * (zReal * zReal - zIm * zIm) + cIm;
				break;
			case 7:
				//bird
				//e^Zi * Z^2 + C
				tmp = cos(zReal) * zReal * zReal - zIm * zIm + cReal;
				zIm = sin(zIm) * 2.0 * zReal * zIm + cIm;
				break;
			default:
				//std::cout << "No fractal function selected, so just drawing default Mandelbrot\n";
				//Z^2 + C
				tmp = zReal * zReal - zIm * zIm + cReal;
				zIm = 2.0 * zReal * zIm + cIm;
				break;
			}
			zReal = tmp;
			i++;
		}
		return i;
	}

	//functions for mapping from the pixels to complex values and the other way
	double mapToReal(int x) {
		double range = xBound[1] - xBound[0];
		return x * (range / windowWidth) + xBound[0];
	}

	double mapToImaginary(int y) {
		double range = yBound[1] - yBound[0];
		return y * (range / windowHeight) + yBound[0];
	}

	//maping  hsv To RGB
	void mapHsvToRGB(int* RGB, int H, double V, double S) {
		//make sure H is between 0 and 360
		while (H > 360)
			H -= 360;
		//make V and S between 0 and 1
		if (V > 1)
			V = 1;
		if (S > 1)
			S = 1;
		//calculate some variables
		double C = V * S;
		double X = C * (1 - abs((H / 60) % 2 - (int8_t)1));
		double m = V - C;
		// Calculate the values for R,G and B
		RGB[0] = (C * ((H >= 0 && H < 60) || (H >= 300 && H < 360)) + X * ((H >= 60 && H < 120) || (H >= 240 && H < 300)) + m) * 255; // red
		RGB[1] = (C * ((H >= 60 && H < 120) || (H >= 120 && H < 180)) + X * ((H >= 0 && H < 60) || (H >= 180 && H < 240)) + m) * 255; // green
		RGB[2] = (C * ((H >= 180 && H < 240) || (H >= 240 && H < 300)) + X * ((H >= 120 && H < 180) || (H >= 300 && H < 360)) + m) * 255; // blue
		return;
	}

	//map to a blue color list using a second order polynomial
	void mapToBlue(int* RGB, int n, int max) {
		double n2 = (double)n / max * 200; //normilize
		double Cr[] = { -0.003143, 0.612571, 0.085714 };
		double Cg[] = { 0.000771, 0.683714, 0.857143 };
		double Cb[] = { 0.000047, -0.015057, 2.164762, 0.714286 };
		RGB[0] = Cr[0] * n2 * n2 + Cr[1] * n2 + Cr[2];
		RGB[1] = Cg[0] * n2 * n2 + Cg[1] * n2 + Cg[2];
		RGB[2] = n2 * (n2 * (Cb[0] * n2 + Cb[1]) + Cb[2]) + Cb[3];
	}

	void setColor(int n, int maxN) {
		//mab the resulting number to RGB
		int RGB[3] = { 0 };
		switch (color)
		{
		case 0://black and white
			RGB[0] = RGB[1] = RGB[2] = n;
			break;
		case 1: //red&orange
			RGB[0] = n * 2; //red
			RGB[1] = n * 0.6; //green
			RGB[2] = 0; //blue
			break;
		case 2: //green
			RGB[0] = 0; //red
			RGB[1] = n * 2; //green
			RGB[2] = n * 0.6; //blue		  
			break;
		case 3: //blue
			mapToBlue(RGB, n, maxN);
			break;
		case 4: //red&purple
			RGB[0] = n * 2; //red
			RGB[1] = 0; //green
			RGB[2] = n * 0.6; //blue
			break;
		case 5: //multiple colors
			mapHsvToRGB(RGB, n, (double)n * 2 / maxN, (double)n * 5 / maxN);
			break;
		case 6: //multible colors, extremer
			mapHsvToRGB(RGB, cos(n * n) * 200, n, n);
			break;

		default:
			break;
		}
		if (n == maxN) { //in stable than make it black
			RGB[0] = RGB[1] = RGB[2] = 0;
		}
		SDL_SetRenderDrawColor(renderer, RGB[0], RGB[1], RGB[2], 1);
	}

private:
	int fracfunct = 1; //the selected function for drawing fractals
	int windowWidth;
	int windowHeight;
	double* xBound;
	double* yBound;
	SDL_Renderer* renderer;
	uint8_t color;
};

