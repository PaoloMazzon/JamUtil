/// \file JamUtil.c
/// \author Paolo Mazzon
#include "JamUtil.h"
#include <stdio.h>

/********************** Constants **********************/
const uint32_t JU_BUCKET_SIZE = 100; // A good size for a small jam game, feel free to adjust

/********************** Static Functions **********************/

// Logs messages, used all over the place
static void juLog(const char *out) {
	printf("[JamUtil] %s\n", out);
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

/********************** Font **********************/

JUFont juFontLoad(const char *filename) {
	// TODO: This
}

JUFont juFontLoadFromImage(const char *image, uint32_t unicodeStart, uint32_t unicodeEnd, float w, float h) {
	// TODO: This
}

void juFontFree(JUFont font) {
	// TODO: This
}

void juFontDraw(JUFont font, float x, float y, const char *fmt, ...) {
	// TODO: This
}

void juFontDrawWrapped(JUFont font, float x, float y, float w, const char *fmt, ...) {
	// TODO: This
}

/********************** Asset Loader **********************/

static void juLoaderAdd(JULoader loader, JUAsset asset) {
	// TODO: This
}

JULoader juLoaderCreate(const char **files, uint32_t fileCount) {
	JULoader loader = juMalloc(sizeof(struct JULoader));
	JUAsset *assets = juMallocZero(JU_BUCKET_SIZE * sizeof(struct JUAsset));
	loader->assets = assets;

	// Load all assets
	for (int i = 0; i < fileCount; i++) {
		const char *extension = files[i] + juLastDot(files[i]);
		JUAsset asset = juMalloc(sizeof(struct JUAsset));

		// Load file based on asset
		asset->name = juCopyString(files[i]);
		if (strcmp(extension, "jufont") == 0) {

		} else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0) {

		} else if (strcmp(extension, "wav") == 0 || strcmp(extension, "ogg") == 0 || strcmp(extension, "mp3") == 0) {

		} else {
			juLog("Unrecognized extension");
		}

		juLoaderAdd(loader, asset);
	}

	return loader;
}

VK2DTexture juLoaderGetTexture(JULoader loader, const char *filename) {
	// TODO: This
}

JUFont juLoaderGetFont(JULoader loader, const char *filename) {
	// TODO: This
}

void juLoaderFree(JULoader loader) {
	if (loader != NULL) {
		for (int i = 0; i < JU_BUCKET_SIZE; i++) {
			// TODO: Free the linked list
		}
		free(loader->assets);
	}
}