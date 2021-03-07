/// \file JamUtil.h
/// \author Paolo Mazzon
/// \brief A small collection of tools for quick game-dev with Vulkan2D
#pragma once
#include <stdbool.h>
#include <VK2D/VK2D.h>
#include "cute_sound.h"

/********************** Typedefs **********************/
typedef struct JUCharacter JUCharacter;
typedef struct JUFont *JUFont;
typedef struct JUAsset *JUAsset;
typedef struct JULoader *JULoader;
typedef struct JUSound *JUSound;
typedef struct JUPlayingSound JUPlayingSound;
typedef struct JURectangle JURectangle;
typedef struct JUCircle JUCircle;
typedef struct JUData JUData;
typedef struct JUSave *JUSave;

/********************** Enums **********************/

/// \brief Types of assets stored in the loader
typedef enum {
	JU_ASSET_TYPE_NONE = 0,
	JU_ASSET_TYPE_FONT = 1,
	JU_ASSET_TYPE_TEXTURE = 2,
	JU_ASSET_TYPE_SOUND = 3,
	JU_ASSET_TYPE_MAX = 4,
} JUAssetType;

/// \brief Types of data the save can handle
typedef enum {
	JU_DATA_TYPE_NONE = 0,
	JU_DATA_TYPE_FLOAT = 1,
	JU_DATA_TYPE_DOUBLE = 2,
	JU_DATA_TYPE_INT64 = 3,
	JU_DATA_TYPE_UINT64 = 4,
	JU_DATA_TYPE_STRING = 5,
	JU_DATA_TYPE_VOID = 6,
	JU_DATA_TYPE_MAX = 7,
} JUDataType;

/********************** Top-Level **********************/

/// \brief Initializes everything, make sure to call this before anything else
/// \param window Window that is used
void juInit(SDL_Window *window);

/// \brief Frees all resources, call at the end of the program
void juClose();

/********************** Font **********************/

/// \brief Data as it relates to storing a bitmap character for VK2D
struct JUCharacter {
	float x;    ///< x position of this character in the bitmap
	float y;    ///< y position of this character in the bitmap
	float w;    ///< width of the character in the bitmap
	float h;    ///< height of the character in the bitmap
	bool drawn; ///< For invisible characters that have width but need not be drawn (ie space)
};

/// \brief A bitmap font, essentially a sprite sheet and some characters
struct JUFont {
	uint32_t unicodeStart;   ///< Code point of the first character in the image (inclusive)
	uint32_t unicodeEnd;     ///< Code point of the last character in the image (exclusive)
	float newLineHeight;     ///< Height of a newline (calculated as the max character height)
	JUCharacter *characters; ///< Vector of characters
	VK2DTexture bitmap;      ///< Bitmap of the characters
	VK2DImage image;         ///< Bitmap image in case it was loaded from a jufnt
};

/// \brief Loads a font from a .jufnt file (create them with the python script)
/// \return Returns a new font or NULL if it failed
JUFont juFontLoad(const char *filename);

/// \brief Loads a font from a
/// \param image Filename of the image to use for the font
/// \param unicodeStart First character in the image (inclusive)
/// \param unicodeEnd Last character in the range (exclusive)
/// \param w Width of each character in the image
/// \param h Height of each character in the image
/// \return Returns a new font or NULL if it failed
///
/// This can only load mono-spaced fonts and it expects the font to have at least
/// an amount of characters in the image equal to unicodeEnd - unicodeStart.
JUFont juFontLoadFromImage(const char *image, uint32_t unicodeStart, uint32_t unicodeEnd, float w, float h);

/// \brief Frees a font
void juFontFree(JUFont font);

/// \brief Draws a font to the screen (supports all printf % things)
///
/// Since this uses Vulkan2D to draw the current colour of the VK2D
/// renderer is used.
///
/// vsprintf is used internally, so any and all printf % operators work
/// in this. Newlines (\n) are also allowed.
void juFontDraw(JUFont font, float x, float y, const char *fmt, ...);

/// \brief Draws a font to the screen, wrapping every w pixels (supports all printf % things)
///
/// Since this uses Vulkan2D to draw the current colour of the VK2D
/// renderer is used.
///
/// vsprintf is used internally, so any and all printf % operators work
/// in this. Newlines (\n) are also allowed.
void juFontDrawWrapped(JUFont font, float x, float y, float w, const char *fmt, ...);

/********************** Asset Manager **********************/

/// \brief Can hold any asset
struct JUAsset {
	JUAssetType type; ///< Type of asset this is
	const char *name; ///< Name of this asset for bucket collision checking
	JUAsset next;     ///< Next asset in this slot should there be a hash collision

	union {
		VK2DTexture tex; ///< Texture bound to this asset
		JUFont font;     ///< Font bound to this asset
		JUSound sound;   ///< Sound bound to this asset
	} Asset; ///< Only need to store one at a time
};

/// \brief Stores, loads, and frees many assets at once
struct JULoader {
	JUAsset *assets; ///< Bucket of assets
};

/// \brief Creates an asset loader, loading all the specified files
/// \param files List of files to load, their filenames will be their key
/// \param fileCount Number of files that will be loaded
/// \return Returns a new JULoader or NULL
///
/// What type of asset is trying to be loaded will be discerned by its extension.
/// Supported extensions are jpg, png, bmp, wav and jufnt
JULoader juLoaderCreate(const char **files, uint32_t fileCount);

/// \brief Gets a texture from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
VK2DTexture juLoaderGetTexture(JULoader loader, const char *filename);

/// \brief Gets a texture from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUFont juLoaderGetFont(JULoader loader, const char *filename);

/// \brief Gets a sound from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUSound juLoaderGetSound(JULoader loader, const char *filename);

/// \brief Frees a JULoader and all the assets it loaded
void juLoaderFree(JULoader loader);

/********************** Audio **********************/

/// \brief A sound to be soundInfo
struct JUSound {
	cs_loaded_sound_t sound;
	cs_play_sound_def_t soundInfo;
};

/// \brief A currently playing sound
struct JUPlayingSound {
	cs_playing_sound_t *playingSound;
};

/// \brief Loads a sound from a file into memory - right now only WAV files are supported
JUSound juSoundLoad(const char *filename);

/// \brief Plays a sound
/// \return Returns a playing sound handle you can use to update/stop the sound, but it doesn't need
/// to be stored (it won't cause a memory leak)
JUPlayingSound juSoundPlay(JUSound sound, bool loop, float volumeLeft, float volumeRight);

/// \brief Change the properties of a currently playing sound
void juSoundUpdate(JUPlayingSound sound, bool loop, float volumeLeft, float volumeRight);

/// \brief Stops a sound if its currently playing
void juSoundStop(JUPlayingSound sound);

/// \brief Frees a sound from memory
void juSoundFree(JUSound sound);

/// \brief Stops all currently playing sounds
void juSoundStopAll();

/********************** File I/O **********************/

/// \brief A piece of data stored in a save
struct JUData {
	JUDataType type; ///< Type of this data
	const char *key; ///< Key of this data

	union {
		int64_t i64;        ///< 64 bit signed int
		uint64_t u64;       ///< 64 bit unsigned int
		float f32;          ///< 32 bit float
		double f64;         ///< 64 bit float
		const char *string; ///< String
		struct {
			void *data;    ///< Raw data
			uint32_t size; ///< Size in bytes of the data
		} data;
	} Data;
};

/// \brief Save data for easily saving and loading many different types of data
struct JUSave {
	uint32_t size; ///< Number of "datas" stored in this save
	JUData *data;  ///< Vector of data
};

/// \brief Loads a save from a save file or returns an empty save if the file wasn't found
///
/// These are basically just tables of data, you use a key to set some data then use a key
/// to later find that data again. The real functionality comes in the form of saving and
/// loading from files with it, since you can save all your data in a human-readable form
/// and load it right into your game easily.
///
/// These aren't particularly fast, and they are not meant to be used every frame non-stop,
/// especially in larger games.
JUSave juSaveLoad(const char *filename);

/// \brief Saves a save to a file
void juSaveStore(JUSave save, const char *filename);

/// \brief Frees a save from memory
void juSaveFree(JUSave save);

/// \brief Sets some data in a save
void juSaveSetInt64(JUSave save, const char *key, int64_t data);

/// \brief Gets some data from a save
int64_t juSaveGetInt64(JUSave save, const char *key);

/// \brief Sets some data in a save
void juSaveSetUInt64(JUSave save, const char *key, uint64_t data);

/// \brief Gets some data from a save
uint64_t juSaveGetUInt64(JUSave save, const char *key);

/// \brief Sets some data in a save
void juSaveSetFloat(JUSave save, const char *key, float data);

/// \brief Gets some data from a save
float juSaveGetFloat(JUSave save, const char *key);

/// \brief Sets some data in a save
void juSaveSetDouble(JUSave save, const char *key, double data);

/// \brief Gets some data from a save
double juSaveGetDouble(JUSave save, const char *key);

/// \brief Sets some data in a save
void juSaveSetString(JUSave save, const char *key, const char *data);

/// \brief Gets some data from a save
/// \warning The pointer/data belongs to the save itself and will be freed with it - copy it if you need it longer
const char *juSaveGetString(JUSave save, const char *key);

/// \brief Sets some data in a save
///
/// The save will make a local copy of the data
void juSaveSetData(JUSave save, const char *key, void *data, uint32_t size);

/// \brief Gets some data from a save
/// \warning The pointer/data belongs to the save itself and will be freed with it - copy it if you need it longer
void *juSaveGetData(JUSave save, const char *key, uint32_t *size);


/********************** Collisions **********************/

/// \brief A simple rectangle
struct JURectangle {
	float x; ///< x position of the top left of the rectangle
	float y; ///< y position of the top left of the rectangle
	float w; ///< Width of the rectangle
	float h; ///< Height of the rectangle
};

/// \brief A simple circle
struct JUCircle {
	float x; ///< x position of the center of the circle
	float y; ///< y position of the center of the circle
	float r; ///< Radius in pixels
};

/// \brief Gets the angle between two points
float juPointAngle(float x1, float y1, float x2, float y2);

/// \brief Gets the distance between two points
float juPointDistance(float x1, float y1, float x2, float y2);

/// \brief Checks for a collision between two rectangles
bool juRectangleCollision(JURectangle *r1, JURectangle *r2);

/// \brief Checks for a collision between two circles
bool juCircleCollision(JUCircle *c1, JUCircle *c2);

/// \brief Checks if a point exists within a given rectangle
bool juPointInRectangle(JURectangle *rect, float x, float y);

/// \brief Checks if a point exists within a given circle
bool juPointInCircle(JUCircle *circle, float x, float y);