# GenHeader.py - Paolo Mazzon
# Generates a C header from a directory to be used with the asset loader
# Usage: python GenFont.py <arguments>
# REQUIRED ARGUMENTS:
#   -dir=<directory>
#   -var=<asset variable name>
#   -struct=<struct name>
#   -o=<outfile>
# Optional Arguments:
#   -header=<header file to be copied to the top of the file>
#   -footer=<footer file to be copied to the bottom of the file>
#   -recursive (recursively searches directory)
#   -map=<map file for mapping sprites>
#
# The file this script generates will declare the struct containing all assets
# and a function to create that struct and fill it with the necessary assets. In
# exactly one file #define <STRUCTNAME>_IMPLEMENTATION before including this and call
# build<Structname> to build the assets struct and return it. Don't forget to free
# it when you're done.
#
# To have this script recognize sprites, you must create a map file (which is just an
# ini file) with all the details of the sprites and pass it via `-map=<MAP>`. Each
# section of the ini should be the name of an image relative to `-dir` and the keys
# may be one of the following: x, y, w, h, delay, frames, origin_x, origin_y.

import sys
import os
import configparser

# returns a map of all the program arguments
def findArgs():
	args = {}
	for arg in sys.argv[1:]:
		parts = arg.split("=")
		args[parts[0][1:]] = parts[1] if (len(parts) > 1) else ""
	return args

# prints a simple help message
def printHelp():
	print("GenHeader.py - Paolo Mazzon")
	print("Generates a C header from a directory to be used with the asset loader")
	print("Usage: python GenFont.py <arguments>")
	print("REQUIRED ARGUMENTS:")
	print("  -dir=<directory>")
	print("  -var=<asset variable name>")
	print("  -struct=<struct name>")
	print("  -o=<outfile>")
	print("Optional Arguments:")
	print("  -header=<header file to be copied to the top of the file>")
	print("  -footer=<footer file to be copied to the bottom of the file>")
	print("  -recursive (recursively searches directory)")
	print("  -map=<map file for mapping sprites>")
	print("")
	print("The file this script generates will declare the struct containing all assets")
	print("and a function to create that struct and fill it with the necessary assets. In")
	print("exactly one file #define <STRUCTNAME>_IMPLEMENTATION before including this and call")
	print("build<Structname> to build the assets struct and return it. Don't forget to free")
	print("it when you're done.")
	print("")
	print("To have this script recognize sprites, you must create a map file (which is just an")
	print("ini file) with all the details of the sprites and pass it via `-map=<MAP>`. Each")
	print("section of the ini should be the name of an image relative to `-dir` and the keys")
	print("may be one of the following: x, y, w, h, delay, frames, origin_x, origin_y.")

# returns a file as a string
def fileAsString(filename):
	s = ""
	try:
		with open(filename, "r") as f:
			s = f.read()
	finally:
		pass # lmao idc
	return s

# locates all files in a directory and returns all of their paths in a list
def findFiles(directory, recursive):
	files = []
	for dir, dirList, fileList in os.walk(directory):
		if recursive or directory == dir:
			for fileName in fileList:
				files += [dir + "/" + fileName]
	return files

# writes the C variable code for a sprite given its ini section
def assembleSpriteCode(filename, section):
	out = "    {\"" + filename + "\", "
	# x, y, w, h, delay, frames
	out += (section["x"] if "x" in section else "0") + ", "
	out += (section["y"] if "y" in section else "0") + ", "
	out += (section["w"] if "w" in section else "0") + ", "
	out += (section["h"] if "h" in section else "0") + ", "
	out += (section["delay"] if "delay" in section else "0") + ", "
	out += (section["frames"] if "frames" in section else "1") + ", "
	out += (section["origin_x"] if "origin_x" in section else "0") + ", "
	out += (section["origin_y"] if "origin_y" in section else "0")
	out += "},\n"
	return out

# assembles a C header for the assets given filenames and returns it
def assembleHeader(files, varName, structName, mapFile):
	if not mapFile == None:
		parser = configparser.ConfigParser()
		parser.read(mapFile)
		totalAssets = len(files) + len(parser.sections())
	else:
		totalAssets = len(files)
	structString = "typedef struct " + structName + " {\n    JULoader loader;\n"
	variableString = "// Forward declare the asset array\nextern JULoadedAsset " + varName + "[" + str(totalAssets) + "];\n\n#ifdef " + structName.upper() + "_IMPLEMENTATION\nJULoadedAsset " + varName + "[] = {\n"
	codeString = "// Functions to create and destroy the asset struct\n" + structName + " *build" + structName + "();\nvoid destroy" + structName + "(" + structName + " *s);\n\n#ifdef " + structName.upper() + "_IMPLEMENTATION\n" + structName + " *build" + structName + "() {\n    " + structName + " *s = malloc(sizeof(struct " + structName + "));\n    s->loader = juLoaderCreate(" + varName + ", " + str(totalAssets) + ");\n"

	for fileName in files:
		fileName = fileName.replace("\\", "/")
		extension = fileName[fileName.rfind("."):]
		name = fileName[fileName.rfind("/") + 1:fileName.rfind(".")] # name of file without path or extension
		if extension in (".png", ".jpg", ".jpeg", ".bmp"): # texture
			structString += "    VK2DTexture tex" + name + ";\n"
			variableString += "    {\"" + fileName + "\"},\n"
			codeString += "    s->tex" + name + " = juLoaderGetTexture(s->loader, \"" + fileName + "\");\n"
		elif extension in (".wav"): # audio
			structString += "    JUSound snd" + name + ";\n"
			variableString += "    {\"" + fileName + "\"},\n"
			codeString += "    s->snd" + name + " = juLoaderGetSound(s->loader, \"" + fileName + "\");\n"
		elif extension in (".jufnt"): # font
			structString += "    JUFont fnt" + name + ";\n"
			variableString += "    {\"" + fileName + "\"},\n"
			codeString += "    s->fnt" + name + " = juLoaderGetFont(s->loader, \"" + fileName + "\");\n"
		else: # buffer
			structString += "    JUBuffer buf" + name + ";\n"
			variableString += "    {\"" + fileName + "\"},\n"
			codeString += "    s->buf" + name + " = juLoaderGetBuffer(s->loader, \"" + fileName + "\");\n"

	# Process map file last
	if not mapFile == None:
		parser = configparser.ConfigParser()
		parser.read(mapFile)
		for sprite in parser.sections():
			name = sprite[sprite.rfind("/") + 1:sprite.rfind(".")]
			structString += "    JUSprite spr" + name + ";\n"
			codeString += "    s->spr" + name + " = juLoaderGetSprite(s->loader, \"" + sprite + "\");\n"
			variableString += assembleSpriteCode(sprite, parser[sprite])

	structString += "} " + structName + ";\n\n"
	variableString += "};\n#endif\n\n"
	codeString += "    return s;\n}\n\nvoid destroy" + structName + "(" + structName + " *s) {\n    juLoaderFree(s->loader);\n    free(s);\n}\n#endif\n"

	return variableString + structString + codeString

def main():
	arguments = findArgs()
	if "dir" in arguments and "var" in arguments and "struct" in arguments and "o" in arguments:
		# final output txt
		outputHeader = "// This code was automatically generated by GenHeader.py\n#pragma once\n#include \"JamUtil.h\"\n\n"
		if "header" in arguments: outputHeader = fileAsString(arguments["header"]) + "\n\n"
		mapFile = arguments["map"] if "map" in arguments else None

		# find all files in the selected directory
		files = findFiles(arguments["dir"], "recursive" in arguments)

		# construct the header
		outputHeader += assembleHeader(files, arguments["var"], arguments["struct"], mapFile)

		# output source and header
		with open(arguments["o"], "w") as f:
			f.write(outputHeader)
			if "footer" in arguments: f.write("\n" + fileAsString(arguments["footer"]))

	else:
		printHelp()

if __name__ == "__main__":
	main()
