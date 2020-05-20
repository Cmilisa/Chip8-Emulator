#include "utils.h"

Window::Window()
{
	window = nullptr;
}

Window::Window(const char* title, int x, int y, int w, int h, Uint32 flags)
{
	window = nullptr;

	window = SDL_CreateWindow(title, x, y, w, h, flags);

	if (window == nullptr)
	{
		std::cout << "Error creating window\n";
	}
}

Window::~Window()
{
	if (window != nullptr)
	{
		SDL_DestroyWindow(window);
	}
}

SDL_Window* Window::get()
{
	return this->window;
}



Renderer::Renderer()
{
	renderer = nullptr;
}

Renderer::Renderer(SDL_Window* window, int index, Uint32 flags)
{
	renderer = nullptr;

	renderer = SDL_CreateRenderer(window, index, flags);

	if (renderer == nullptr)
	{
		std::cout << "Error creating renderer\n";
	}
}

Renderer::~Renderer()
{
	if (renderer != nullptr)
	{
		SDL_DestroyRenderer(renderer);
	}
}

SDL_Renderer* Renderer::get()
{
	return this->renderer;
}



Texture::Texture()
{
	texture = nullptr;
}

Texture::Texture(SDL_Renderer* renderer, Uint32 flags, int access, int w, int h)
{
	texture = nullptr;

	texture = SDL_CreateTexture(renderer, flags, access, w, h);

	if (texture == nullptr)
	{
		std::cout << "Error creating texture\n";
	}
}

Texture::~Texture()
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
	}
}

SDL_Texture* Texture::get()
{
	return this->texture;
}
