# GenFont.py - Paolo Mazzon
# Generates .jufont files that can be loaded by JamUtil (from ascii 0 - 255)
# Requires pygame to use (pip install pygame)
# Usage: python GenFont.py font.ttf size
import pygame
from sys import argv
from os import remove
from io import BytesIO

def main():
	fontname = argv[1]
	size = int(argv[2])
	pygame.init()
	font = pygame.font.Font(fontname, size)
	characters = []
	size = [0, 0]
	sizes = []

	# generate all character
	for i in range(255):
		characters += [font.render(chr(i + 1), True, (255, 255, 255))]
		size[0] += characters[i].get_width()
		size[1] = characters[i].get_height() if size[1] < characters[i].get_height() else size[1]
		sizes += [[characters[i].get_width(), characters[i].get_height()]]

	# create and populate master surface
	output = pygame.surface.Surface(size, pygame.SRCALPHA, 32)
	output.fill((0, 0, 0, 0))
	x = 0
	for i in range(255):
		output.blit(characters[i], (x, 0))
		x += characters[i].get_width()

	outfile = b""
	pygame.image.save(output, "temp.png")
	with open("temp.png", "rb") as f:
		outfile = f.read()
	remove("temp.png")

	# we now have all the information we need to create the file
	with open(fontname[:fontname.rfind(".")] + ".jufnt", "wb") as f:
		f.write(b"JUFNT")
		f.write(len(outfile).to_bytes(4, "big"))
		f.write((255).to_bytes(4, "big"))
		for i in range(255):
			f.write(sizes[i][0].to_bytes(2, "big"))
			f.write(sizes[i][1].to_bytes(2, "big"))
		f.write(outfile)

if __name__ == "__main__":
	if (len(argv) != 3):
		print("Usage: python GenFont.py font.ttf size")
	else:
		main()