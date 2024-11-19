# Game Assets

Here you can find all the assets for the game.

## data

Contains CSV files with information for object types, triggers, and zones. This is an easy way to combine table data into an editable format.

## gfx

This is where to put grayscale png files for sprites and fonts. These get converted to the donut format to be loaded into CHR RAM.

## levels

These are Tiled level maps that contain the layout for each area. The `levels.lvl` file contains references to all levels used. Check out the comments in that file for more details.

## sound

Where the FamiStudio source files are stored. These need to be opened in FamiStudio and exported.

`assets/sound/music.fms` => `source/sound/music.s`

`assets/sound/sfx.fms` => `source/sound/sfx.s`

Make sure to export them as `FamiStudio Music/SFX Code` respectively and use the `CA65` format. Ordering matters and you'll also want to update `source/include/sfx.h` with the new sounds and music.

## tilesets

Contains the tilesets for the game. These are used by the Tiled maps in the `levels` folder.

Each tileset can be up to 128x256 in size (128 tiles).

The tileset graphic uses Aseprite's NES NTSC palette for conversion to 2bpp format but you can change the hex values in `tools/tsx2metatile.py`.

If you add a new tileset, make sure to add the meta information to `source/include/gfxdata.h`. Space can quickly become an issue so I tend to separate tilesets by bank.

