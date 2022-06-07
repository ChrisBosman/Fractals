/*
	Handels the creation of the game and passes the updata,render and input though
*/

#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <ctime>
#include <thread>
#include "GenerateImage.h"

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
	//private enums
	enum class State
	{
		fractal,
		escMenu,
		functionMenu
	};
	//private functions
	void fatalError(std::string);
	void initTextures();
	void createTexture(const char*,SDL_Texture*&);
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
	void processEscMenuInput(SDL_Event&); //process input while in esc menu
	void processFunMenuInput(SDL_Event&); //process input while in func menu

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
	//textures
	SDL_Texture* fractalTex = nullptr;
	SDL_Texture* escMenuTex = nullptr;
	SDL_Texture* funcMenuTex = nullptr;
	//Settings
	uint8_t color = 3; //1 = red, 2 = green, 3 = blue, 5 = all colors 
	bool juliaSet = false; //switch to the julia set
	bool buddhaSet = false; //switch to a sort buddha thing
	int fracfunct = 1; //the selected function for drawing fractals
	State state = State::fractal;
	uint8_t amountOfThreads = 2;
	unsigned int maxN = 500;
};