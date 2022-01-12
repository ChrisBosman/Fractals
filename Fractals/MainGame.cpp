#include "MainGame.h"
//returns N numbers that are linear spaced from .. to .. 
std::vector<double> linspace(double start, double end, int N) {
	std::vector<double> ouput;
	double step = (end - start) / N; //calculate the stepsize
	ouput.push_back(start); // fill the first element to save time
	//fill the vector
	for (int n = 1; n < N; n++) {
		ouput.push_back(ouput[n-1] + step);
	}
	return ouput;
}

//functions for mapping from the pixels to complex values and the other way
double MainGame::mapToReal(int x) {
	double range = xBound[1] - xBound[0];
	return x * (range / windowWidth) + xBound[0];
}

double MainGame::mapToImaginary(int y) {
	double range = yBound[1] - yBound[0];
	return y * (range / windowHeight) + yBound[0];
}

int MainGame::mapFromReal(double real) {
	double range = xBound[1] - xBound[0];
	return (real - xBound[0]) * (windowWidth / range);
}

int MainGame::mapFromImaginary(double im) {
	double range = yBound[1] - yBound[0];
	return (im - yBound[0]) * (windowHeight / range);
}

//check if points are stable with recursion
int MainGame::checkStability(double cReal, double cIm, int N, double zReal = 0, double zIm = 0) {
	int i = 0;
	while (i < N && zReal * zReal + zIm * zIm < 4.0) {
			//Z^2 + C
		double tmp = zReal * zReal - zIm * zIm + cReal;
		if (false){ //make the burining ship fractal if false
			zIm = 2.0 * zReal * zIm + cIm;
		}
		else {
			zIm = abs(2.0 * zReal * zIm) + cIm;
		}
		
			//Z^3 + C
		//double tmp = zReal * (zReal * zReal - 3 * zIm * zIm) + cReal;
		//zIm = zIm * (3 * zReal * zReal - zIm * zIm) + cIm;
			//Z^4 + C
		//double tmp = zReal*zReal*zReal*zReal - 6*zReal*zReal*zIm*zIm + zIm*zIm*zIm*zIm + cReal;
		//zIm = 4 * zReal * zIm * (zReal * zReal - zIm * zIm) + cIm;
		zReal = tmp;
		i++;
	}
	return i;
}

//check if points are stable, so not return the points they went though
void MainGame::checkDensity(double cReal, double cIm, int N, std::vector<std::vector<uint16_t>> &density,double zReal = 0, double zIm = 0) {
	//create an new vector
	std::vector<std::vector<uint16_t>> newDensity = density; 
	//check stability and store the intermediate points in newDensity
	int i = 0;
	while (i < N) {
		//calculate z^2 + c
		double tmp = zReal * zReal - zIm * zIm + cReal;
		zIm = 2.0 * zReal * zIm + cIm;
		zReal = tmp;
		//store point
		int x = mapFromReal(zReal);
		int y = mapFromImaginary(zIm);
		if (x < windowWidth && x >= 0 && y < windowHeight && y >= 0)
			newDensity[x][y] ++;
		i++;
	}
	if (zReal * zReal + zIm * zIm > 4.0)//if unstable
		density = newDensity; //save points
	return;
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

void MainGame::setColor(int n, int maxN) {
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
		mapHsvToRGB(RGB, cos(n*n)*200, n,n);
		break;

	default:
		break;
	}		
	if (n == maxN) { //in stable than make it black
		RGB[0] = RGB[1] = RGB[2] = 0;
	}
	SDL_SetRenderDrawColor(renderer, RGB[0], RGB[1], RGB[2], 1);
}

//shows the error and waits for the user to enter a key
void MainGame::fatalError(std::string errorString) {
	std::cerr << errorString << std::endl;
	std::cout << "Enter any key to quit...\n";
	int tmp;
	std::cin >> tmp;
	SDL_Quit();
	clean();
	exit(-1);
}

//Contructor
MainGame::MainGame(int _windowWidth, int _windowHeight) {
	window = nullptr;
	renderer = nullptr;
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;
}

//Destructor
MainGame::~MainGame() {
	clean();
}

//Initialize Systems
void MainGame::initSystems(const char* title, bool fullscreen) {
	int flags = 0;
	if (fullscreen)
		flags = SDL_WINDOW_FULLSCREEN;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		std::cout << "SDL Initialised" << std::endl;
		if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG))//initialize SDL2_images
			std::cout << "SDL Image Initialised" << std::endl;
		else fatalError("SDL_Image could not be initialised");

		//open window
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, flags);
		if (window) //check if created
			std::cout << "SDL Window Created" << std::endl;
		else
			fatalError("SDL Window could not be created");

		//create the renderer
		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer) //check if created
			std::cout << "SDL Renderer created" << std::endl;
		else
			fatalError("SDL Renderer could not be created");

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //set background to white

		isRunning = true;
	}
	else {
		fatalError("SDL could not be initialised");
	}
	initTextures();
	update(); //don't start with a empty screen
}

void MainGame::initTextures() {
	//create a texture where the fractal can be drawn on
	fractalTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, windowWidth, windowHeight);
	if (!fractalTex)
		fatalError("Fractal Texture could not be created\n");
}

void MainGame::createTexture(const char* filePath, SDL_Texture* tex) {
	SDL_Surface* tmpSurface = IMG_Load(filePath); //create temporary surface
	tex = SDL_CreateTextureFromSurface(renderer, tmpSurface); //turn surface into texture
	if (!tex) fatalError("Texture could not be loaded!"); //check if succesfull
	SDL_FreeSurface(tmpSurface); //free up the surface
}

//proccesses user input
void MainGame::processInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) { //while there is an event, (somethings happens)
		//see what event it is
		switch (event.type) {
		case SDL_QUIT: //when the red cross is pressed
			isRunning = false;
			break;
		case SDL_MOUSEMOTION:
			mousePos[0] = event.motion.x;
			mousePos[1] = event.motion.y;
			break;
		case SDL_KEYDOWN: //when a key is pressed
			//std::cout << event.key.keysym.sym << std::endl;
			if(keyPressed == 'c' && event.key.keysym.sym >= 48 && event.key.keysym.sym < 58) { //change the color if c+num is pressed
				color = (int)event.key.keysym.sym - 48;
				update();
				break;
			}
			switch(event.key.keysym.sym) { 
			case  13: //enter
				std::cout << "Enter real value\n";
				std::cin >> initialR;
				std::cout << "Enter Imaginary value\n";
				std::cin >> initialI;
				break;
			case 32: //spacebar
				update();
				break;
			case 27: //esc key
				//reset zoom
				xBound[0] = -1.5; xBound[1] = 1.5;
				yBound[0] = -1.5; yBound[1] = 1.5;
				break;
			case 115: //s
				saveTexture("FractalImage1.png", fractalTex);
				break;
			case 1073741904: //left arrow
				//move the bounds left
			{
				double range = xBound[1] - xBound[0];
				xBound[0] = xBound[0] - range * 0.2;
				xBound[1] = xBound[1] - range * 0.2;
				update();
			}
				break;
			case 1073741903: //right arrow
				//move the bounds right
			{
				double range = xBound[1] - xBound[0];
				xBound[0] = xBound[0] + range * 0.2;
				xBound[1] = xBound[1] + range * 0.2;
				update();
			}
				break;
			case 1073741906: //up arrow
				//move the bounds up
			{
				double range = yBound[1] - yBound[0];
				yBound[0] = yBound[0] - range * 0.2;
				yBound[1] = yBound[1] - range * 0.2;
				update();
			}
				break;
			case 1073741905: //down arrow
				//move the bounds down
			{
				double range = yBound[1] - yBound[0];
				yBound[0] = yBound[0] + range * 0.2;
				yBound[1] = yBound[1] + range * 0.2;
				update();
			}
				break;
			case 'j': //toggle julia set
				juliaSet = !juliaSet;
				buddhaSet = false;
				break;
			case 'b': //toggle buddha set
				buddhaSet = !buddhaSet;
				juliaSet = false;
				break;
			case 'r': //resize window
				std::cout << "\n-----------------------------------------\n"<<"\tResizeing window\n";
				std::cout << "Enter width\n";
				int w,h; //variables for the size
				std::cin >> w;
				std::cout << "Enter heigh\n";
				std::cin >> h;
				std::cout << "-----------------------------------------\n";
				SDL_SetWindowSize(window, w, h); //change size
				windowWidth = w; windowHeight = h; //change variables
				//destroy texture and create a new one
				SDL_DestroyTexture(fractalTex);
				initTextures();
				break;
			default: //Save while key is pressed
				keyPressed = event.key.keysym.sym;
				break;
			}
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == keyPressed) //reset the preset key
				keyPressed = 0;
			break;
		case SDL_MOUSEBUTTONDOWN: //when mouse is pressed
			if (event.button.button == SDL_BUTTON_LEFT) {
				// zoom in by 80 %
				zoomIn(event.button.x, event.button.y, 0.1);
			}
			if (event.button.button == SDL_BUTTON_RIGHT) {
				//print the coordinates
				std::cout << "Real = " << mapToReal(event.button.x);
				std::cout << "\tIm = " << mapToImaginary(event.button.y) << std::endl;
				if (keyPressed == 'g') { //if the g key is pressed
					initialR = mapToReal(event.button.x);
					initialI = mapToImaginary(event.button.y);
					update();
				}
			}
			break;
		case SDL_MOUSEWHEEL:
			if (event.wheel.y > 0) //zoom in
					zoomIn(mousePos[0], mousePos[1], event.wheel.preciseY / 4.0);
			if (event.wheel.y < 0) // zoom out
				zoomIn(mousePos[0], mousePos[1], -event.wheel.preciseY * 4);
			break;
		}
	}
}

//update
void MainGame::update() {
	//start tracking the time
	std::cout << "Start clocking\n";
	long double t0 = clock();
	//change renderer target (so draw on a texture)
	SDL_SetRenderTarget(renderer, fractalTex);
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
	//parameters
	int imageWidth = windowWidth;
	int imageHeight = windowHeight;
	int maxN = 500;
	if (buddhaSet) {
		CreateBuddha(maxN);
	}
	else {
		//loop over eacht pixel
		for (int y = 0; y < imageHeight; y++) { //each column
			for (int x = 0; x < imageWidth; x++) { //each row
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
				setColor(n, maxN); //set renderer draw color
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}
	}
	std::cout << "Time elapsed = " << clock() - t0 << std::endl;
	//reset target
	SDL_SetRenderTarget(renderer, NULL);
	render();
}

void MainGame::CreateBuddha(int maxN) {
	std::cout << "Starting Creating a BuddhaBrot, note that it will take some time.\n";
	std::cout << "Calculating:  0 %";
	//variables
	int maxI = 1000;
	//make an 2d array to store the density points
	std::vector<std::vector<uint16_t>> density;
	density.resize(windowWidth);
	for (int i = 0; i < windowWidth; i++)
		for (int j = 0; j < windowHeight; j++)
			density[i].push_back(0);

	//generate numbers for the x & y coords (real&im)
	std::vector<double> realNums=linspace(xBound[0],xBound[1],maxI);
	std::vector<double> ImNums = linspace(yBound[0],yBound[1],maxI);
	for (int i = 0; i < maxI; i++) {
		for (int j = 0; j < maxI; j++) {
			checkDensity(realNums[i], ImNums[j], maxN, density);
		}
		printf("\b\b\b\b%2i %%", i * 100 / maxI);
	}
	printf("\b\b\b\bDone\n");

	//find the maximum value
	uint16_t max = 0;
	for (std::vector<uint16_t> vect : density) {
		for (uint16_t num : vect) {
			if (num > max)
				max = num;
		}
	}
	std::cout << "The largest value = " << max << std::endl;

	//render everyting
	for (int i = 0; i < windowWidth; i++) {
		for (int j = 0; j < windowHeight; j++) {
			//SDL_SetRenderDrawColor(renderer, density[i][j]*(255.0/max), density[i][j] * (255.0 / max), density[i][j] * (255.0 / max), 1);
			setColor(density[i][j]*(255/max), 255);
			SDL_RenderDrawPoint(renderer, i, j);
		}
	}

	//saving to file 
}

//render to screen
void MainGame::render() {
	//clear renderer
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1); //set background to black
	SDL_RenderClear(renderer);
	//render the texture to the screen
	SDL_RenderCopy(renderer, fractalTex, NULL, NULL); //render texture to screen
	SDL_RenderPresent(renderer);
}

//save texture to file
void MainGame::saveTexture(const char* fileName, SDL_Texture* tex){
	std::cout << "Saving texture to file...\n";
	//set the texture as the renderer target
	SDL_SetRenderTarget(renderer, tex);
	//create a surface and put the pixels of the texture on the surface
	SDL_Surface* s = SDL_CreateRGBSurface(0, windowWidth, windowHeight, 32, 0, 0, 0, 0);
	if (!s) {
		std::cerr << "Image could not be saved, since the surface could not be created\n Error: " << SDL_GetError() << std::endl;
		return;
	}
	SDL_RenderReadPixels(renderer, NULL, s->format->format, s->pixels, s->pitch);
	//save to PNG
	IMG_SavePNG(s, fileName);
	//cleaning up
	SDL_FreeSurface(s);
	SDL_SetRenderTarget(renderer, NULL);
	std::cout << "Saving compled! saved as:" << fileName << std::endl;
}

void MainGame::zoomIn(int x, int y, double procent) {
	//Zoom in by ... procent (larger than 1 is zoom out)
	double tmp = mapToReal(x - procent * windowWidth);
	xBound[1] = mapToReal(x + procent * windowWidth);
	xBound[0] = tmp;
	tmp = mapToImaginary(y - procent * windowHeight);
	yBound[1] = mapToImaginary(y + procent * windowHeight);
	yBound[0] = tmp;
	//update screen
	update();
}

//clean the game
void MainGame::clean() {
	SDL_DestroyTexture(fractalTex); //destroy texture
	SDL_DestroyWindow(window); //destroy the window
	SDL_DestroyRenderer(renderer); //destroy the renderer
	SDL_Quit();
	std::cout << "Game cleaned" << std::endl;
} 