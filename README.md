JamUtil
=======
JamUtil is a very simple little framework meant to be used on top of [Vulkan2D](https://github.com/PaoloMazzon/Vulkan2D)
with the purpose being use in game jams. These tools are not the best in class, they are just
easy to use on top of VK2D since game jams have very limited time.

Planned Features
================

 + Audio library
 + Asset management for textures and audio
 + Basic game math/physics
 + Some collisions
 
Usage
=====
This is meant to be used the same way Vulkan2D is, in that you just add it as a submodule
and include the few files in your CMake manually.

.jufont File
============
In order with no padding, this is the binary file format `.jufont`.

 + 5 bytes at the top that just say `JUFNT`
 + 4 bytes representing the size in bytes of the png at the bottom
 + 4 bytes for the number of characters in the font
 + 4 bytes * number of characters denoting each character's size in pixels (2 bytes for width, 2 bytes for height)
 + X bytes for the png stored at the end of the file

Or more visually,

    JUFNTSIZESIZE101010101010101010101010PNGPNGPNGPNGPNGPNGPNGPNGPNGPNG
   
Kind of a poor representation but it's something.