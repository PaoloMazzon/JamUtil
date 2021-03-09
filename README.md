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
 + Simplified keyboard controls
 + Basic collisions
 
Usage
=====
This is meant to be used the same way Vulkan2D is, in that you just add it as a submodule
and include the few files in your CMake manually. On Windows you may need to link `dsound`
as well depending on your build environment.

Make sure to call `juInit` after SDL/VK2D and `juQuit` before you clean them up. Also call
`juUpdate` before the SDL events loop each frame or certain systems won't work. There is also
`juDelta` which will simply return the amount of time in seconds the last frame took to process.

Asset Loader
------------
The asset loader is used to load many different assets with very little code. The general
idea is you make a list of assets you want to load and pass it to the loader and it will
attempt to load them all based on the file extension. If the extension is not recognized,
it will be loaded as a `JUBuffer` (see buffers below).

    JULoadedAsset FILES[] = {{...}};
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

The data you pass to the loader to load files is usually just the path, but it must be wrapped
in a `JULoadedAsset` struct, which allows for the loading of sprite sheets and possibly other
things that would require parameters to be created with.

    JULoadedAsset FILES[] = {
        {"myspritesheet.png", 10, 10, 20, 20, 0.2, 5},
        {"myaudioclip.wav"},
        {"myfont.jufnt"},
        {"somebinaryfile.bin"},
    };

Of course check `JamUtil.h` for the documentation on `JULoadedAsset`. 

Sprites
-------
Sprites are pretty much what you would expect, you can load sprite sheets and draw
animations. They also store some metadata like origin and rotation that you can
change whenever. They are simple to load from sprite sheets, just specify the file
and give some parameters and JamUtil will figure out how to draw it (see `main.c`
for an example using a weird sprite sheet).

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

Example CMake
=============
It can be a bit complicated to understand and add both libraries (Vulkan2D and JamUtil) so
here is a "working" (I made one that worked then put it here to explain) CMakeLists.txt
so you can mostly just copy/paste this to start.

    project(YOUR_GAME)
    ...
    # At the top we find the two required packages: Vulkan and SDL2. You may need to link SDL2 manually on Windows or use FindSDL2.cmake
    find_package(Vulkan)
    find_package(SDL2 REQUIRED)
    
    # For the sake of readability all of the needed files are put into variables, this should be a straight copy/paste
    set(VMA_FILES Vulkan2D/VulkanMemoryAllocator/src/vk_mem_alloc.h Vulkan2D/VulkanMemoryAllocator/src/VmaUsage.cpp)
    file(GLOB VK2D_FILES Vulkan2D/VK2D/*.c)
    set(JAMUTIL_FILES JamUtil/JamUtil.c)
    
    # Obviously include/link whatever else you need here, but this is the minimum required stuff
    include_directories(Vulkan2D/ JamUtil/ ${SDL2_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})
    add_executable(${PROJECT_NAME} main.c ${JAMUTIL_FILES} ${VK2D_FILES} ${VMA_FILES})
    
    # You also may or may not need to link "dsound" depending on your compiler/environment
    target_link_libraries(${PROJECT_NAME} m ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})

And from there you can use VK2D and JamUtil to your heart's content. See `main.c` if you want a
quick setup example (be sure to change the assets directory or remove them).

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