/// \file JamUtil.c
/// \author Paolo Mazzon
#include <stdio.h>
#include <stdarg.h>
#include <VK2D/stb_image.h>
#include <SDL2/SDL_syswm.h>

#define CUTE_SOUND_IMPLEMENTATION
#include "cute_sound.h"
#include "JamUtil.h"

/********************** Constants **********************/
const uint32_t JU_BUCKET_SIZE = 100; // A good size for a small jam game, feel free to adjust
const uint32_t JU_BINARY_FONT_HEADER_SIZE = 13; // Size of the header of jufnt files
const uint32_t JU_STRING_BUFFER = 1024; // Maximum amount of text that can be rendered at once, a kilobyte is good for most things

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
uint32_t RMASK = 0xff000000;
uint32_t GMASK = 0x00ff0000;
uint32_t BMASK = 0x0000ff00;
uint32_t AMASK = 0x000000ff;
#else // little endian, like x86
uint32_t RMASK = 0x000000ff;
uint32_t GMASK = 0x0000ff00;
uint32_t BMASK = 0x00ff0000;
uint32_t AMASK = 0xff000000;
#endif

/********************** Globals **********************/
cs_context_t *gSoundContext = NULL;

/********************** "Private" Structs **********************/

/// \brief Character dimensions in the jufnt file
typedef struct JUBinaryCharacter {
	uint16_t width;  ///< Width of the character
	uint16_t height; ///< Height of the character
} JUBinaryCharacter;

/// \brief This is an unpacked representation of a binary jufnt file
typedef struct JUBinaryFont {
	uint32_t size;                          ///< Size in bytes of the png
	uint32_t characters;                    ///< Total number of characters in the font
	JUBinaryCharacter *characterDimensions; ///< Vector of jufnt characters
	void *png;                              ///< Raw bytes for the png image
} JUBinaryFont;

/********************** Top-Level **********************/

static void juLog(const char *out, ...);
void juInit(SDL_Window *window) {
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version)
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;
	gSoundContext = cs_make_context(hwnd, 41000, 1024 * 1024 * 10, 20, NULL);
	if (gSoundContext != NULL) {
		cs_spawn_mix_thread(gSoundContext);
	} else {
		juLog("Failed to initialize sound.");
	}
}

void juClose() {
	cs_shutdown_context(gSoundContext);
	gSoundContext = NULL;
}

/********************** Static Functions **********************/

// Logs messages, used all over the place
static void juLog(const char *out, ...) {
	va_list list;
	va_start(list, out);
	printf("[JamUtil] ");
	vprintf(out, list);
	printf("\n");
	va_end(list);
	fflush(stdout);
}

// Allocates memory, crashing if it doesn't work
static void *juMalloc(uint32_t size) {
	void *out = malloc(size);
	if (out == NULL) {
		juLog("Failed to allocate memory");
		abort();
	}
	return out;
}

// Allocates memory, crashing if it doesn't work
static void *juMallocZero(uint32_t size) {
	void *out = calloc(1, size);
	if (out == NULL) {
		juLog("Failed to allocate memory");
		abort();
	}
	return out;
}

// Hashes a string into a 32 bit number between 0 and JU_BUCKET_SIZE
static uint32_t juHash(const char *string) {
	uint32_t hash = 5381;
	int i = 0;

	while (string[i] != 0) {
		hash = ((hash << 5) + hash) + string[i]; /* hash * 33 + c */
		i++;
	}

	return hash % JU_BUCKET_SIZE;
}

// Find the position of the last period in a string (for filenames)
static uint32_t juLastDot(const char *string) {
	uint32_t dot = 0;
	uint32_t pos = 0;
	while (string[pos] != 0) {
		if (string[pos] == '.') dot = pos;
		pos++;
	}

	return dot;
}

// Copies a string
static const char *juCopyString(const char *string) {
	char *out = juMalloc(strlen(string) + 1);
	strcpy(out, string);
	return out;
}

// Swaps bytes between little and big endian
static void juSwapEndian(void *bytes, uint32_t size) {
	uint8_t new[size];
	memcpy(new, bytes, size);
	for (int i = size - 1; i >= 0; i--)
		((uint8_t*)bytes)[i] = new[size - i - 1];
}

static void juCopyFromBigEndian(void *dst, void *src, uint32_t size) {
	memcpy(dst, src, size);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	juSwapEndian(dst, size);
#endif // SDL_LIL_ENDIAN
}

// Dumps a file into a binary buffer (free it yourself)
static uint8_t *juGetFile(const char *filename, uint32_t *size) {
	uint8_t *buffer = NULL;
	FILE *file = fopen(filename, "rb");
	int len = 0;

	if (file != NULL) {
		while (!feof(file)) {
			fgetc(file);
			len++;
		}
		//len--;
		*size = len;
		buffer = juMalloc(len);
		rewind(file);
		fread(buffer, len, 1, file);

		fclose(file);
	} else {
		juLog("Couldn't open file \"%s\"", filename);
	}

	return buffer;
}

// Loads all jufnt data into a struct
static JUBinaryFont juLoadBinaryFont(const char *file, bool *error) {
	JUBinaryFont font = {};
	*error = false;
	uint32_t size;
	uint8_t *buffer = juGetFile(file, &size);
	uint32_t pointer = 5; // We don't care about the header

	if (buffer != NULL && size >= JU_BINARY_FONT_HEADER_SIZE) {
		// png size
		juCopyFromBigEndian(&font.size, buffer + pointer, 4);
		pointer += 4;

		// number of characters
		juCopyFromBigEndian(&font.characters, buffer + pointer, 4);
		pointer += 4;

		// We now have enough data to calculate the total size the file should be
		if (size - 1 == 13 + font.size + (font.characters * 4)) {
			font.size--;
			font.characterDimensions = juMalloc(font.characters * sizeof(struct JUBinaryCharacter));
			font.png = juMalloc(font.size);

			// Grab all the characters
			for (int i = 0; i < font.characters; i++) {
				juCopyFromBigEndian(&font.characterDimensions[i].width, buffer + pointer, 2);
				pointer += 2;
				juCopyFromBigEndian(&font.characterDimensions[i].height, buffer + pointer, 2);
				pointer += 2;
			}

			memcpy(font.png, buffer + pointer, font.size);

		} else {
			juLog("jufnt file \"%s\" is unreadable", file);
		}
	} else {
		*error = true;
	}
	free(buffer);

	return font;
}

/********************** Font **********************/

JUFont juFontLoad(const char *filename) {
	JUFont font = juMalloc(sizeof(struct JUFont));
	bool error;
	JUBinaryFont binaryFont = juLoadBinaryFont(filename, &error);

	if (!error) {
		// Create an SDL surface for the png
		int w, h, channels;
		void *pixels = stbi_load_from_memory(binaryFont.png, binaryFont.size, &w, &h, &channels, 4);

		if (pixels != NULL) {
			font->image = vk2dImageFromPixels(vk2dRendererGetDevice(), pixels, w, h);
			free(pixels);
			font->bitmap = vk2dTextureLoadFromImage(font->image);

			// Basic data
			font->unicodeStart = 1;
			font->unicodeEnd = binaryFont.characters + 1;
			font->characters = juMalloc(sizeof(struct JUCharacter) * binaryFont.characters);
			font->newLineHeight = 0;

			// Load the characters
			int x = 0;
			for (int i = 0; i < binaryFont.characters; i++) {
				font->characters[i].x = x;
				font->characters[i].y = 0;
				font->characters[i].w = binaryFont.characterDimensions[i].width;
				font->characters[i].h = binaryFont.characterDimensions[i].height;
				if (font->newLineHeight < font->characters[i].h)
					font->newLineHeight = font->characters[i].h;
				x += binaryFont.characterDimensions[i].width;
				font->characters[i].drawn = i >= 32;
			}
		} else {
			juLog("Failed to load font's image");
		}

		free(binaryFont.png);
		free(binaryFont.characterDimensions);
	}

	return font;
}

JUFont juFontLoadFromImage(const char *image, uint32_t unicodeStart, uint32_t unicodeEnd, float w, float h) {
	// Setup font struct
	JUFont font = juMalloc(sizeof(struct JUFont));
	font->characters = juMalloc(sizeof(struct JUCharacter) * (unicodeEnd - unicodeStart));
	font->bitmap = vk2dTextureLoad(image);
	font->image = NULL;
	font->newLineHeight = h;
	font->unicodeStart = unicodeStart;
	font->unicodeEnd = unicodeEnd;

	// Make sure the texture loaded and the texture has enough space to load the desired characters
	if (font->bitmap != NULL && w * h * (unicodeEnd - unicodeStart) <= font->bitmap->img->width * font->bitmap->img->height) {
		// Calculate positions of every character in the font
		float x = 0;
		float y = 0;
		int i = unicodeStart;
		while (i < unicodeEnd) {
			font->characters[i - unicodeStart].x = x;
			font->characters[i - unicodeStart].y = y;
			font->characters[i - unicodeStart].w = w;
			font->characters[i - unicodeStart].h = h;
			if (x + w >= font->bitmap->img->width) {
				y += h;
				x = 0;
			} else {
				x += w;
			}
			i++;
		}
	} else {
		juLog("Failed to load texture \"%s\"", image);
		vk2dTextureFree(font->bitmap);
		free(font->characters);
		free(font);
		font = NULL;
	}

	return font;
}

void juFontFree(JUFont font) {
	if (font != NULL) {
		vk2dTextureFree(font->bitmap);
		vk2dImageFree(font->image);
		free(font->characters);
		free(font);
	}
}

void juFontDraw(JUFont font, float x, float y, const char *fmt, ...) {
	// Var args stuff
	char buffer[JU_STRING_BUFFER];
	va_list va;
	va_start(va, fmt);
	vsprintf(buffer, fmt, va);
	va_end(va);

	// Information needed to draw the text
	float startX = x;
	int len = strlen(buffer);

	// Loop through each character and render individually
	for (int i = 0; i < len; i++) {
		if (font->unicodeStart <= buffer[i] && font->unicodeEnd > buffer[i]) {
			JUCharacter *c = &font->characters[buffer[i] - font->unicodeStart];

			// Move to the next line if we're about to go over
			if (buffer[i] == '\n') {
				x = startX;
				y += font->newLineHeight;
			}

			// Draw character (or not) and move the cursor forward
			if (c->drawn)
				vk2dRendererDrawTexture(font->bitmap, x, y, 1, 1, 0, 0, 0, c->x, c->y, c->w, c->h);
			if (buffer[i] != '\n') x += c->w;
		}
	}
}

void juFontDrawWrapped(JUFont font, float x, float y, float w, const char *fmt, ...) {
	// Var args stuff
	char buffer[JU_STRING_BUFFER];
	va_list va;
	va_start(va, fmt);
	vsprintf(buffer, fmt, va);
	va_end(va);

	// Information needed to draw the text
	float startX = x;
	int len = strlen(buffer);

	// Loop through each character and render individually
	for (int i = 0; i < len; i++) {
		if (font->unicodeStart <= buffer[i] && font->unicodeEnd > buffer[i]) {
			JUCharacter *c = &font->characters[buffer[i] - font->unicodeStart];

			// Move to the next line if we're about to go over
			if ((c->w + x) - startX > w || buffer[i] == '\n') {
				x = startX;
				y += font->newLineHeight;
			}

			// Draw character (or not) and move the cursor forward
			if (c->drawn)
				vk2dRendererDrawTexture(font->bitmap, x, y, 1, 1, 0, 0, 0, c->x, c->y, c->w, c->h);
			if (buffer[i] != '\n') x += c->w;
		}
	}
}

/********************** Buffer **********************/

JUBuffer juBufferLoad(const char *filename) {
	JUBuffer buffer = juMalloc(sizeof(struct JUBuffer));
	buffer->data = juGetFile(filename, &buffer->size);

	if (buffer->data == NULL) {
		free(buffer);
		buffer = NULL;
	}

	return buffer;
}

JUBuffer juBufferCreate(void *data, uint32_t size) {
	JUBuffer buffer = juMalloc(sizeof(struct JUBuffer));
	buffer->data = juMalloc(size);
	buffer->size = size;
	memcpy(buffer->data, data, size);
	return buffer;
}

void juBufferSave(JUBuffer buffer, const char *filename) {
	FILE *out = fopen(filename, "wb");

	if (out != NULL) {
		fwrite(buffer->data, buffer->size, 1, out);
		fclose(out);
	} else {
		juLog("Failed to open file \"%s\"", filename);
	}
}

void juBufferFree(JUBuffer buffer) {
	if (buffer != NULL) {
		free(buffer->data);
		free(buffer);
	}
}

void juBufferSaveRaw(void *data, uint32_t size, const char *filename) {
	FILE *out = fopen(filename, "wb");

	if (out != NULL) {
		fwrite(data, size, 1, out);
		fclose(out);
	} else {
		juLog("Failed to open file \"%s\"", filename);
	}
}

/********************** Asset Loader **********************/

// Puts an asset into the loader (properly)
static void juLoaderAdd(JULoader loader, JUAsset asset) {
	uint32_t hash = juHash(asset->name);

	// Either we drop the asset right into its slot or send it down a linked
	// list chain if there is a hash collision
	if (loader->assets[hash] == NULL) {
		loader->assets[hash] = asset;
	} else {
		JUAsset current = loader->assets[hash];
		bool placed = false;
		while (!placed) {
			if (current->next == NULL) {
				placed = true;
				current->next = asset;
			} else {
				current = current->next;
			}
		}
	}
}

// Just gets the raw asset from the loader
static JUAsset juLoaderGet(JULoader loader, const char *key) {
	JUAsset current = loader->assets[juHash(key)];
	bool found = false;

	while (!found) {
		if (current != NULL && strcmp(current->name, key) == 0) // this is the rigt asset
			found = true;
		else if (current != NULL) // wrong one, next one down the chain
			current = current->next;
		else // it doesn't exist, current should be null at this point
			found = true;
	}

	return current;
}

// Frees a specific asset (not its next one in its chain though)
static void juLoaderAssetFree(JUAsset asset) {
	if (asset->type == JU_ASSET_TYPE_FONT) {
		juFontFree(asset->Asset.font);
	} else if (asset->type == JU_ASSET_TYPE_TEXTURE) {
		vk2dTextureFree(asset->Asset.tex);
	} else if (asset->type == JU_ASSET_TYPE_SOUND) {
		juSoundFree(asset->Asset.sound);
	} else if (asset->type == JU_ASSET_TYPE_BUFFER) {
		juBufferFree(asset->Asset.buffer);
	}
	free((void*)asset->name);
	free(asset);
}

JULoader juLoaderCreate(const char **files, uint32_t fileCount) {
	JULoader loader = juMalloc(sizeof(struct JULoader));
	JUAsset *assets = juMallocZero(JU_BUCKET_SIZE * sizeof(struct JUAsset));
	loader->assets = assets;

	// Load all assets
	for (int i = 0; i < fileCount; i++) {
		const char *extension = files[i] + juLastDot(files[i]) + 1;
		JUAsset asset = juMalloc(sizeof(struct JUAsset));
		asset->name = juCopyString(files[i]);
		asset->next = NULL;

		// Load file based on asset
		if (strcmp(extension, "jufnt") == 0) {
			asset->type = JU_ASSET_TYPE_FONT;
			asset->Asset.font = juFontLoad(files[i]);
		} else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0 || strcmp(extension, "bmp") == 0) {
			asset->type = JU_ASSET_TYPE_TEXTURE;
			asset->Asset.tex = vk2dTextureLoad(files[i]);
		} else if (strcmp(extension, "wav") == 0) {
			asset->type = JU_ASSET_TYPE_SOUND;
			asset->Asset.sound = juSoundLoad(files[i]);
		} else {
			asset->type = JU_ASSET_TYPE_BUFFER;
			asset->Asset.buffer = juBufferLoad(files[i]);
		}

		juLoaderAdd(loader, asset);
	}

	return loader;
}

VK2DTexture juLoaderGetTexture(JULoader loader, const char *filename) {
	JUAsset asset = juLoaderGet(loader, filename);
	VK2DTexture out = NULL;

	if (asset != NULL) {
		if (asset->type == JU_ASSET_TYPE_TEXTURE)
			out = asset->Asset.tex;
		else
			juLog("Asset \"%s\" is of incorrect type", filename);
	} else {
		juLog("Asset \"%s\" was never loaded", filename);
	}

	return out;
}

JUFont juLoaderGetFont(JULoader loader, const char *filename) {
	JUAsset asset = juLoaderGet(loader, filename);
	JUFont out = NULL;

	if (asset != NULL) {
		if (asset->type == JU_ASSET_TYPE_FONT)
			out = asset->Asset.font;
		else
			juLog("Asset \"%s\" is of incorrect type", filename);
	} else {
		juLog("Asset \"%s\" doesn't exist", filename);
	}

	return out;
}

JUSound juLoaderGetSound(JULoader loader, const char *filename) {
	JUAsset asset = juLoaderGet(loader, filename);
	JUSound out = NULL;

	if (asset != NULL) {
		if (asset->type == JU_ASSET_TYPE_SOUND)
			out = asset->Asset.sound;
		else
			juLog("Asset \"%s\" is of incorrect type", filename);
	} else {
		juLog("Asset \"%s\" doesn't exist", filename);
	}

	return out;
}

JUBuffer juLoaderGetBuffer(JULoader loader, const char *filename) {
	JUAsset asset = juLoaderGet(loader, filename);
	JUBuffer out = NULL;

	if (asset != NULL) {
		if (asset->type == JU_ASSET_TYPE_BUFFER)
			out = asset->Asset.buffer;
		else
			juLog("Asset \"%s\" is of incorrect type", filename);
	} else {
		juLog("Asset \"%s\" doesn't exist", filename);
	}

	return out;
}

void juLoaderFree(JULoader loader) {
	if (loader != NULL) {
		for (int i = 0; i < JU_BUCKET_SIZE; i++) {
			JUAsset current = loader->assets[i];
			while (current != NULL) {
				JUAsset next = current->next;
				juLoaderAssetFree(current);
				current = next;
			}
		}
		free(loader->assets);
	}
}

/********************** Sound **********************/

JUSound juSoundLoad(const char *filename) {
	JUSound sound = juMallocZero(sizeof(struct JUSound));
	sound->sound = cs_load_wav(filename);
	return sound;
}

JUPlayingSound juSoundPlay(JUSound sound, bool loop, float volumeLeft, float volumeRight) {
	sound->soundInfo = cs_make_def(&sound->sound);
	sound->soundInfo.looped = loop;
	sound->soundInfo.volume_left = 0.5;
	sound->soundInfo.volume_right = 0.5;
	JUPlayingSound new = {cs_play_sound(gSoundContext, sound->soundInfo)};
	return new;
}

void juSoundUpdate(JUPlayingSound sound, bool loop, float volumeLeft, float volumeRight) {
	if (sound.playingSound != NULL && cs_is_active(sound.playingSound)) {
		cs_loop_sound(sound.playingSound, loop);
		cs_set_volume(sound.playingSound, volumeLeft, volumeRight);
	}
}

void juSoundStop(JUPlayingSound sound) {
	if (sound.playingSound != NULL && cs_is_active(sound.playingSound))
		cs_stop_sound(sound.playingSound);
}

void juSoundFree(JUSound sound) {
	cs_free_sound(&sound->sound);
	free(sound);
}

void juSoundStopAll() {
	cs_stop_all_sounds(gSoundContext);
}

/********************** Collisions **********************/

float juPointAngle(float x1, float y1, float x2, float y2) {
	return atanf((x2 - x1) / (y2 - y1));
}

float juPointDistance(float x1, float y1, float x2, float y2) {
	return sqrtf(powf(y2 - y1, 2) + powf(x2 - x1, 2));
}

bool juRectangleCollision(JURectangle *r1, JURectangle *r2) {
	return (r1->y + r1->h > r2->y && r1->y < r2->y + r2->h && r1->x + r1->w > r2->x && r1->x < r2->x + r2->w);
}

bool juCircleCollision(JUCircle *c1, JUCircle *c2) {
	return juPointDistance(c1->x, c1->y, c2->x, c2->y) < c1->r + c2->r;
}

bool juPointInRectangle(JURectangle *rect, float x, float y) {
	return (x >= rect->x && x <= rect->x + rect->w && y >= rect->y && x <= rect->y + rect->h);
}

bool juPointInCircle(JUCircle *circle, float x, float y) {
	return juPointDistance(circle->x, circle->y, x, y) <= circle->r;
}

/********************** File I/O **********************/

JUSave juSaveLoad(const char *filename) {
	JUSave save = juMalloc(sizeof(JUSave));
	JUBuffer buffer = juBufferLoad(filename);
	uint32_t pointer = 5;

	if (buffer != NULL && buffer->size >= 9) {
		// TODO: This
	} else {
		save->data = NULL;
		save->size = 0;
	}

	juBufferFree(buffer);
	return save;
}

void juSaveStore(JUSave save, const char *filename) {
	// TODO: This
}

void juSaveFree(JUSave save) {
	if (save != NULL) {
		for (int i = 0; i < save->size; i++) {
			free((void*)save->data[i].key);
			if (save->data[i].type == JU_DATA_TYPE_STRING)
				free((void*)save->data[i].Data.string);
			else if (save->data[i].type == JU_DATA_TYPE_VOID)
				free(save->data[i].Data.data.data);
		}
		free(save->data);
		free(save);
	}
}

void juSaveSetInt64(JUSave save, const char *key, int64_t data) {
	// TODO: This
}

int64_t juSaveGetInt64(JUSave save, const char *key) {
	// TODO: This
}

void juSaveSetUInt64(JUSave save, const char *key, uint64_t data) {
	// TODO: This
}

uint64_t juSaveGetUInt64(JUSave save, const char *key) {
	// TODO: This
}

void juSaveSetFloat(JUSave save, const char *key, float data) {
	// TODO: This
}

float juSaveGetFloat(JUSave save, const char *key) {
	// TODO: This
}

void juSaveSetDouble(JUSave save, const char *key, double data) {
	// TODO: This
}

double juSaveGetDouble(JUSave save, const char *key) {
	// TODO: This
}

void juSaveSetString(JUSave save, const char *key, const char *data) {
	// TODO: This
}

const char *juSaveGetString(JUSave save, const char *key) {
	// TODO: This
}

void juSaveSetData(JUSave save, const char *key, void *data, uint32_t size) {
	// TODO: This
}

void *juSaveGetData(JUSave save, const char *key, uint32_t *size) {
	// TODO: This
}
