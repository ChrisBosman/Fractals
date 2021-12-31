#include <iostream>
#include "MainGame.h"
#include <ctime>
#include "ComplexNum.h"

int main(int argc, char** argv) {
	srand(time(0)); //made a seed for the rand function
	MainGame* mainGame;
	mainGame = new MainGame(512, 512); //1920, 1010      //1920,1080    //1024, 768  //3440, 1440
	mainGame->initSystems("Fractals", false);

	while (mainGame->running()) {

		mainGame->processInput();
		//mainGame->update();
		mainGame->render();

	}

	mainGame->clean();
	return 0;
	delete(mainGame);
}