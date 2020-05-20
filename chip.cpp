#include "chip.h"

Chip::Chip() 
{ 
	// empty chip, all initialization is done by default member variables values, so all we have to do, is load
	// fontset data at the begining of the memory
	
	for (int i = 0; i < 64 * 32; ++i)
	{
		pixels[i] = 1;
	}

	memcpy(memory, fontset, 80 * sizeof(uint8_t));
}

Chip::Chip(const std::string& fileName)
{
	loadROM(fileName);

	// same as other constructor, we need to load fontset data into the memory

	for (int i = 0; i < 64 * 32; ++i)
	{
		pixels[i] = 1;
	}

	memcpy(memory, fontset, 80 * sizeof(uint8_t));
}

void Chip::loadROM(const std::string& fileName)
{
	std::ifstream file;
	file.open(fileName, std::ios::binary | std::ios::in /*| std::ios::ate*/);

	if (!file.is_open())
	{
		std::cout << "Error opening file " << fileName << "\n";
		return;
	}

	std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

	for (unsigned int i = 0; i < buffer.size(); ++i)
	{
		memory[0x200 + i] = (uint8_t)buffer[i];
	}

	file.close();

	std::cout << "Program " << fileName << " correctly loaded in memory\n";

}

void Chip::cycle()
{
	drawFlag = false; // reinitializing drawFlag each cylce

	uint16_t op = (memory[PC] << 8) + memory[PC + 1];

	std::cout << "opcode : " << std::hex << op << std::dec << "\n";
	//system("pause");

	switch (op & 0xf000) // TODO : change to have function pointers
	{
	case 0x0000:
		switch (op & 0x000f)
		{
		case 0x0: //clear display
		{
			std::cout << "Clearing display\n";
			for (int i = 0; i < 64 * 32; ++i)
			{
				pixels[i] = 1;
			}

			PC += 2;
			drawFlag = true;
			break;
		}

		case 0xe: //return from subroutine
		{
			--SP;
			PC = stack[SP] + 2;
			break;
		}

		default:
			std::cout << "Unknown opcode " << op << "\n";
		}
		break;

	case 0x1000: // 1NNN : jump to NNN
	{
		uint16_t target = op & 0x0fff;
		PC = target;
		std::cout << "jumping to " << std::hex << target << std::dec << "\n";
		break;
	}

	case 0x2000: // 2NNN : call subroutine NNN
	{
		uint16_t val = op & 0x0fff;
		stack[SP] = PC;
		++SP;
		PC = val;
		break;
	}

	case 0x3000: // 3XNN : if VX == NN skip next
	{
		uint8_t reg = (op & 0x0f00) >> 8;
		uint8_t val = (op & 0x0ff);
		if (registers[reg] == val)
			PC += 4;
		else
			PC += 2;
		break;
	}
	
	case 0x4000: // 4XNN : if VX != NN skip next
	{
		uint8_t reg = (op & 0x0f00) >> 8;
		uint8_t val = (op & 0x0ff);
		if (registers[reg] != val)
			PC += 4;
		else
			PC += 2;
		break;
	}

	case 0x5000: // 5XY0 : if VX == VY skip next
	{
		uint8_t regX = (op & 0x0f00) >> 8;
		uint8_t regY = (op & 0x00f0) >> 4;
		if (registers[regX] == registers[regY])
			PC += 4;
		else
			PC += 2;
		break;
	}

	case 0x6000: // 6XNN : VX = NN
	{
		uint8_t reg = (op & 0x0f00) >> 8;
		uint8_t val = (op & 0x00ff);
		registers[reg] = val;
		PC += 2;
		break;
	}

	case 0x7000: // 7XNN : VX += NN
	{
		uint8_t reg = (op & 0x0f00) >> 8;
		uint8_t val = (op & 0x00ff);
		registers[reg] = registers[reg] + val;
		PC += 2;
		break;
	}

	case 0x8000:
	{
		uint8_t regX = (op & 0x0f00) >> 8;
		uint8_t regY = (op & 0x00f0) >> 4;

		switch (op & 0x000f)
		{
		case 0x0: // 8XY0 : VX = VY
			registers[regX] = registers[regY];
			break;

		case 0x1: // 8XY1 : VX |= VY
			registers[regX] = registers[regX] | registers[regY];
			break;

		case 0x2: // 8XY2 : VX &= VY
			registers[regX] = registers[regX] & registers[regY];
			break;

		case 0x3: // 8XY3 : VX ^= VY
			registers[regX] = registers[regX] ^ registers[regY];
			break;

		case 0x4: // 8XY4 : VX += VY with carry
			if (registers[regY] + registers[regX] > 0xff)
				registers[0xf] = 1;
			else
				registers[0xf] = 0;
			registers[regX] = registers[regX] + registers[regY];
			break;

		case 0x5: // 8XY5 : VX -= VY with borrow
			if (registers[regY] > registers[regX])
				registers[0xf] = 0;
			else
				registers[0xf] = 1;
			registers[regX] = registers[regX] - registers[regY];
			break;

		case 0x6: // 8XY6 : VX >>= 1
			registers[0xf] = registers[regX] & 0b00000001;
			registers[regX] = registers[regX] >> 1;
			break;

		case 0x7: // 8XY7 : VX = VY - VX with borrow
			if (registers[regX] > registers[regY])
				registers[0xf] = 0;
			else
				registers[0xf] = 1;
			registers[regX] = registers[regY] - registers[regX];
			break;

		case 0xe: // 8XYE : VX <<= 1
			registers[0xf] = registers[regX] & 0b10000000;
			registers[regX] = registers[regX] << 1;
			break;

		default:
			std::cout << "Unknown opcode " << op << "\n";

		}
		PC += 2;
		break;
	}
		
	case 0x9000: // 9XY0 : if VX != VY skip next
	{
		uint8_t regX = (op & 0x0f00) >> 8;
		uint8_t regY = (op & 0x00f0) >> 4;
		if (registers[regX] != registers[regY])
			PC += 4;
		else
			PC += 2;
		break;
	}

	case 0xa000: // ANNN : I = NNN
	{
		uint16_t val = op & 0x0fff;
		I = val;
		PC += 2;
		break;
	}

	case 0xb000: // BNNN : PC = V0 + NNN
	{
		uint16_t val = op & 0x0fff;
		PC = registers[0x0] + val;
		break;
	}

	case 0xc000: // CXNN : VX = rand(0,255) & NN
	{
		uint8_t reg = (op & 0x0f00) >> 8;
		uint8_t val = op & 0x00ff;
		std::uniform_int_distribution<int> distrib(0, 255);
		uint8_t rand = (uint8_t)distrib(generator);
		registers[reg] = rand & val;
		PC += 2;
		break;
	}

	case 0xd000: // DXYN : draw sprite at (VX, VY) with N rows, at memory location I (I does not change, if pixels flipped, VF = 1, else 0)
	{
		uint8_t posx = (op & 0x0f00) >> 8;
		uint8_t posy = (op & 0x00f0) >> 4;
		uint8_t rows = (op & 0x000f);

		registers[0xf] = 0;

		for (int i = 0; i < rows; ++i) //i scans across rows
		{
			uint8_t line = memory[I + i];

			for (int j = 0; j < 8; ++j) //j scans across one single row
			{
				if ((line & (0x80 >> j)) != 0) // line & 0b1000 0000 scanning bit by bit, if != 0, then we render it
				{
					if (pixels[posx + j + ((posy + i) * 64)] != 0) // posx + j is (posx + j)th row of the screen, (posy + i)th column of the screen, * 64 to account for the format
						registers[0xf] = 1;

					//std::cout << "Unmodified pixel " << pixels[posx + j + ((posy + i) * 64)] << "\n";
					pixels[posx + j + ((posy + i) * 64)] ^= 1;
					//std::cout << "Modified pixel " << pixels[posx + j + ((posy + i) * 64)] << "\n";
				}
			}
		}
		PC += 2;
		drawFlag = true;
		break;
	}

	case 0xe000:
	{
		uint8_t reg = (op & 0x0f00) >> 8;
		switch (op & 0x000f)
		{
		case 0xe: // EX9E : if key stored in VX is pressed, skip next
			if (keys[registers[reg]] == 1)
				PC += 4;
			else
				PC += 2;
			break;

		case 0x1: // EXA1 : if key stored in VX is not pressed, skip next
			if (keys[registers[reg]] == 0)
				PC += 4;
			else
				PC += 2;
			break;

		default:
			std::cout << "Unknown opcode " << op << "\n";
		}
		break;
	}

	case 0xf000:
	{
		uint8_t reg = (op & 0x0f00) >> 8;
		switch (op & 0x00ff)
		{	
		case 0x07: // FX07 : VX = delay timer
			registers[reg] = delayTimer;
			break;

		case 0x0a: // FX0A : VX = key pressed
			if (keyPressed) // need to await for a key press, halts everything else until key is pressed
			{
				keys[lastPressedKey] = 1;
				registers[reg] = lastPressedKey;
				PC += 2;
				keyPressed = false;
			}
			break;

		case 0x15: // FX15 : delay timer = VX
			delayTimer = registers[reg];
			break;

		case 0x18: // FX18 : sound timer = VX
			soundTimer = registers[reg];
			break;

		case 0x1e: // FX1E : I += VX with range overflow
			if (I + registers[reg] > 0xfff)
				registers[0xf] = 1;
			else
				registers[0xf] = 0;
			I += registers[reg];
			break;

		case 0x29: // FX29 : set I to location of character in VX
		{
			uint8_t c = registers[reg];
			I = 5 * c; // each character is 4*5 pixels, so with 5 rows, so char 0 is at memory[0], char 1 is at memory[5], etc...
			PC += 2;
			break;
		}

		case 0x33: // FX33 : stores binary coded decimal representation of VX at adress I, I + 1 and I + 2
			memory[I] = registers[reg] / 100;
			memory[I + 1] = (registers[reg] / 100) % 10;
			memory[I + 2] = (registers[reg] % 100) % 10;
			break;

		case 0x55: //FX55 : stores registers from V0 to (including) VX in memory from adress I (I unmodified)
			for (int i = 0; i < reg; ++i)
			{
				memory[I + i] = registers[i];
			}
			break;

		case 0x65: //FX65 : loads registers from V0 to (including) VX with values starting at adress I
			for (int i = 0; i < reg; ++i)
			{
				registers[i] = memory[I + i];
			}
			break;

		default:
			std::cout << "Unknown opcode " << op << "\n";
		}
		PC += 2;
		break;
	}

	default:
		std::cout << "Unknown opcode " << op << "\n";
	}

	if (delayTimer > 0)
		--delayTimer;

	if (soundTimer > 0)
	{
		if (soundTimer == 1)
			std::cout << "BEEP\n";
		--soundTimer;
	}
}

void Chip::pressKey(uint8_t key)
{
	if (key >= 16 || key < 0)
	{
		std::cout << "Unknown key " << key << "\n";
		return;
	}
	keys[key] = 1;
	keyPressed = true;
}

void Chip::releaseKey(uint8_t key)
{
	if (key >= 16 || key < 0)
	{
		std::cout << "Unknown key " << key << "\n";
		return;
	}
	keys[key] = 0;
}

uint32_t* Chip::getPixels()
{
	for (int i = 0; i < 64 * 32; ++i)
	{
		if (!(pixels[i] == 1 || pixels[i] == 0))
		{
			std::cout << "Sanity check failed !\n" << pixels[i] << "\n";
			system("pause");
			return this->pixels;
		}
	}
	return this->pixels;
}

bool Chip::isDrawing()
{
	return this->drawFlag;
}