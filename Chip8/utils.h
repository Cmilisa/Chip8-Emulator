#pragma once

#include<SDL2/SDL.h>
#include<iostream>

class Window
{
	SDL_Window* window;

public:
	Window();
	Window(const char* title, int x, int y, int w, int h, Uint32 flags);
	~Window();

	SDL_Window* get();
};

class Renderer
{
	SDL_Renderer* renderer;

public:
	Renderer();
	Renderer(SDL_Window* window, int index, Uint32 flags);
	~Renderer();

	SDL_Renderer* get();
};

class Texture
{
	SDL_Texture* texture;

public:
	Texture();
	Texture(SDL_Renderer* renderer, Uint32 format, int access, int w, int h);
	~Texture();

	SDL_Texture* get();
};
