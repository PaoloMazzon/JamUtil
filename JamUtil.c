/// \file JamUtil.c
/// \author Paolo Mazzon
#include <stdio.h>
#include <stdarg.h>
#include "JamUtil.h"

/********************** Constants **********************/
const uint32_t JU_BUCKET_SIZE = 100; // A good size for a small jam game, feel free to adjust
const uint32_t JU_BINARY_FONT_HEADER_SIZE = 13; // Size of the header of jufnt files

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

// Swaps bytes between little and big endian
static void juSwapEndian(void *bytes, uint32_t size) {
	uint8_t new[size];
	memcpy(new, bytes, size);
	for (int i = size - 1; i >= 0; i--)
		((uint8_t*)bytes)[i] = new[size - i - 1];
}

static void juCopyFromBigEndian(void *dst, void *src, uint32_t size) {
	memcpy(dst, src, size);
#if SDL_LIL_ENDIAN
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
		if (size == 13 + font.size + (font.characters * 4)) {
			font.characterDimensions = juMalloc(font.characters * sizeof(struct JUBinaryCharacter));
			font.png = juMalloc(font.size);

			// Grab all the characters
			for (int i = 0; i < font.characters; i++) {
				juCopyFromBigEndian(&font.characterDimensions[i].width, buffer + pointer, 2);
				pointer += 2;
				juCopyFromBigEndian(&font.characterDimensions[i].height, buffer + pointer, 2);
				pointer += 2;
			}

			memcpy(&font.png, buffer + pointer, font.size);
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
		// TODO: This


		free(binaryFont.png);
		free(binaryFont.characterDimensions);
	}

	return font;
}

JUFont juFontLoadFromImage(const char *image, uint32_t unicodeStart, uint32_t unicodeEnd, float w, float h) {
	// TODO: This
}

void juFontFree(JUFont font) {
	if (font != NULL) {
		free(font->characters);
		vk2dTextureFree(font->bitmap);
		free(font);
	}
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