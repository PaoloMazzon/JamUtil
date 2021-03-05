/// \file JamUtil.c
/// \author Paolo Mazzon
#include "JamUtil.h"
#include <stdio.h>
#include <stdarg.h>

/********************** Constants **********************/
const uint32_t JU_BUCKET_SIZE = 100; // A good size for a small jam game, feel free to adjust

/********************** Static Functions **********************/

// Logs messages, used all over the place
static void juLog(const char *out, ...) {
	va_list list;
	va_start(list, out);
	printf("[JamUtil] ");
	vprintf(out, list);
	printf("\n");
	va_end(list);
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
		// TODO: Implement sound
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
		const char *extension = files[i] + juLastDot(files[i]);
		JUAsset asset = juMalloc(sizeof(struct JUAsset));

		// Load file based on asset
		asset->name = juCopyString(files[i]);
		if (strcmp(extension, "jufnt") == 0) {
			asset->type = JU_ASSET_TYPE_FONT;
			asset->Asset.font = juFontLoad(files[i]);
		} else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0) {
			asset->type = JU_ASSET_TYPE_TEXTURE;
			asset->Asset.tex = vk2dTextureLoad(files[i]);
		} else if (strcmp(extension, "wav") == 0 || strcmp(extension, "ogg") == 0 || strcmp(extension, "mp3") == 0) {
			asset->type = JU_ASSET_TYPE_SOUND;
			//asset->Asset.sound = NULL; TODO: Audio implementation
		} else {
			juLog("Unrecognized extension");
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

JUFont juLoaderGetSound(JULoader loader, const char *filename) {
	JUAsset asset = juLoaderGet(loader, filename);
	void *out = NULL; // TODO: Implement sound

	if (asset != NULL) {
		if (asset->type == JU_ASSET_TYPE_SOUND)
			out = asset->Asset.font; // TODO: Implement sound
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