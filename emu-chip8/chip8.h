#ifndef SGB_EMU_CHIP8_H
#define SGB_EMU_CHIP8_H

#define STACKSIZE 16
#define MEMSIZE 4096
#define GFXSIZE 64*32
#define NUMFONTS 80
#define PROGSTART 0x200

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <time.h>

/* 
 * System memory map
 * 0x000 - 0x1FF - Chip 8 interpreter(contains font set in emu)
 * 0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
 * 0x200 - 0xFFF - Program ROM and work RAM
 */

namespace sgb {
	class Chip8 {
	public:
		Chip8();
		void loadGame(std::string filename); // load program into memory
		void emulateCycle(); // execute one cycle
		void setKeys(); // store key press state

		bool drawFlag; // set when screen should be redrawn

	private:
		uint16_t opcode; // current opcode
		uint8_t memory[MEMSIZE]; // chip8 memory
		uint8_t V[STACKSIZE]; // registers
		uint16_t I; // index register
		uint16_t pc; // program counter
		
		uint8_t gfx[GFXSIZE]; // pixels
		
		// timers count down to zero when set above zero
		// timers count at 60 hz
		uint8_t delay_timer;
		uint8_t sound_timer; // buzzer sounds when sound_timer reaches 0

		uint16_t stack[STACKSIZE];
		uint16_t sp; // stack pointer

		uint8_t key[STACKSIZE]; // keypad goes from 0x0 to 0xF

		void next(); // go to next instruction
		void skip(); // skip instruction
	};
}


#endif //SGB_EMU_CHIP8_H