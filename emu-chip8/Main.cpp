#include <stdio.h>

#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>
#include <SFML\Graphics\Image.hpp>
#include <SFML\System\Thread.hpp>

#include "chip8.h"

void updateGraphics(sgb::Chip8 &c8);
sf::RenderWindow window(sf::VideoMode(640, 320), "CHIP-8 Emulator");
sf::Image back;
sf::RectangleShape backshape(sf::Vector2f(640, 320));
sf::Texture backtext;

int main() {
	sgb::Chip8 chip8;
	chip8.loadGame("PONG2");

	back.create(64, 32, sf::Color::Black);
	backshape.setPosition(0, 0);
	backtext.loadFromImage(back);
	backshape.setTexture(&backtext);

	while(window.isOpen()){
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		// emulate one cycle
		chip8.emulateCycle();

		// if drawflag is set, update screen;
		if (chip8.drawFlag)
			updateGraphics(chip8);

		//query keys
		chip8.setKeys();
		sf::sleep(sf::seconds(1.0f/500.0f));
	}

	return 0;
}

void updateGraphics(sgb::Chip8 &c8) {
	//window.clear();
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 32; j++) {
			int currpos = i + j*64;
			int on = c8.gfx[currpos];
			if (on == 1)
				back.setPixel(i, j, sf::Color::Green);
			else
				back.setPixel(i, j, sf::Color::Black);
		}
	}
	c8.drawFlag = false;
	backtext.update(back);
	window.draw(backshape);
	window.display();
}