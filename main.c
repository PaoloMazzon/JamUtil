#define SDL_MAIN_HANDLED
#include <stdio.h>
#include "JamUtil.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const char *FILES[] = {
		"assets/image1.png",
		"assets/image2.png",
		"assets/image3.png",
		"assets/comic.jufnt"
};
const uint32_t FILE_COUNT = 4;

int main() {
	SDL_Window *window = SDL_CreateWindow("VK2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN);
	SDL_Event e;
	VK2DRendererConfig config = {msaa_32x, sm_TripleBuffer, ft_Nearest};
	vk2dRendererInit(window, config);
	vec4 clearColour = {0.0, 0.0, 0.0, 1.0}; // Black
	bool stopRunning = false;

	// Load resources
	JULoader loader = juLoaderCreate(FILES, FILE_COUNT);

	while (!stopRunning) {
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				stopRunning = true;

		vk2dRendererStartFrame(clearColour);

		// Draw your things
		vk2dDrawTextureExt(juLoaderGetTexture(loader, "assets/image1.png"), 400, 300, 5, 5, 0, 0, 0);
		juFontDrawWrapped(juLoaderGetFont(loader, "assets/comic.jufnt"), 0, 0, 800, "The quick brown fox jumps over the lazy dog.");

		vk2dRendererEndFrame();
	}

	vk2dRendererWait();

	// Free assets
	juLoaderFree(loader);

	vk2dRendererQuit();
	SDL_DestroyWindow(window);
	return 0;
}
