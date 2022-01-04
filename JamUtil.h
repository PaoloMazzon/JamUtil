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
typedef struct JUPoint2D JUPoint2D;
typedef struct JUCircle JUCircle;
typedef struct JUData JUData;
typedef struct JUSave *JUSave;
typedef struct JUBuffer *JUBuffer;
typedef struct JUSprite *JUSprite;
typedef struct JULoadedAsset JULoadedAsset;
typedef struct JUJob JUJob;

/********************** Enums **********************/

/// \brief Types of assets stored in the loader
typedef enum {
	JU_ASSET_TYPE_NONE = 0,
	JU_ASSET_TYPE_FONT = 1,
	JU_ASSET_TYPE_TEXTURE = 2,
	JU_ASSET_TYPE_SOUND = 3,
	JU_ASSET_TYPE_BUFFER = 4,
	JU_ASSET_TYPE_SPRITE = 5,
	JU_ASSET_TYPE_MAX = 6,
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
/// \param jobChannels Number of channels for jobs (8 is a good number, 0 if you're not using jobs)
void juInit(SDL_Window *window, int jobChannels);

/// \brief Keeps various systems up to date, call every frame at the start before the SDL event loop
void juUpdate();

/// \brief Frees all resources, call at the end of the program
void juQuit();

/// \brief Returns the time in seconds that the last frame took
double juDelta();

/// \brief Returns the time in seconds since juInit was called
double juTime();

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

/********************** Buffer **********************/

/// \brief Simple buffer to make loading binary easier
struct JUBuffer {
	void *data;    ///< Data stored in this buffer
	uint32_t size; ///< Size of the data stored in the buffer
};

/// \brief Loads a buffer from a file
JUBuffer juBufferLoad(const char *filename);

/// \brief Creates a buffer from given data, the data will be copied to the buffer
JUBuffer juBufferCreate(void *data, uint32_t size);

/// \brief Saves a buffer to a file
void juBufferSave(JUBuffer buffer, const char *filename);

/// \brief Frees a buffer from memory
void juBufferFree(JUBuffer buffer);

/// \brief Saves some data to a file without the need for a buffer
void juBufferSaveRaw(void *data, uint32_t size, const char *filename);

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

/// \brief Returns true if the key exists in the save file
bool juSaveKeyExists(JUSave save, const char *key);

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


/********************** Collisions/Math **********************/

/// \brief A simple rectangle
struct JURectangle {
	double x; ///< x position of the top left of the rectangle
	double y; ///< y position of the top left of the rectangle
	double w; ///< Width of the rectangle
	double h; ///< Height of the rectangle
};

/// \brief A simple circle
struct JUCircle {
	double x; ///< x position of the center of the circle
	double y; ///< y position of the center of the circle
	double r; ///< Radius in pixels
};

/// \brief A 2D coordinate
struct JUPoint2D {
	double x; ///< x position in 2D space
	double y; ///< y position in 2D space
};

/// \brief Gets the angle between two points
double juPointAngle(double x1, double y1, double x2, double y2);

/// \brief Gets the distance between two points
double juPointDistance(double x1, double y1, double x2, double y2);

/// \brief Rotates a point in 2D space about an (absolute) origin
JUPoint2D juRotatePoint(double x, double y, double originX, double originY, double rotation);

/// \brief Checks for a collision between two rectangles
bool juRectangleCollision(JURectangle *r1, JURectangle *r2);

/// \brief Checks for a collision between two rotated rectangles
bool juRotatedRectangleCollision(JURectangle *r1, double rot1, double originX1, double originY1, JURectangle *r2, double rot2, double originX2, double originY2);

/// \brief Checks for a collision between two circles
bool juCircleCollision(JUCircle *c1, JUCircle *c2);

/// \brief Checks if a point exists within a given rectangle
bool juPointInRectangle(JURectangle *rect, double x, double y);

/// \brief Checks if a point exists within a given rotated rectangle
bool juPointInRotatedRectangle(JURectangle *rect, double rot, double originX, double originY, double x, double y);

/// \brief Checks if a point exists within a given circle
bool juPointInCircle(JUCircle *circle, double x, double y);

/// \brief Linear interpolation (given a start, stop, and percent it returns the point x% along that distance)
double juLerp(double percent, double start, double stop);

/// \brief Same as lerp but on a sin graph instead of a linear graph (for smooth transitions)
double juSerp(double percent, double start, double stop);

/// \brief Casts a ray out at a given angle and returns the x component
double juCastX(double length, double angle);

/// \brief Casts a ray out at a given angle and returns the y component
double juCastY(double length, double angle);

/********************** Keyboard **********************/

/// \brief Checks if a key is currently pressed
bool juKeyboardGetKey(SDL_Scancode key);

/// \brief Checks if a key was just pressed
bool juKeyboardGetKeyPressed(SDL_Scancode key);

/// \brief Checks if a key is currently pressed
bool juKeyboardGetKeyReleased(SDL_Scancode key);

/********************** Animations **********************/

/// \brief Information for sprites
///
/// No "cells" are stored because the image coordinates that need to be
/// drawn are calculated on the fly.
struct JUSprite {
	struct {
		uint64_t lastTime; ///< Last time the animation was updated
		uint32_t frames;   ///< Number of frames in the animation
		uint32_t frame;    ///< Current frame in the animation
		float w;           ///< Width of each cell
		float h;           ///< Height of each cell
		VK2DTexture tex;   ///< Sprite sheet
		bool copy;         ///< If this is a copy of a sprite or not (for the purposes of only freeing the texture once)
	} Internal;     ///< Data for the sprite to keep track of itself
	double delay;   ///< Time in seconds a single frame lasts
	float x;        ///< X position in the texture where the sprite sheet starts
	float y;        ///< Y position in the texture where the sprite sheet starts
	float originX;  ///< X origin of the sprite (used for drawing position and rotation)
	float originY;  ///< Y origin of the sprite (used for drawing position and rotation)
	float scaleX;   ///< X scale of the sprite
	float scaleY;   ///< Y scale of the sprite
	float rotation; ///< Rotation of the sprite
};

/// \brief Loads an animation from a sprite sheet file
/// \param filename Image file of the spritesheet
/// \param x X in the sprite sheet where the cells begin
/// \param y Y in the sprite sheet where the cells begin
/// \param w Width of each cell
/// \param h Height of each cell
/// \param delay Delay in seconds between animation frames
/// \param frames Animation frame count (a value of zero will be interpreted as 1)
/// \return Returns a new sprite or NULL if it failed
JUSprite juSpriteCreate(const char *filename, float x, float y, float w, float h, float delay, int frames);

/// \brief The same as `juSpriteCreate` except it creates a sprite from an already existing texture (which is not freed with the sprite)
/// \return Returns a new sprite that uses `tex` but does not technically own `tex`
JUSprite juSpriteFrom(VK2DTexture tex, float x, float y, float w, float h, float delay, int frames);

/// \brief Makes a copy of a sprite (usually only use this with a loader)
/// \param original Sprite to copy from
/// \warning The sprite copy does not own the texture pointer and if the original is freed the copy can still
/// be safely freed but it can no longer be used.
JUSprite juSpriteCopy(JUSprite original);

/// \brief Draws an animation, advancing the current frame if enough time has passed
void juSpriteDraw(JUSprite spr, float x, float y);

/// \brief Draws a specific frame of a frame, not doing any sprite updating or anything
void juSpriteDrawFrame(JUSprite spr, uint32_t index, float x, float y);

/// \brief Frees an animation from memory
void juSpriteFree(JUSprite spr);

/********************** Jobs System **********************/

/// \brief Description of a job
struct JUJob {
	int channel;        ///< Channel the job is on
	void (*job)(void*); ///< Job function
	void *data;         ///< Data to pass to the function when its executed
};

/// \brief Queues a job to be run as soon as a worker thread is available
void juJobQueue(JUJob job);

/// \brief Waits for all jobs on a channel to be completed
void juJobWaitChannel(int channel);

/********************** Asset Manager **********************/

/// \brief Data used to tell the loader what to load
///
/// Specifying a width/height/delay for an image tells the loader that the image
/// should be treated as a sprite.
struct JULoadedAsset {
	const char *path; ///< Path to the asset to load
	float x;          ///< If its a sprite, this is the x in the sheet where the cells start
	float y;          ///< If its a sprite, this is the y in the sheet where the cells start
	float w;          ///< If its a sprite, this is the width of each cell in the animation
	float h;          ///< If its a sprite, this is the height of each cell in the animation
	float delay;      ///< If its a sprite, this is the delay in seconds between animation frames
	int frames;       ///< Number of frames in the animation, 0 is assumed to be 1
	float originX;    ///< If its a sprite, this is the x origin of the sprite
	float originY;    ///< If its a sprite, this is the y origin of the sprite
};

/// \brief Can hold any asset
struct JUAsset {
	JUAssetType type; ///< Type of asset this is
	const char *name; ///< Name of this asset for bucket collision checking
	JUAsset next;     ///< Next asset in this slot should there be a hash collision

	union {
		VK2DTexture tex; ///< Texture bound to this asset
		JUFont font;     ///< Font bound to this asset
		JUSound sound;   ///< Sound bound to this asset
		JUBuffer buffer; ///< Buffer bound to this asset
		JUSprite sprite; ///< Sprite bound to this asset
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
/// Supported extensions are jpg, png, bmp, wav and jufnt. Any other file extension loaded
/// through this function will be loaded as a buffer (so feel free to load whatever
/// files you want, but anything not loaded as a specific type will be loaded as
/// a JUBuffer).
JULoader juLoaderCreate(JULoadedAsset *files, uint32_t fileCount);

/// \brief Gets a texture from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
VK2DTexture juLoaderGetTexture(JULoader loader, const char *filename);

/// \brief Gets a texture from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUFont juLoaderGetFont(JULoader loader, const char *filename);

/// \brief Gets a sound from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUSound juLoaderGetSound(JULoader loader, const char *filename);

/// \brief Gets a buffer from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUBuffer juLoaderGetBuffer(JULoader loader, const char *filename);

/// \brief Gets a sprite from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUSprite juLoaderGetSprite(JULoader loader, const char *filename);

/// \brief Frees a JULoader and all the assets it loaded
void juLoaderFree(JULoader loader);