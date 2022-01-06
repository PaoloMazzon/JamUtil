#define SDL_MAIN_HANDLED
#include <stdio.h>
#include "JamUtil.h"

/***************************** Constants *****************************/

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int WINDOW_SCALE = 1;

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

/***************************** ECS stuff *****************************/

typedef struct CompKinematics {
	vec2 velocity;
	vec2 acceleration;
	float friction;
} CompKinematics;

typedef struct CompPosition {
	vec2 position;
} CompPosition;

typedef struct CompVisible {
	float radius;
} CompVisible;

size_t COMPONENT_SIZES[] = {
		sizeof(struct CompKinematics),
		sizeof(struct CompPosition),
		sizeof(struct CompVisible),
};

typedef enum {
	COMPONENT_KINEMATICS = 0,
	COMPONENT_POSITION = 1,
	COMPONENT_VISIBLE = 2,
	COMPONENT_COUNT = 3,
} Components;

void systemDraw(JUEntity *entity, const JUComponentVector* const readComponents, JUComponentVector* writeComponents) {

}

void systemPhysics(JUEntity *entity, const JUComponentVector* const readComponents, JUComponentVector* writeComponents) {

}
JUComponent DRAW_COMPONENTS[] = {COMPONENT_POSITION, COMPONENT_VISIBLE};
JUComponent PHYSICS_COMPONENTS[] = {COMPONENT_POSITION, COMPONENT_KINEMATICS};
JUSystem SYSTEMS[] = {
		{DRAW_COMPONENTS, 2, systemDraw},
		{PHYSICS_COMPONENTS, 2, systemPhysics},
};
const int SYSTEM_COUNT = 2;

/***************************** Main *****************************/

int main() {
	SDL_Window *window = SDL_CreateWindow("VK2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH * WINDOW_SCALE, WINDOW_HEIGHT * WINDOW_SCALE, SDL_WINDOW_VULKAN);
	SDL_Event e;
	VK2DRendererConfig config = {msaa_32x, sm_TripleBuffer, ft_Nearest};
	juInit(window, 10);
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
	double angle = VK2D_PI / 3;
	double ox = 0;
	double oy = 0;
	JURectangle movingRectangle = {100, 100, 100, 100};
	double movingAngle = VK2D_PI / 6;

	// ECS
	juECSAddComponents(COMPONENT_SIZES, COMPONENT_COUNT);
	juECSAddSystems(SYSTEMS, SYSTEM_COUNT);

	while (!stopRunning) {
		juUpdate();
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				stopRunning = true;
			}
		}

		vk2dRendererStartFrame(clearColour);

		// ECS
		juECSRunSystems();
		juECSCopyState();

		vk2dRendererEndFrame();
	}

	vk2dRendererWait();

	// Free assets
	juLoaderFree(loader);

	juQuit();
	vk2dRendererQuit();
	SDL_DestroyWindow(window);
	return 0;
}
