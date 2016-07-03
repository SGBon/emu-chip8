#include <stdio.h>

#include "chip8.h"

int main() {
	sgb::Chip8 chip8;
	chip8.loadGame("PONG");

	for (;;) {
		// emulate one cycle
		chip8.emulateCycle();

		// if drawflag is set, update screen;
		if (chip8.drawFlag);

		//query keys
		chip8.setKeys();
	}

	return 0;
}