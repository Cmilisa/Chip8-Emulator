#pragma once

#include <cstdint>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>

class Chip
{
	std::default_random_engine generator; // random number generator

	std::array<uint8_t, 4096> memory;
	std::array<uint16_t, 16> stack;
	std::array<uint8_t, 16> registers;
	uint16_t I = 0;
	uint16_t PC = 0x200;
	uint16_t SP = 0;

	std::array<uint32_t, 64 * 32> pixels; // array containing the pixel data, 0 if pixel is black, 1 if pixel is white

	const int width = 64; // width in pixels of the screen
	const int height = 32;

	std::array<uint8_t, 16> keys; // key[n] = 1 if is being pressed, 0 otherwise

	uint8_t delayTimer = 0; 
	uint8_t soundTimer = 0;


	std::array<uint8_t, 80> fontset =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // a
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // b
		0xF0, 0x80, 0x80, 0x80, 0xF0, // c
		0xE0, 0x90, 0x90, 0x90, 0xE0, // d
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // e
		0xF0, 0x80, 0xF0, 0x80, 0x80  // f
	};

	// opcodes function pointers
	uint16_t current_opcode = 0;

	// no-op operation to fill certain tabs
	void nop();

	// 0x0*** to 0xf*** opcodes
	void sec();
	void jmp();
	void call();
	void jiev();
	void jinev();
	void jier();
	void ldx();
	void add();
	void logic();
	void jiner();
	void setadr();
	void jr0v();
	void ran();
	void draw();
	void jmpkeys();
	void hexF();

	// 0x0*** sub-opcodes
	void clear();
	void ret();

	// 0x8*** sub-opcodes
	void ldxy();
	void orxy();
	void andxy();
	void xorxy();
	void addxy();
	void subxy();
	void shiftr();
	void subyx();
	void shiftl();

	// 0xe*** sub-opcodes
	void jik();
	void jink();

	typedef void (Chip::*instruction)();
	// main instruction tab
	instruction inst_tab[16] = { &Chip::sec, &Chip::jmp, &Chip::call, &Chip::jiev, &Chip::jinev, &Chip::jier, &Chip::ldx, &Chip::add, 
								 &Chip::logic, &Chip::jiner, &Chip::setadr, &Chip::jr0v, &Chip::ran, &Chip::draw, &Chip::jmpkeys, &Chip::hexF };

	// 0x0*** opcodes
	instruction chipSec[16] = { &Chip::clear, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop,
								&Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::ret, &Chip::nop };

	// logic instructions tab, 0x8*** opcodes
	instruction chipLogic[16] = { &Chip::ldxy, &Chip::orxy, &Chip::andxy, &Chip::xorxy, &Chip::addxy, &Chip::subxy, &Chip::shiftr, &Chip::subyx,
								  &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::shiftl, &Chip::nop };

	// key related instructions, 0xe*** opcodes
	instruction chipKeys[16] = { &Chip::nop, &Chip::jik, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop,
								 &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::nop, &Chip::jink, &Chip::nop };

	bool drawFlag = false;
	bool keyPressed = false;
	uint8_t lastPressedKey = 0;

public:
	Chip();
	Chip(const std::string& fileName);

	void loadROM(const std::string& fileName);

	void cycle();

	void pressKey(uint8_t key); // interface for pressing keys
	void releaseKey(uint8_t key); // interface for releasing keys
	
	const std::array<uint32_t, 64 * 32>& getPixels() const;
	bool isDrawing() const;
	int getWidth() const;
	int getHeight() const;
};