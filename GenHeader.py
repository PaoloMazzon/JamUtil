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

import sys
import os

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

# assembles a C header for the assets given filenames and returns it
def assembleHeader(files, structName, varName):
	structString = "typedef struct " + structName + " {\nJULoader loader;\n"
	variableString = "JULoadedAsset " + varName + "[] = {\n"
	codeString = "#ifdef " + structName.upper() + "_IMPLEMENTATION\n" + structName + " *build" + structName + "() {\n    " + structName + " *s = malloc(sizeof(struct " + structName + "));\n    s->loader = juLoaderCreate(" + varName + ", " + str(len(files)) + ");\n"

	for fileName in files:
		extension = fileName[fileName.rfind("."):]
		name = fileName[fileName.rfind("/") + 1:fileName.rfind(".")] # name of file without path or extension
		if extension in (".png", ".jpg", ".jpeg", ".bmp"): # texture
			structString += "    VK2DTexture tex" + name + ";\n"
			variableString += "    {\"" + name + "\"},\n"
			codeString += "    s->tex" + name + " = juLoaderGetTexture(s->loader, " + fileName + ");\n"
		elif extension in (".wav"): # audio
			structString += "    JUSound snd" + name + ";\n"
			variableString += "    {\"" + name + "\"},\n"
			codeString += "    s->snd" + name + " = juLoaderGetSound(s->loader, " + fileName + ");\n"
		elif extension in (".jufnt"): # font
			structString += "    JUFont fnt" + name + ";\n"
			variableString += "    {\"" + name + "\"},\n"
			codeString += "    s->fnt" + name + " = juLoaderGetFont(s->loader, " + fileName + ");\n"
		else: # buffer
			structString += "    JUBuffer buf" + name + ";\n"
			variableString += "    {\"" + name + "\"},\n"
			codeString += "    s->buf" + name + " = juLoaderGetBuffer(s->loader, " + fileName + ");\n"

	structString += "} " + structName + ";\n\n"
	variableString += "};\n\n"
	codeString += "}\n\nvoid destroy" + structName + "(" + structName + " *s) {\n    juLoaderFree(s->loader);\n    free(s);\n#endif\n"

	return variableString + structString + codeString

def main():
	arguments = findArgs()
	if "dir" in arguments and "var" in arguments and "struct" in arguments and "o" in arguments:
		# final output txt
		outputHeader = "#pragma once\n\n"
		outputSource = "#include \"" + arguments["o"] + "\""
		if "header" in arguments: outputHeader = fileAsString(arguments["header"]) + "\n\n"

		# find all files in the selected directory
		files = findFiles(arguments["dir"], "recursive" in arguments)

		# construct the header
		outputHeader += assembleHeader(files)

		# output source and header
		with open(arguments["o"], "w") as f:
			f.write(outputHeader + "\n")
			if "footer" in arguments: f.write("\n" + fileAsString(arguments["footer"]))

	else:
		printHelp()

if __name__ == "__main__":
	main()
