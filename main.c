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

// All of the components are just structs
typedef struct CompKinematics {
	JUECSLock inputLock; // For AI and player to lock out kinematics until input is given
	vec2 velocity;
	vec2 acceleration;
	float friction;
	float mass;
} CompKinematics;

typedef struct CompPosition {
	vec2 position;
} CompPosition;

typedef struct CompVisible {
	VK2DTexture tex;
} CompVisible;

typedef struct CompHitbox {
	float w;
	float h;
} CompHitbox;

typedef struct CompPlayerInput {
	uint8_t garbage;
} CompPlayerInput;

typedef struct CompNPCAI {
	uint8_t garbage;
} CompNPCAI;

// The ECS only needs to know how large the structs are
size_t COMPONENT_SIZES[] = {
		sizeof(struct CompKinematics),
		sizeof(struct CompPosition),
		sizeof(struct CompVisible),
		sizeof(struct CompHitbox),
		sizeof(struct CompNPCAI),
		sizeof(CompPlayerInput),
};

// For ease of use everywhere, make an enum that corresponds to the size list above
typedef enum {
	COMPONENT_KINEMATICS = 0,
	COMPONENT_POSITION = 1,
	COMPONENT_VISIBLE = 2,
	COMPONENT_HITBOX = 3,
	COMPONENT_PLAYER_INPUT = 4,
	COMPONENT_NPC_AI = 5,
	COMPONENT_COUNT = 6,
} Components;

/************************ System functions ************************/

void systemDraw(JUEntityID entity) {
	// Get the necessary components
	const CompPosition *prevPos = juECSGetPreviousComponent(COMPONENT_POSITION, entity);
	const CompVisible *prevVisible = juECSGetPreviousComponent(COMPONENT_VISIBLE, entity);
	vk2dDrawTexture(prevVisible->tex, prevPos->position[0], prevPos->position[1]);
}

void systemPhysics(JUEntityID entity) {
	// Get the necessary components
	const CompPosition *prevPos = juECSGetPreviousComponent(COMPONENT_POSITION, entity);
	CompPosition *pos = juECSGetComponent(COMPONENT_POSITION, entity);
	const CompKinematics *prevKinematics = juECSGetPreviousComponent(COMPONENT_KINEMATICS, entity);
	CompKinematics *kinematics = juECSGetComponent(COMPONENT_KINEMATICS, entity);
	const CompHitbox *prevHitbox = juECSGetPreviousComponent(COMPONENT_HITBOX, entity);
	CompHitbox *hitbox = juECSGetComponent(COMPONENT_HITBOX, entity);
	juECSLockWait(&kinematics->inputLock, 1);

	pos->position[0] += kinematics->velocity[0];
	pos->position[1] += kinematics->velocity[1];

	juECSLockReset(&kinematics->inputLock);
}

void systemPlayerInput(JUEntityID entity) {
	// Get the necessary components
	const CompPlayerInput *prevPlayerInput = juECSGetPreviousComponent(COMPONENT_PLAYER_INPUT, entity);
	CompPlayerInput *playerInput = juECSGetComponent(COMPONENT_PLAYER_INPUT, entity);
	const CompKinematics *prevKinematics = juECSGetPreviousComponent(COMPONENT_KINEMATICS, entity);
	CompKinematics *kinematics = juECSGetComponent(COMPONENT_KINEMATICS, entity);
	juECSLockWait(&kinematics->inputLock, 0);

	kinematics->velocity[0] = (-juKeyboardGetKey(SDL_SCANCODE_A) + juKeyboardGetKey(SDL_SCANCODE_D)) * 180 * juDelta();
	kinematics->velocity[1] = (-juKeyboardGetKey(SDL_SCANCODE_W) + juKeyboardGetKey(SDL_SCANCODE_S)) * 180 * juDelta();

	juECSLockNext(&kinematics->inputLock);
}

void systemNPCAI(JUEntityID entity) {
	// Get the necessary components
	const CompNPCAI *prevNPCAI= juECSGetPreviousComponent(COMPONENT_NPC_AI, entity);
	CompNPCAI *npcAI = juECSGetComponent(COMPONENT_NPC_AI, entity);
	const CompKinematics *prevKinematics = juECSGetPreviousComponent(COMPONENT_KINEMATICS, entity);
	CompKinematics *kinematics = juECSGetComponent(COMPONENT_KINEMATICS, entity);
	juECSLockWait(&kinematics->inputLock, 0);



	juECSLockNext(&kinematics->inputLock);
}

/************************ System declarations ************************/

JUComponent DRAW_COMPONENTS[] = {COMPONENT_POSITION, COMPONENT_VISIBLE};
JUComponent PHYSICS_COMPONENTS[] = {COMPONENT_POSITION, COMPONENT_KINEMATICS, COMPONENT_HITBOX};
JUComponent PLAYER_INPUT_COMPONENTS[] = {COMPONENT_KINEMATICS, COMPONENT_PLAYER_INPUT};
JUComponent NPC_AI_COMPONENTS[] = {COMPONENT_KINEMATICS, COMPONENT_NPC_AI};
JUSystem SYSTEMS[] = {
		{DRAW_COMPONENTS, 2, systemDraw},
		{PHYSICS_COMPONENTS, 3, systemPhysics},
		{PLAYER_INPUT_COMPONENTS, 2, systemPlayerInput},
		{NPC_AI_COMPONENTS, 2, systemNPCAI},
};

typedef enum {
	SYSTEM_DRAW = 0,
	SYSTEM_PHYSICS = 1,
	SYSTEM_PLAYER_INPUT = 2,
	SYSTEM_NPC_AI = 3,
	SYSTEM_COUNT = 4,
} Systems;

const JUComponent ENT_COMPS_PLAYER[] = {COMPONENT_POSITION, COMPONENT_VISIBLE, COMPONENT_PLAYER_INPUT, COMPONENT_HITBOX, COMPONENT_KINEMATICS};
const int ENT_COMPS_PLAYER_SIZE = 5;
const JUComponent ENT_COMPS_NPC[] = {COMPONENT_POSITION, COMPONENT_KINEMATICS, COMPONENT_VISIBLE, COMPONENT_NPC_AI, COMPONENT_HITBOX};
const int ENT_COMPS_NPC_SIZE = 5;
const JUComponent ENT_COMPS_BOX[] = {COMPONENT_POSITION, COMPONENT_VISIBLE, COMPONENT_KINEMATICS, COMPONENT_HITBOX};
const int ENT_COMPS_BOX_SIZE = 4;

/***************************** Main *****************************/

int main() {
	SDL_Window *window = SDL_CreateWindow("VK2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH * WINDOW_SCALE, WINDOW_HEIGHT * WINDOW_SCALE, SDL_WINDOW_VULKAN);
	SDL_Event e;
	VK2DRendererConfig config = {msaa_32x, sm_TripleBuffer, ft_Nearest};
	juInit(window, 10, 3);
	vk2dRendererInit(window, config);
	vec4 clearColour = {0, 0, 0, 1}; // Black
	bool stopRunning = false;
	double totalTime = 0;
	double iters = 0;
	double average = 1;
	JUClock framerateTimer;
	juClockReset(&framerateTimer);

	// Setup camera
	VK2DCamera cam = vk2dRendererGetCamera();
	cam.w = WINDOW_WIDTH;
	cam.h = WINDOW_HEIGHT;
	vk2dRendererSetCamera(cam);

	// Load resources
	JULoader loader = juLoaderCreate(FILES, FILE_COUNT);
	juECSAddComponents(COMPONENT_SIZES, COMPONENT_COUNT);
	juECSAddSystems(SYSTEMS, SYSTEM_COUNT);

	// Add player
	CompPosition pos = {30, 30};
	CompVisible visible = {juLoaderGetTexture(loader, "assets/image1.png")};
	CompPlayerInput playerInput = {};
	CompHitbox hitbox = {10, 10};
	CompKinematics kin = {};
	JUComponentVector comps[] = {&pos, &visible, &playerInput, &hitbox, &kin};
	juECSAddEntity(ENT_COMPS_PLAYER, comps, ENT_COMPS_PLAYER_SIZE);

	while (!stopRunning) {
		juUpdate();
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				stopRunning = true;
			}
		}

		vk2dRendererStartFrame(clearColour);
		juECSRunSystems();

		// Draw UI
		juECSWaitSystemFinished(SYSTEM_DRAW);
		juFontDraw(juLoaderGetFont(loader, "assets/comic.jufnt"), 0, 0, "FPS: %0.2f", 1.0 / average);

		juECSCopyState();

		// Frame timing
		if (juClockTime(&framerateTimer) >= 1) {
			average = totalTime / iters;
			totalTime = 0;
			iters = 0;
			juClockStart(&framerateTimer);
		} else {
			totalTime += juDelta();
			iters += 1;
		}
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
