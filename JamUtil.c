/// \file JamUtil.c
/// \author Paolo Mazzon
#include "JamUtil.h"

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

JULoader juLoaderCreate(const char **files, uint32_t fileCount) {
	// TODO: This
}

VK2DTexture juLoaderGetTexture(JULoader loader, const char *filename) {
	// TODO: This
}

JUFont juLoaderGetFont(JULoader loader, const char *filename) {
	// TODO: This
}

void juLoaderFree(JULoader loader) {
	// TODO: This
}