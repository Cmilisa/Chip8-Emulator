#include "chip.h"
#include "utils.h"

int main(int argc, char* args[])
{
	SDL_Init(SDL_INIT_VIDEO);

	bool quit = false;
	int scale = 8;

	Window win("Chip8 emulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64 * scale, 32 * scale, 0);
	Renderer ren(win.get(), -1, 0);
	Texture tex(ren.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 32);

	SDL_SetRenderDrawColor(ren.get(), 255, 255, 255, 255);

	//memset(pixels, 255, 64 * 32 * sizeof(Uint32));

	Chip test("test_opcode.ch8");

	SDL_Event event;

	while (!quit)
	{
		test.cycle();

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
			//SDL_SetRenderDrawColor(ren.get(), 255, 255, 255, 255);
			uint32_t* chipPixels = test.getPixels(); // should have 64 * 32 elements

			Uint32 pixels[64 * 32] = { 0 };
			SDL_PixelFormat *format;
			format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);

			for (int i = 0; i < 32; ++i)
			{
				for (int j = 0; j < 64; ++j)
				{
					SDL_SetRenderDrawColor(ren.get(), (Uint8)(pixels[i * 32 + j] * 255), (Uint8)(pixels[i * 32 + j] * 255), (Uint8)(pixels[i * 32 + j] * 255), 255);
					if(pixels[i * 32 + j] != 0)
						std::cout << "Pixel color " << pixels[i * 32 + j] << "\n";
					SDL_RenderDrawPoint(ren.get(), j, i);
				}
			}

			/*for (int i = 0; i < 64 * 32; ++i) // updating screen texture
			{
				
				/*for (int j = 0; j < 32; ++j)
				{
					if (chipPixels != 0)
					{
						SDL_SetRenderDrawColor(ren.get(), 0, 0, 0, 255);
						//SDL_RenderDrawPoint(ren.get(), i, j);
						SDL_Rect temp;
						temp.x = i;
						temp.y = j;
						temp.w = scale;
						temp.h = scale;
						SDL_RenderDrawRect(ren.get(), &temp);
						SDL_RenderPresent(ren.get());
						SDL_SetRenderDrawColor(ren.get(), 255, 255, 255, 255);
					}
				}
				if (chipPixels != 0)
				{

				}*/
				/*Uint32 temp = 0;
				if (chipPixels != 0)
					temp = 255;*/

				//pixels[i] = SDL_MapRGB(format, pixels[i] * 255, pixels[i] * 255, pixels[i] * 255);

				/*if (chipPixels[i] != 0)
				{
					pixels[i] = SDL_MapRGB(format, 0, 0, 0);
				}*/

				//pixels[i] = (Uint32)*chipPixels;
				//pixels[i] = temp;
				//std::cout << "pixels[" << i << "] = " << pixels[i] << "\n";
				//++chipPixels;
			//}

			//SDL_UpdateTexture(tex.get(), NULL, pixels, sizeof(Uint32) * 1);
			//SDL_FreeFormat(format);
			SDL_RenderCopy(ren.get(), tex.get(), NULL, NULL);
			SDL_RenderPresent(ren.get());
		}

		SDL_Delay(33); // should be 30 FPS, ugly, remplace with something better
	}

	SDL_Quit();

	return 0;
}