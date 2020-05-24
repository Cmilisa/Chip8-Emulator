#include "chip.h"
#include "utils.h"


int main(int argc, char* args[])
{
	Chip test("test_opcode.ch8"); // test rom to verify all opcodes work properly
	const int chipWidth = test.getWidth();
	const int chipHeight = test.getHeight();

	SDL_Init(SDL_INIT_VIDEO);

	bool quit = false;
	int scale = 8; // the base resolution is very small, so we scale it up

	Window win("Chip8 emulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, chipWidth * scale, chipHeight * scale, 0);
	Renderer ren(win.get(), -1, 0);
	Texture tex(ren.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 32);
	
	SDL_Event event;
	SDL_PixelFormat *format;
	format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);

	while (!quit)
	{
		test.cycle(); // simulate one cycle

		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		if (test.isDrawing())
		{
			SDL_RenderClear(ren.get());
			const std::array<uint32_t, 64 * 32> chipPixels = test.getPixels();

			Uint32 pixels[64 * 32] = { 0 };
			
			for (int i = 0; i < 64 * 32; ++i) // updating screen texture
			{
				pixels[i] = SDL_MapRGB(format, chipPixels[i] * 255, chipPixels[i] * 255, chipPixels[i] * 255);
			}


			int pitch = format->BytesPerPixel * 64;
			SDL_UpdateTexture(tex.get(), NULL, pixels, pitch);
			SDL_RenderCopy(ren.get(), tex.get(), NULL, NULL);
			SDL_RenderPresent(ren.get());
		}

		SDL_Delay(33); // should be 30 FPS, ugly, remplace with something better
	}

	SDL_FreeFormat(format);
	SDL_Quit();

	return 0;
}