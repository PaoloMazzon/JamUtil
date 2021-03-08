JamUtil
=======
JamUtil is a very simple little framework meant to be used on top of [Vulkan2D](https://github.com/PaoloMazzon/Vulkan2D)
with the purpose being use in game jams. These tools are not the best in class, they are just
easy to use on top of VK2D since game jams have very limited time.

Features
========

 + Asset loader to easily load and free many resources at once
 + Simple bitmap fonts, monospaced and automatically generated
 + Sound support (Windows, Mac, and possibly Linux only)
 + Easily save and load many different types of data
 + Save/load buffers to/from files easily
 + Basic collisions
 
Usage
=====
This is meant to be used the same way Vulkan2D is, in that you just add it as a submodule
and include the few files in your CMake manually. On Windows you may need to link `dsound`
as well depending on your build environment. Aside from that, just make sure to call `juInit`
and `juClose`.

Asset Loader
------------
The asset loader is used to load many different assets with very little code. The general
idea is you make a list of assets you want to load and pass it to the loader and it will
attempt to load them all based on the file extension. If the extension is not recognized,
it will be loaded as a `JUBuffer` (see buffers below).

    const char *FILES[] = {...};
    const int FILE_COUNT = 10;
    JULoader loader = juLoaderCreate(FILES, FILE_COUNT);
    ...
    juLoaderGetTexture(loader, "assets/myasset.png");
    ...
    juLoaderFree(loader);

You grab assets from the loader by filename. Internally a hash map is used, so it is a quick
process to grab assets, but it is still recommended that you store the assets locally if you
plan on grabbing a bunch every frame.

Supported loader file extensions are 

 + `.png`, `.jpg`, `.jpeg`, `.bmp` will create a VK2D texture
 + `.jufnt` will create a font (see fonts below)
 + `.wav` will create a sound (see sounds below)
 + all else will be loaded as a buffer

Fonts
-----
There are two ways you can use fonts, you can either use `.jufnt` files which automate loading
fonts, or manually load a bitmap font. You can create `.jufnt` files by calling `GenFont.py`
with a path to a ttf font and the size, which will put the newly generated font in the same
folder as the ttf (with the same name but proper extension). You can then load these with 
`juFontLoad` (or just use the loader). If you want to use an image, you just supply the image's
filename and some other data, check the documentation in the header for more info.

    juFontDraw(font, 50, 50, "The quick brown fox jumps over the lazy dog.");
    
It should be noted that `.jufnt`s are strongly recommended because it is a tiresome process
to manually create bitmap fonts, and `.jufnt`s allow for non-mono-spaced bitmap fonts. So
here is a basic process for it

    # In your command prompt of choice
    python GenFont.py myfont.ttf 15
    
    # In your code (or just load with the loader)
    JUFont font = juFontLoad("myfont.jufnt");
    ...
    juFontFree(font);
    
Sounds
------
Sounds are quite simple. You load them and play them. You can also update their volume as they
play. Nothing too fancy, but gets the job done for simple games. Just check the in-header
documentation for sounds because it pretty much operates exactly how you would want it to.

Saves
-----
These are a handy tool to easily save and load many different types of data. They are basically
just hash tables that support multiple different types. They save as binary files so they
are not easily edited by the user (but obviously this is open source and the specification for
the save format is below and its not at all encrypted). 

    // This will create an empty save if the file doesn't exist
    JUSave save = juSaveLoad("thing.jusav");
    ...
    juSaveSetString(save, "some key", "some string");
    const char *myString = juSaveGetString(save, "some key");
    ...
    juSaveStore(save, "thing.jusav");
    juSaveFree(save);

Supported types to save and load are

 + Floats
 + Doubles
 + 64-bit signed ints
 + 64-bit unsigned ints
 + Strings
 + Void pointers (it saves the size and data itself)

Buffers
-------
Buffers are the simplest piece of JamUtil, they mostly exist to help the loader. It can

 + Copy files into buffers
 + Save buffers to files
 + Save raw `void*` pointers to file

Very simple, just exists to make some tasks simple.

Collisions
----------
The collision "subsystem" includes several functions that are all very simple to understand
and use, so here they are

 + Check for collisions between two rectangles
 + Check for collisions between two circles
 + Check for a point in a rectangle
 + Check for a point in a circle
 + Get the distance between two points
 + Get the angle between two points
 
Again, for specifics, just check the header. Everything is documented.

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
character sizes. See below for the binary file format if you wish.

Also you can just manually load images as bitmap fonts too, this is meant to easily create bitmap
fonts as it can often take a bit of time to manually create a bitmap font, especially when its
not mono-spaced.

.jufnt File Format
==================
In order with no padding, this is the binary file format `.jufnt`.

 + 5 bytes at the top that just say `JUFNT`
 + 4 bytes representing the size in bytes of the png at the bottom
 + 4 bytes for the number of characters in the font
 + 4 bytes * number of characters denoting each character's size in pixels (2 bytes for width, 2 bytes for height)
 + X bytes for the png stored at the end of the file

.jusav File Format
==================
This is not at all important for the average user to know, but for my own sake and those curious, the
save functionality uses the following format to save data

 + 5 bytes at the top that read `JUSAV`
 + 4 bytes stating the number of "datas" stored in the file
 + From here on out its just data, so for each piece of data
   + 4 bytes for the size of the key string
   + X bytes for the key string
   + 4 byte representing the type of data it is
     + If it is a fixed width piece of data (floats, ints), X bytes for that data
     + If it is a variable length thing (strings, void bytestreams)
       + 4 bytes for the size of the data
       + X bytes for the data itself