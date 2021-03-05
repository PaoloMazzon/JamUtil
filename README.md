JamUtil
=======
JamUtil is a very simple little framework meant to be used on top of [Vulkan2D](https://github.com/PaoloMazzon/Vulkan2D)
with the purpose being use in game jams. These tools are not the best in class, they are just
easy to use on top of VK2D since game jams have very limited time.

Features
========

 + Asset loader to easily load and free many resources at once

Planned Features
================

 + Audio library
 + Bitmap fonts
 + Basic game math/physics
 + Some collisions
 
Usage
=====
This is meant to be used the same way Vulkan2D is, in that you just add it as a submodule
and include the few files in your CMake manually.

.jufnt Font Files
=================
Loading and drawing TTF fonts not only takes a lot more work and performance, it basically
requires the usage of another library. For the sake of keeping this as a single source-header
ordeal, only bitmap fonts are supported. To this end, jufnt files exist to make bitmap
fonts super easy to deal with. In order to use a jufnt file,

 1. Get a preferred ttf font
 2. Run `python GenFont.py <font-name> <font-size>`
 3. Load the generated `.jufnt` file with `juFontLoad("myfont.jufnt");` (or with the asset loader)
 
That's it. The Python script will take the ttf font and turn it into a bitmap font, also storing
the size of each character as to allow for non-mono-spaced bitmap fonts. Furthermore, no other
files are required to load the font other than the jufnt file as it stores the image and all
character sizes. See below for the binary file format.

Also you can just manually load images as bitmap fonts too, this is just a fancy feature.

.jufnt File Format
==================
In order with no padding, this is the binary file format `.jufnt`.

 + 5 bytes at the top that just say `JUFNT`
 + 4 bytes representing the size in bytes of the png at the bottom
 + 4 bytes for the number of characters in the font
 + 4 bytes * number of characters denoting each character's size in pixels (2 bytes for width, 2 bytes for height)
 + X bytes for the png stored at the end of the file

Or more visually,

    JUFNTSIZESIZE101010101010101010101010PNGPNGPNGPNGPNGPNGPNGPNGPNGPNG
   
Kind of a poor representation but it's something.