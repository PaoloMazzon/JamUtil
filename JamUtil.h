/// \file JamUtil.h
/// \author Paolo Mazzon
/// \brief A small collection of tools for quick game-dev with Vulkan2D
#pragma once
#include <stdbool.h>
#include <VK2D/VK2D.h>

/********************** Typedefs **********************/
typedef struct JUCharacter JUCharacter;
typedef struct JUFont *JUFont;
typedef struct JUAsset *JUAsset;
typedef struct JULoader *JULoader;

/********************** Enums **********************/
typedef enum {
	JU_ASSET_TYPE_NONE = 0, // Specifically for checking if there is an asset at all
	JU_ASSET_TYPE_FONT = 1,
	JU_ASSET_TYPE_TEXTURE = 2,
	JU_ASSET_TYPE_SOUND = 3,
	JU_ASSET_TYPE_MAX = 4,
} JUAssetType;

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
		// TODO: Sound
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
/// Supported extensions are jpg, png, bmp, ogg, wav, mp3, and jufnt
JULoader juLoaderCreate(const char **files, uint32_t fileCount);

/// \brief Gets a texture from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
VK2DTexture juLoaderGetTexture(JULoader loader, const char *filename);

/// \brief Gets a texture from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUFont juLoaderGetFont(JULoader loader, const char *filename);

/// \brief Gets a sound from the loader
/// \return Returns the requested asset or NULL if it doesn't exist
JUFont juLoaderGetSound(JULoader loader, const char *filename);

/// \brief Frees a JULoader and all the assets it loaded
void juLoaderFree(JULoader loader);

/********************** Math/Physics **********************/
// TODO: This

/********************** Audio **********************/
// TODO: This