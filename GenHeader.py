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
	pass # TODO: This

# assembles a C array and struct for the assets given filenames and returns it
def assembleC(files):
	pass # TODO: This

def main():
	arguments = findArgs()
	if "dir" in arguments and "var" in arguments and "struct" in arguments and "o" in arguments:
		# final output txt
		outputFile = "#pragma once\n\n"
		if "header" in arguments: outputFile = fileAsString(arguments["header"]) + "\n\n"

		# find all files in the selected directory
		files = findFiles(arguments["dir"], "recursive" in arguments)

		# construct the meat of the file
		outputFile += assembleC(files)

		# output to file
		with open(arguments["o"], "w") as f:
			f.write(outputFile + "\n")
			if "footer" in arguments: f.write("\n" + fileAsString(arguments["footer"]))

	else:
		printHelp()

if __name__ == "__main__":
	main()
