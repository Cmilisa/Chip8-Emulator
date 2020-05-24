#include "chip.h"

Chip::Chip() 
{ 
	// initializing all there is to initialize
	
	// memory
	for (auto& a : memory)
	{
		a = 0;
	}
	// copying font data to the unused memory
	std::copy(fontset.begin(), fontset.end(), memory.begin());

	// stack
	for (auto& a : stack)
	{
		a = 0;
	}

	// registers
	for (auto& a : registers)
	{
		a = 0;
	}

	// pixel data
	for (auto& a : pixels)
	{
		a = 1;
	}

	// key presses
	for (auto& a : keys)
	{
		a = 0;
	}
}

Chip::Chip(const std::string& fileName)
{
	for (auto& a : memory)
	{
		a = 0;
	}
	
	std::copy(fontset.begin(), fontset.end(), memory.begin());

	for (auto& a : stack)
	{
		a = 0;
	}

	for (auto& a : registers)
	{
		a = 0;
	}

	for (auto& a : pixels)
	{
		a = 1;
	}
	
	for (auto& a : keys)
	{
		a = 0;
	}

	// loading rom into the memory
	loadROM(fileName);
}

void Chip::loadROM(const std::string& fileName)
{
	std::ifstream file;
	file.open(fileName, std::ios::binary | std::ios::in);

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


// ======================================
// MAIN OPCODES FUNCTIONS
// ======================================


void Chip::nop() { return; }

void Chip::sec()
{
	instruction op = chipSec[(current_opcode & 0x00f)];
	(this->*op)();
}

void Chip::jmp()
{
	// 1NNN : jump to NNN
	uint16_t target = current_opcode & 0x0fff;
	PC = target;
}

void Chip::call()
{
	// 2NNN : call subroutine NNN
	uint16_t val = current_opcode & 0x0fff;
	stack[SP] = PC;
	++SP;
	PC = val;
}

void Chip::jiev()
{
	// 3XNN : if VX == NN skip next
	uint8_t reg = (current_opcode & 0x0f00) >> 8;
	uint8_t val = (current_opcode & 0x0ff);
	if (registers[reg] == val)
		PC += 4;
	else
		PC += 2;
}

void Chip::jinev()
{
	// 4XNN : if VX != NN skip next
	uint8_t reg = (current_opcode & 0x0f00) >> 8;
	uint8_t val = (current_opcode & 0x0ff);
	if (registers[reg] != val)
		PC += 4;
	else
		PC += 2;
}

void Chip::jier()
{
	// 5XY0 : if VX == VY skip next
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;
	if (registers[regX] == registers[regY])
		PC += 4;
	else
		PC += 2;
}

void Chip::ldx()
{
	// 6XNN : VX = NN
	uint8_t reg = (current_opcode & 0x0f00) >> 8;
	uint8_t val = (current_opcode & 0x00ff);
	registers[reg] = val;
	PC += 2;
}

void Chip::add()
{
	// 7XNN : VX += NN
	uint8_t reg = (current_opcode & 0x0f00) >> 8;
	uint8_t val = (current_opcode & 0x00ff);
	registers[reg] = registers[reg] + val;
	PC += 2;
}

void Chip::logic()
{
	instruction op = chipLogic[(current_opcode & 0x00f)]; 
	(this->*op)();
}

void Chip::jiner()
{
	// 9XY0 : if VX != VY skip next
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;
	if (registers[regX] != registers[regY])
		PC += 4;
	else
		PC += 2;
}

void Chip::setadr()
{
	// ANNN : I = NNN
	uint16_t val = current_opcode & 0x0fff;
	I = val;
	PC += 2;
}

void Chip::jr0v()
{
	// BNNN : PC = V0 + NNN
	uint16_t val = current_opcode & 0x0fff;
	PC = registers[0x0] + val;
}

void Chip::ran()
{
	// CXNN : VX = rand(0,255) & NN
	uint8_t reg = (current_opcode & 0x0f00) >> 8;
	uint8_t val = current_opcode & 0x00ff;
	std::uniform_int_distribution<int> distrib(0, 255);
	uint8_t rand = (uint8_t)distrib(generator);
	registers[reg] = rand & val;
	PC += 2;
}

void Chip::draw()
{
	// DXYN : draw sprite at (VX, VY) with N rows, at memory location I (I does not change, if pixels flipped, VF = 1, else 0)
	uint8_t posx = registers[(current_opcode & 0x0f00) >> 8];
	uint8_t posy = registers[(current_opcode & 0x00f0) >> 4];
	uint8_t rows = (current_opcode & 0x000f);

	registers[0xf] = 0;

	for (int i = 0; i < rows; ++i) //i scans across rows
	{
		uint8_t line = memory[I + i];

		for (int j = 0; j < 8; ++j) //j scans across one single row
		{
			if ((line & (0x80 >> j)) != 0) // line & 0b1000 0000 scanning bit by bit, if != 0, then we render it
			{
				if (pixels[posx + j + ((posy + i) * 64)] == 1) // posx + j is (posx + j)th row of the screen, (posy + i)th column of the screen, * 64 to account for the format
					registers[0xf] = 1;

				pixels[posx + j + ((posy + i) * 64)] ^= 1;
			}
		}
	}
	PC += 2;
	drawFlag = true;
}

void Chip::jmpkeys()
{
	instruction op = chipKeys[(current_opcode & 0x00f)];
	(this->*op)();
}

void Chip::hexF()
{
	uint8_t reg = (current_opcode & 0x0f00) >> 8;
	switch (current_opcode & 0x00ff)
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
		memory[I] = uint8_t(registers[reg] / 100);
		memory[I + 1] = (registers[reg] / 10) % 10;
		memory[I + 2] = (registers[reg] % 100) % 10;
		break;

	case 0x55: //FX55 : stores registers from V0 to (including) VX in memory from adress I (I unmodified)
		for (int i = 0; i <= reg; ++i)
		{
			memory[I + i] = registers[i];
		}
		break;

	case 0x65: //FX65 : loads registers from V0 to (including) VX with values starting at adress I
		for (int i = 0; i <= reg; ++i)
		{
			registers[i] = memory[I + i];
		}
		break;

	default:
		std::cout << "Unknown opcode " << current_opcode << "\n";
	}
	PC += 2;
}

// ======================================
// 0x0*** sub opcodes
// ======================================

void Chip::clear()
{
	// 00E0 : Clear display
	for (auto& a : pixels)
	{
		a = 1;
	}

	PC += 2;
	drawFlag = true;
}

void Chip::ret()
{
	// 00EE : return from subroutine
	--SP;
	PC = stack[SP] + 2;
}

// ======================================
// 0x8*** sub opcodes
// ======================================

void Chip::ldxy()
{ 
	// 8XY0 : VX = VY
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	registers[regX] = registers[regY];

	PC += 2;
}

void Chip::orxy()
{ 
	// 8XY1 : VX |= VY
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	registers[regX] = registers[regX] | registers[regY];

	PC += 2;
}

void Chip::andxy()
{
	// 8XY2 : VX &= VY
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	registers[regX] = registers[regX] & registers[regY];

	PC += 2;
}

void Chip::xorxy()
{ 
	// 8XY3 : VX ^= VY
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	registers[regX] = registers[regX] ^ registers[regY];

	PC += 2;
}

void Chip::addxy()
{ 
	// 8XY4 : VX += VY with carry
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	if (registers[regY] + registers[regX] > 0xff)
		registers[0xf] = 1;
	else
		registers[0xf] = 0;
	registers[regX] = registers[regX] + registers[regY];

	PC += 2;
}

void Chip::subxy()
{
	// 8XY5 : VX -= VY with borrow
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	if (registers[regY] > registers[regX])
		registers[0xf] = 0;
	else
		registers[0xf] = 1;
	registers[regX] = registers[regX] - registers[regY];

	PC += 2;
}

void Chip::shiftr()
{ 
	// 8XY6 : VX >>= 1
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	registers[0xf] = registers[regX] & 0b00000001;
	registers[regX] = registers[regX] >> 1;

	PC += 2;
}

void Chip::subyx()
{
	// 8XY7 : VX = VY - VX with borrow
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	if (registers[regX] > registers[regY])
		registers[0xf] = 0;
	else
		registers[0xf] = 1;
	registers[regX] = registers[regY] - registers[regX];

	PC += 2;
}

void Chip::shiftl()
{ 
	// 8XYE : VX <<= 1
	uint8_t regX = (current_opcode & 0x0f00) >> 8;
	uint8_t regY = (current_opcode & 0x00f0) >> 4;

	registers[0xf] = registers[regX] & 0b10000000;
	registers[regX] = registers[regX] << 1;

	PC += 2;
}


// ======================================
// 0xe*** sub opcodes
// ======================================

void Chip::jik()
{
	// EX9E : if key stored in VX is pressed, skip next
	uint8_t reg = (current_opcode & 0x0f00) >> 8;

	if (keys[registers[reg]] == 1)
		PC += 4;
	else
		PC += 2;
}

void Chip::jink()
{
	// EXA1 : if key stored in VX is not pressed, skip next
	uint8_t reg = (current_opcode & 0x0f00) >> 8;

	if (keys[registers[reg]] == 0)
		PC += 4;
	else
		PC += 2;
}

// ======================================

void Chip::cycle()
{
	drawFlag = false; // reinitializing drawFlag each cylce

	current_opcode = (memory[PC] << 8) + memory[PC + 1]; // fetch opcode

	instruction op = inst_tab[((current_opcode & 0xf000) >> 12)]; // fetch op corresponding to the opcode
	(this->*op)(); // execute the op

	if (delayTimer > 0)
		--delayTimer;

	if (soundTimer > 0)
	{
		if (soundTimer == 1)
			std::cout << "BEEP\n";
		--soundTimer;
	}
}

void Chip::pressKey(uint8_t key) // interface for the cpu to register pressed keys
{
	if (key >= 16 || key < 0)
	{
		std::cout << "Unknown key " << key << "\n"; // debug output
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

const std::array<uint32_t, 64 * 32>& Chip::getPixels() const
{
	return this->pixels;
}

bool Chip::isDrawing() const
{
	return this->drawFlag;
}

int Chip::getWidth() const
{
	return this->width;
}

int Chip::getHeight() const
{
	return this->height;
}