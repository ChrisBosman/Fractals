#include "MainGame.h"

template<typename T>
struct xyVec
{
	T x, y;
	xyVec() {
		x = 0;
		y = 0;
	}
	xyVec(T _x, T _y) {
		x = _x; y = _y;
	}
	bool operator==(xyVec second) {
		return (this->x == second.x && this->y == second.y);
	}
};

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
	return int((real - xBound[0]) * (windowWidth / range));
}

int MainGame::mapFromImaginary(double im) {
	double range = yBound[1] - yBound[0];
	return int((im - yBound[0]) * (windowHeight / range));
}

//check if points are stable with recursion
int MainGame::checkStability(double cReal, double cIm, int N, double zReal = 0, double zIm = 0) {
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
			std::cout << "No fractal function selected, so just drawing default Mandelbrot\n";
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
void mapHsvToRGB(int* RGB, int H, double V, float S) {
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
	double X = C * (1 - abs(fmod((H / 60.0),2) - 1));
	double m = V - C;
	// Calculate the values for R,G and B
	if (H == 299) {
		;
	}
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
		mapHsvToRGB(RGB, cos(100*n)*360, (double)n * n / maxN, (double)n * n / maxN);
		break;
	case 7: //multible colors, calmer
		mapHsvToRGB(RGB, n * 360 /maxN , (double)n*n/maxN, (double)n*n / maxN);
		break;
	case 8: //multible colors, more
		mapHsvToRGB(RGB, n*5, (double)n * n / maxN, (double)n * n / maxN);
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
	//create the textures for the menu's
	createTexture("assets/Menu.png", escMenuTex);
	//create texture for the function menu
	createTexture("assets/FractalFunctions.png", funcMenuTex);
}

void MainGame::createTexture(const char* filePath, SDL_Texture* &tex) {
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
		if (state == State::escMenu) {
			processEscMenuInput(event);
			continue;
		}
		if (state == State::functionMenu) {
			processFunMenuInput(event);
			continue;
		}
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
			else if (keyPressed == 't' && event.key.keysym.sym >= 48 && event.key.keysym.sym < 58) { //change the color if c+num is pressed
				amountOfThreads = (int)event.key.keysym.sym - 48;
				if (amountOfThreads % 2 != 0 && amountOfThreads != 1) //check if not even
					amountOfThreads = amountOfThreads - 1;
				if (amountOfThreads > 8 || amountOfThreads < 1) //limit the threads to 8
					amountOfThreads = 1;
				update();
				break;
			}
			else if (keyPressed == 'z') {
				switch (event.key.keysym.sym)
				{
				case 'i':
					zoomIn(windowWidth / 2, windowHeight / 2, 0.7);
					break;
				case 'o':
					zoomIn(windowWidth / 2, windowHeight / 2, 1.3);
					break;
				}
				continue;
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
				//open esc menu
				state = State::escMenu;
				break;
			case 115: //s
				saveTexture("output/FractalImage1.png", fractalTex);
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
				update();
				break;
			case 'b': //toggle buddha set
				buddhaSet = !buddhaSet;
				juliaSet = false;
				update();
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
				update();
				break;
			case 'n':
				std::cout << "Enter a new value for the maximum number of itterations,\n the higher the better but it will be slower\n (previous value was:" << maxN << ")\n";
				std::cin >> maxN;
				update();
				break;
			case 'a':
				makeAnimation();
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
				zoomIn(event.button.x, event.button.y, 0.2);
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
				zoomIn(mousePos[0], mousePos[1], -(double)event.wheel.preciseY * 4.0);
			break;
		}
	}
}

//process input for the esc menu
void MainGame::processEscMenuInput(SDL_Event& event) {
	switch (event.type) {
	case SDL_QUIT: //when the red cross is pressed
		isRunning = false;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == 27)
			state = State::fractal;
		break;
	case SDL_MOUSEBUTTONDOWN:
		//check if x coords are in within the xsize of the buttons
		if (event.button.x > 0.25*windowWidth && event.button.x < 0.75*windowWidth) {
			//check y coords
			int num = ceil( (event.button.y - 0.1 * windowHeight) / (0.2*windowHeight));
			switch (num) {
			case 1: //continue
				state = State::fractal;
				break;
			case 2: //reset view
				xBound[0] = -2; xBound[1] = 1;
				yBound[0] = -1.5; yBound[1] = 1.5;
				state = State::fractal;
				update();
				break;
			case 3: //funtion changing screen
				state = State::functionMenu;
				break;
			case 4: //quit
				isRunning = false;
				break;
			}
		}
			
		break;
	}
}

//procces input for the function changing menu
void MainGame::processFunMenuInput(SDL_Event& event) {
	switch (event.type)
	{
	case SDL_QUIT: //when the red cross is pressed
		isRunning = false;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == 27)
			state = State::escMenu;
		break;
	case SDL_MOUSEBUTTONDOWN:
		//See if and which button is pressed
		//divide the screen into rows the size of the buttons and calculate the row that has been clicked
		int num = ceil((event.button.y-0.09*windowHeight)/(0.2*windowHeight));
		if (num > 4) //if out of bounds
			break;
		//Increase num by adding 4*columnNumber
		num += 4 * (ceil((event.button.x - 0.01 * windowWidth) / (0.49 * windowWidth)) - 1);

		//update the selected fraction function setting
		fracfunct = num;
		//close the menu
		state = State::fractal;
		update();
		break;
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
	if (buddhaSet) {
		int maxN = 50;
		CreateBuddha(maxN);
	}
	else {
		// Divide up the pixels over the threads
		std::vector<std::thread> threads;
		// Make a vector for the stability values and resize it, (NOTE it is unfolded)
		std::vector<std::vector<int>*> stabilityVal; 
		stabilityVal.resize(amountOfThreads);
		for (int i = 0; i < stabilityVal.size(); i++)
			stabilityVal[i] = new std::vector<int>((int)((double)windowHeight*((double)imageWidth / amountOfThreads)));

		for (int i = 0; i < amountOfThreads; i++) {
			int xBegin =(int)( i * ((double)imageWidth / amountOfThreads)); 
			int xEnd = (int)((i + 1) * ((double)imageWidth / amountOfThreads));
			// Generate the object that will be put in the thread
			GenerateImage functorGen = GenerateImage(renderer, windowWidth, windowHeight, xBound, yBound, fracfunct, color);
			// Start the thread
			threads.push_back(std::thread(functorGen, imageHeight, maxN, xBegin, xEnd, stabilityVal[i], initialR, initialI, juliaSet));
		}
		// Join the strings
		for (int i = 0; i < threads.size(); i++)
			threads[i].join();
		
		// Render everything
		for (int i = 0; i < stabilityVal.size(); i++){		
			int beginX = (int)(i * ((double)imageWidth / amountOfThreads));
			for (int j = 0; j < stabilityVal[i]->size(); j++) {
				setColor(stabilityVal[i]->at(j), maxN); //set renderer draw color
				SDL_RenderDrawPoint(renderer, (int)j / windowHeight + beginX, j % windowHeight);
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
	//W.I.P. probably for forever
}

//render to screen
void MainGame::render() {
	//clear renderer
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1); //set background to black
	SDL_RenderClear(renderer);
	//render the texture to the screen
	SDL_RenderCopy(renderer, fractalTex, NULL, NULL); //render texture to screen
	switch (state) {
	case State::escMenu:
		SDL_RenderCopy(renderer, escMenuTex, NULL, NULL);
		break;
	case State::functionMenu:
		SDL_RenderCopy(renderer, funcMenuTex, NULL, NULL);
		break;
	}
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
	double tmp = mapToReal(x - (procent * windowWidth)/2);
	xBound[1] = mapToReal(x + (procent * windowWidth)/2);
	xBound[0] = tmp;
	tmp = mapToImaginary(y - (procent * windowHeight)/2);
	yBound[1] = mapToImaginary(y + (procent * windowHeight)/2);
	yBound[0] = tmp;
	//update screen
	update();
}

//Zoom in using the complex coordinates
void MainGame::zoomInComplex(double x, double y, double procent) {
	//Zoom in by ... procent (larger than 1 is zoom out)
	double xRange = xBound[1] - xBound[0];
	xBound[0] = x - procent*xRange/2;
	xBound[1] = x + procent*xRange/2;
	double yRange = yBound[1] - yBound[0];
	yBound[0] = y - procent*yRange/2;
	yBound[1] = y + procent*yRange/2;
	//update screen
	update();
}

//Generate images over a path		Maybe also add rotation
void MainGame::makeAnimation(){
	//Disable buddhaset since it will take too long
	buddhaSet = false;
	//Print the settings explaination
	std::cout << "______________________________________________________\n";
	std::cout << "Setup to generate alot of images\nEnter the following settings with an [Enter] in between,\nmake sure that the screen is now at the first point\n(Type -1 to quit)\n\n"
		<< "Zoom per step (0.1 = zoom until only 10% is visible)\n"
		<< "Maximum number of itteration, 0 = keep current value (first point [space] Second point)\n"
		<< "Number of frames\n"
		<< "Screen resolution 0=keep current\n"
		<< "Name of file, max 40 characters ('0' will lead to the default name)\n"
		<< "Change initial values (0=no, 1=linear, 2=circualr)\n"
		<< "Use the mouse to set the second point (0=no,1=yes)\n";
	
	// Make variables for the settings
	double zoomStep;
	int frames;
	int w, h; //variables for the size
	xyVec<double> pos0 = xyVec<double>(); //Start position
	pos0.x = xBound[0] + (xBound[1] - xBound[0]) / 2;
	pos0.y = yBound[0] + (yBound[1] - yBound[0]) / 2;
	xyVec<double> pos1 = pos0; //end position
	int changeIntialValues;
	xyVec<double> initialChange = xyVec<double>();  // The relative change in the intial Real and Imag
	double radiusInit; //radius of the circular path of the initial values
	std::pair<int, int> maxItterations;
	std::string fileName;
	const unsigned short maxCharacters = 50;



	// Ask for input
	std::cin >> zoomStep;
	if (zoomStep == -1) //if -1 was entered at the start, quit.
		return;
	std::cin >> maxItterations.first >> maxItterations.second >> frames >> w >> h >> fileName >> changeIntialValues;
	if (maxItterations.first == 0)
		maxItterations.first = maxN;
	if (maxItterations.second == 0)
		maxItterations.second = maxN;
	if (frames < 1)
		frames = 1;
	if (fileName == "0" || fileName.size() > maxCharacters-4)
		fileName = "Frame";
	if (fileName.size() + ceil(log10((frames + 1))) > maxCharacters)
		fileName = "F"; 
	
	// Use mouse to set the second point
	{
		int tmp;
		std::cin >> tmp;
		switch (tmp)
		{
		case 1:
			//Use mouse to set second point
			std::cout << "Left click for the last position\n";
			while (pos1 == pos0) { //while the position is not set
				SDL_Event event;
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_MOUSEBUTTONDOWN) {
						if (event.button.button == SDL_BUTTON_LEFT) {
							pos1.x = mapToReal(event.button.x);
							pos1.y = mapToImaginary(event.button.y);
						}
					}
				}
			}
			break;
		case -1:
			return;
		}
	}

	// Change the initial values
	if (changeIntialValues < 1) {
		initialChange.x = 0;
		initialChange.y = 0;
	}
	else {
		switch (changeIntialValues) {
		case 1:  // Linear
			std::cout << "Relative change to initial real value (the value is now: " << initialR << ")\n"
				<< "Relative change to initial imaginary value (the value is now: " << initialI << ")\n";
			std::cin >> initialChange.x >> initialChange.y;
			break;
		case 2:  // Circular
			std::cout << "Circular path, the center points will be the value that it is now.\n Enter the radius, (real value = " << initialR << ", imag value = " << initialI << ")\n";
			std::cin >> radiusInit;
			initialChange.x = initialR;  // Reusing this variable for storing the centre point
			initialChange.y = initialI;
		}
	}

	// Rescale the window
	if (w != 0 && h != 0) {
		SDL_SetWindowSize(window, w, h); //change size
		windowWidth = w; windowHeight = h; //change variables
		//destroy texture and create a new one
		SDL_DestroyTexture(fractalTex);
		initTextures();
	}

	//Precalculate some stuff
	double dx = (pos1.x - pos0.x) / frames;
	double dy = (pos1.y - pos0.y) / frames;
	double dN = (maxItterations.second - maxItterations.first) / (double)frames;

	std::cout << "Starting to generate " << frames << "\n";
	update(); //The first frame has to be updated in advance
	int frameNum; // Current frame number
	for (frameNum = 0; frameNum < frames; frameNum++) {
		if (frameNum > 0) { //don't move the first frame
			//Update the maximum amount of itterations
			maxN += dN;
			//Update the initial values
			switch (changeIntialValues) {
			case 1:  // Linear
				initialR += initialChange.x / frames;
				initialI += initialChange.y / frames;
				break;
			case 2: // Circular
				initialR = initialChange.x + radiusInit * cos((double)frameNum / frames * 2 * M_PI);
				initialI = initialChange.y + radiusInit * sin((double)frameNum / frames * 2 * M_PI);
				break;
			}
			//Get the x and y values of the frame
			double x = dx * frameNum + pos0.x;
			double y = dy * frameNum + pos0.y;
			zoomInComplex(x,y,zoomStep); //zoom in, move and update
		}
		std::string finalFileName = "output/" + fileName + std::to_string(frameNum) + ".png";
		saveTexture(finalFileName.c_str(), fractalTex);
	}
}

//clean the game
void MainGame::clean() {
	//destroy textures
	SDL_DestroyTexture(fractalTex); 
	SDL_DestroyTexture(escMenuTex);
	SDL_DestroyTexture(funcMenuTex);
	//destroy other things
	SDL_DestroyWindow(window); //destroy the window
	SDL_DestroyRenderer(renderer); //destroy the renderer
	SDL_Quit();
	std::cout << "Game cleaned" << std::endl;
} 