/*
	Handels the creation of the game and passes the updata,render and input though
*/

#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

class MainGame
{
public:
	MainGame(int, int);
	~MainGame();

	void initSystems(const char* title, bool fullscreen);

	void processInput();
	void update();
	void render();
	void clean();

	bool running() { return isRunning; }

private:
	//private functions
	void fatalError(std::string);
	void initTextures();
	void createTexture(const char*,SDL_Texture*);
	int checkStability(double, double, int, double, double);
	void checkDensity(double cReal, double cIm, int N, std::vector<std::vector<uint16_t>>& density, double zReal, double zIm);
	void saveTexture(const char*, SDL_Texture*);
	void zoomIn(int x ,int y, double procent);
	void CreateBuddha(int);
	double mapToReal(int); //map pixel to complex plane
	double mapToImaginary(int); //map pixel to complex plane
	int mapFromReal(double); //map real to pixel
	int mapFromImaginary(double); //map imaginary to pixel
	void setColor(int n, int max); //set the color for the pixel based on the stability
	//private variables
	bool isRunning;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int windowWidth;
	int windowHeight;
	double initialR = 0, initialI = 0; //values for the start point of the stability check
	double xBound[2] = { -2 , 1 };
	double yBound[2] = { -1.5 , 1.5 };
	char keyPressed = 0;
	int mousePos[2] = { 0,0 };
	//texture
	SDL_Texture* fractalTex = nullptr;
	//Settings
	uint8_t color = 1; //1 = red, 2 = green, 3 = blue, 5 = all colors 
	bool juliaSet = false; //switch to the julia set
	bool buddhaSet = false; //switch to a sort buddha thing
};


/*What I need:
	* renderer
	* gridsize and bounds
	* make a vector of points over the screen with gridsieze size
	* full the array if number on how stable the points are
	* render it some way, maybe using marching square or make my own methode
	* rendering might be difficult
*/