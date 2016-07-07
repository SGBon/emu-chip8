#include "chip8.h"

namespace sgb {

	uint8_t chip8_fontset[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, //0
		0x20, 0x60, 0x20, 0x20, 0x70, //1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
		0x90, 0x90, 0xF0, 0x10, 0x10, //4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
		0xF0, 0x10, 0x20, 0x40, 0x40, //7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
		0xF0, 0x90, 0xF0, 0x90, 0x90, //A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
		0xF0, 0x80, 0x80, 0x80, 0xF0, //C
		0xE0, 0x90, 0x90, 0x90, 0xE0, //D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
		0xF0, 0x80, 0xF0, 0x80, 0x80  //F
	};

	Chip8::Chip8() :
		opcode(0),
		I(0),
		pc(PROGSTART),
		delay_timer(0),
		sound_timer(0),
		sp(0),
		drawFlag(true)
	{
		for (int i = 0; i < MEMSIZE; i++) {
			memory[i] = 0;
			if (i < GFXSIZE) {
				gfx[i] = 0;
				if (i < STACKSIZE) {
					V[i] = 0;
					stack[i] = 0;
					key[i] = 0;
				}
			}
		}

		for (int i = 0; i < 80; i++) {
			memory[i] = chip8_fontset[i];
		}

		// set keyboard mapping
		keymap[0] = sf::Keyboard::Num1;
		keymap[1] = sf::Keyboard::Num2;
		keymap[2] = sf::Keyboard::Num3;
		keymap[3] = sf::Keyboard::Num4;


		keymap[4] = sf::Keyboard::Q;
		keymap[5] = sf::Keyboard::W;
		keymap[6] = sf::Keyboard::E;
		keymap[7] = sf::Keyboard::R;

		keymap[8] = sf::Keyboard::A;
		keymap[9] = sf::Keyboard::S;
		keymap[10] = sf::Keyboard::D;
		keymap[11] = sf::Keyboard::F;

		keymap[12] = sf::Keyboard::Z;
		keymap[13] = sf::Keyboard::X;
		keymap[14] = sf::Keyboard::C;
		keymap[15] = sf::Keyboard::V;

		// load sound

		if (!sb.loadFromFile("../sound/tone.ogg")) {
			printf("could not find/read sound file, proceeding in silent mode\n");
		}
		else {
			tone.setBuffer(sb);
		}

		srand(time(NULL));
	}

	void Chip8::loadGame(std::string filename) {
		FILE* prog = fopen(filename.c_str(), "rb");
		if (prog!=NULL) {
			uint8_t buffer[MEMSIZE];
			size_t bufferSize = fread(buffer,1,MEMSIZE,prog);
			for (size_t i = 0; i < bufferSize; i++)
				memory[i + PROGSTART] = buffer[i];

			for (size_t i = 0x200; i < 0x200 + bufferSize; i+=2) {
				//printf("%x %d\n", memory[i] << 8 | memory[i+1],i);
			}
			fclose(prog);
		}
		else {
			printf("Error reading file\n");
		}
	}

	void Chip8::emulateCycle() {
		// fetch opcode
		opcode = memory[pc] << 8 | memory[pc + 1];
		
		// decode opcode
		switch (opcode & 0xF000) {

			// 0x0000 series opcodes
		case 0x0000:
			switch (opcode & 0x000F) {
			case 0x0000: // 0x00E0: Clears the screen
				for (int i = 0; i < GFXSIZE; i++)
					gfx[i] = 0;
				drawFlag = true;
				next();
				break;

			case 0x000E: // 0x00EE: Returns from subroutine
				--sp;
				pc = stack[sp];
				next();
				break;

			default:
				printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
				break;
			}
			break;

		case 0x1000: // 1NNN: jump to address NNN
			pc = opcode & 0x0FFF;
			break;

		case 0x2000: // 2NNN: call subroutine at address 2NNN
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
			break;

		case 0x3000: // 0x3XNN: skips next instruction if VX == NN
			if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				skip();
			else
				next();
			break;

		case 0x4000: // 0x4XNN: skips next instruction if VX != NN
			if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				skip();
			else
				next();
			break;

		case 0x5000: // 0x5XY0: skips next instruction if VX == VY
			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				skip();
			else
				next();
			break;

		case 0x6000: // 0x6XNN: sets VX to NN
			V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			next();
			break;

		case 0x7000: // 0x7XNN: adds NN to VX
			V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			next();
			break;

			// 0x8000 series opcodes
		case 0x8000:
			switch (opcode & 0x000F) {
			case 0x0000: // 0x8XY0: set VX to value of VY
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
				next();
				break;

			case 0x0001: // 0x8XY1: set VX to VX OR VY
				V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
				next();
				break;

			case 0x0002: // 0x8XY2: set VX to VX AND VY
				V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
				next();
				break;

			case 0x0003: // 0x8XY3: set VX to VX XOR VY
				V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
				next();
				break;

			case 0x0004: // 0x8XY4: add register Y to register X
				if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
					V[0xF] = 1; // carry
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
				next();
				break;

			case 0x0005: // 0x8XY5: Subtract VY from VX
				if (V[(opcode & 0x00F0) >> 4] > (V[(opcode & 0x0F00) >> 8]))
					V[0xF] = 0; // borrow
				else
					V[0xF] = 1;
				V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
				next();
				break;

			case 0x0006: // 0x8XY6: shift VX right by one
				V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x0001; // store least significant bit
				V[(opcode & 0x0F00) >> 8] >>= 1;
				next();
				break;

			case 0x0007: // 0x8XY7: set VX to VY minus VX
				if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
					V[0xF] = 0; // borrow
				else
					V[0xF] = 1;
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
				next();
				break;

			case 0x000E: // 0x8XY8: shift VX left by one
				V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7; // store most significant bit
				V[(opcode & 0x0F00) >> 8] <<= 1;
				next();
				break;

			default:
				printf("Unknown Opcode [0x8000]: 0x%X\n", opcode);
				break;
			}
			break;

		case 0x9000: // 0x9XY0: Skip next instruction if VX != VY
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				skip();
			else
				next();
			break;

		case 0xA000: // 0xANNN: Sets I to the address NNN
			I = opcode & 0x0FFF;
			next();
			break;

		case 0xB000: // 0xBNNN: Jumps to address NNN plus V0
			pc = (opcode & 0x0FFF) + V[0x0];
			break;

		case 0xC000: // 0xCXNN: set VX to random AND NN
			V[(opcode & 0x0F00) >> 8] = (rand()%255) & (opcode & 0x00FF);
			next();
			break;

		case 0xD000: // 0xDXYN: draw 8 pixel width sprite from (x,y) N pixels high
		{
			uint16_t x = V[(opcode & 0x0F00) >> 8];
			uint16_t y = V[(opcode & 0x00F0) >> 4];
			uint16_t height = (opcode & 0x000F);
			uint16_t pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++) {
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++) {
					if((pixel & (0x80 >> xline))!=0){
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
							V[0xF] = 1;
						gfx[x + xline + ((y + yline)*64)] ^= 1;
					}
				}
			}

			if(V[0xF] == 0)
				drawFlag = true;
			next();
		}
			break;

			//0xE000 series opcodes
		case 0xE000:
			switch (opcode & 0x00FF) {
			case 0x009E: //	EX9E: skip next instruction if key in VX is pressed
				if (key[V[(opcode & 0x0F00) >> 8]] != 0)
					skip();
				else
					next();
				break;

			case 0x00A1: // 0xEXA1: skip next isntruction if key in VX isn't pressed
				if (key[V[(opcode & 0x0F00) >> 8]] == 0)
					skip();
				else
					next();
				break;

			default:
				printf("Unknown opcode [0xE000]: 0x%X %d\n", opcode,pc);
				break;
			}
			break;

			//0xF000 series opcodes
		case 0xF000:
			switch (opcode & 0x00FF) {
			case 0x0007: // 0xFX07: set VX to value of the delay timer
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				next();
				break;

			case 0x000A: // 0xFX0A: await keypress, then store in VX
			{
				bool keyPress = false;
				for (int i = 0; i < STACKSIZE; i++) {
					if (key[i] != 0) {
						V[(opcode & 0x0F00) >> 8] = i;
						keyPress = true;
					}
				}
				// repeat cycle if no key was pressed
				if (!keyPress)
					return;
				next();
			}
				break;

			case 0x0015: // 0xFX15: set delay timer to VX
				delay_timer = V[(opcode & 0x0F00) >> 8];
				next();
				break;

			case 0x0018: // 0xFX18: set sound timer to VX
				sound_timer = V[(opcode & 0x0F00) >> 8];
				next();
				break;

			case 0x001E: // 0xFX1E: add VX to I
				if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
					V[0xF] = 1;
				else
					V[0xF] = 0;
				I += V[(opcode & 0x0F00) >> 8];
				next();
				break;

			case 0x0029: // 0xFX29: Set I to location of character sprite in VX
				I = V[(opcode & 0x0F00) >> 8] * 0x5;
				next();
				break;

			case 0x0033: // 0xFX33: store binary coded decimal of VX in memory location at I
				memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
				memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
				memory[I+2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
				next();
				break;

			case 0x0055: // 0xFX55: store V0 to VX in memory starting at address I
			{
				int last = (opcode & 0x0F00) >> 8;
				for (int i = 0; i <= last; i++) {
					memory[I + i] = V[i];
				}
				I += last + 1;
			}
				next();
				break;

			case 0x0065: // 0xFX65: fill V0 to VX with values from memory at I
			{
				int last = (opcode & 0x0F00) >> 8;
				for (int i = 0; i <= last; i++) {
					V[i] = memory[I + i];
				}
				I += last + 1;
			}
				next();
				break;

			default:
				printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
				break;
			}
			break;

		default:
			printf("unknown opcode: 0x%X\n", opcode);
			break;
		}

		// update timers
		if (delay_timer > 0)
			--delay_timer;

		if (sound_timer > 0) {
			playSound();
			--sound_timer;
		}
	}

	void Chip8::setKeys() {
		for (int i = 0; i < STACKSIZE; i++)
			key[i] = 0;

		for (int i = 0; i < STACKSIZE; i++) {
			if (sf::Keyboard::isKeyPressed(keymap[i]))
				key[i] = 1;
		}
	}

	void Chip8::playSound() {
		if (tone.getBuffer() && tone.getStatus() == tone.Stopped)
			tone.play();
	}

	void Chip8::next() {
		pc += 2;
	}

	void Chip8::skip() {
		pc += 4;
	}
}