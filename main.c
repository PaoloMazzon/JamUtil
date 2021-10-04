#define SDL_MAIN_HANDLED
#include <stdio.h>
#include "JamUtil.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int WINDOW_SCALE = 3;

JULoadedAsset FILES[] = {
	{"assets/image1.png"},
	{"assets/comic.jufnt"},
	{"assets/test_sound.wav"},
	{"GenFont.py"},
	{"assets/sheet.png", 50, 50, 50, 50, 0.1, 9},
};
const uint32_t FILE_COUNT = 5;

vec4 DEFAULT_COLOUR = {1, 1, 1, 1};
vec4 COLLISION_COLOUR = {1, 0, 0, 1};

int main() {
	SDL_Window *window = SDL_CreateWindow("VK2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH * WINDOW_SCALE, WINDOW_HEIGHT * WINDOW_SCALE, SDL_WINDOW_VULKAN);
	SDL_Event e;
	VK2DRendererConfig config = {msaa_32x, sm_TripleBuffer, ft_Nearest};
	juInit(window);
	vk2dRendererInit(window, config);
	vec4 clearColour = {0.0, 0.0, 0.0, 1.0}; // Black
	bool stopRunning = false;

	// Setup camera
	VK2DCamera cam = vk2dRendererGetCamera();
	cam.w = WINDOW_WIDTH;
	cam.h = WINDOW_HEIGHT;
	vk2dRendererSetCamera(cam);

	// Load resources
	JULoader loader = juLoaderCreate(FILES, FILE_COUNT);
	JURectangle rectangle = {100, 100, 100, 100};
	double angle = VK2D_PI / 6;
	double ox = 0;
	double oy = 0;

	// Make a collision map image
	SDL_Surface *collisionMap = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0, 0, 0, 0);
	for (int y = 0; y < WINDOW_HEIGHT; y++) {
		for (int x = 0; x < WINDOW_WIDTH; x++) {
			if (juPointInRotatedRectangle(&rectangle, angle, ox, oy, x, y))
				((uint32_t*)collisionMap->pixels)[(WINDOW_WIDTH * y) + x] = 0xff0000ff;
			else
				((uint32_t*)collisionMap->pixels)[(WINDOW_WIDTH * y) + x] = 0xff000000;
		}
	}
	VK2DImage testImage = vk2dImageFromPixels(vk2dRendererGetDevice(), collisionMap->pixels, WINDOW_WIDTH, WINDOW_HEIGHT);
	VK2DTexture test = vk2dTextureLoadFromImage(testImage);
	SDL_FreeSurface(collisionMap);

	while (!stopRunning) {
		juUpdate();
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				stopRunning = true;
			}
		}

		vk2dRendererStartFrame(clearColour);

		// Draw your things
		int mx, my;
		SDL_GetMouseState(&mx, &my);
		mx /= WINDOW_SCALE;
		my /= WINDOW_SCALE;
		if (juKeyboardGetKey(SDL_SCANCODE_SPACE))
			vk2dDrawTexture(test, 0, 0);
		juSpriteDraw(juLoaderGetSprite(loader, "assets/sheet.png"), 400, 500);
		vk2dDrawTextureExt(juLoaderGetTexture(loader, "assets/image1.png"), 400, 300, 5, 5, 0, 0, 0);
		if (juPointInRotatedRectangle(&rectangle, angle, ox, oy, mx, my))
			vk2dRendererSetColourMod(COLLISION_COLOUR);
		vk2dRendererDrawRectangle(rectangle.x, rectangle.y, rectangle.w, rectangle.h, juKeyboardGetKey(SDL_SCANCODE_RETURN) ? 0 : angle, ox, oy);
		vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		juFontDrawWrapped(juLoaderGetFont(loader, "assets/comic.jufnt"), 0, 0, 800, "The quick brown fox jumps over the lazy dog.");

		vk2dRendererEndFrame();
	}

	vk2dRendererWait();

	// Free assets
	juLoaderFree(loader);
	vk2dTextureFree(test);
	vk2dImageFree(testImage);

	juQuit();
	vk2dRendererQuit();
	SDL_DestroyWindow(window);
	return 0;
}
