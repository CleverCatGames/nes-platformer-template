# Platformer Source

This is the main source directory for the game. It contains game and player logic.

## Basic Overview

### main.c

Handles the overall game state. Has a fade function, start/reset game, and general timer.

### variables.c and zeropage.s

All the data exists here. Register everything as a global and then reference it with an extern. Why do it this way? Because you can control the ordering of variables and fix alignment issues easier by having it all in one place.

### level.c and megatile.s

Contains level loading and scrolling code.

### player.c

This is where all the player movement code is handled.

### object.c

Finds active objects and updates them. It's mostly a fancy loop with a bunch of assembly to make loading state variables really fast.

### ppu.c

Useful PPU functions to force vram to flush, or to turn on/off the PPU by resetting brightness and scrolling. Handy for menus.

### trigger.c

Deals with trigger zones. These are full height boxes that are 64 pixels in width. Used for level transitions, camera locking, and other things.

### button.s

Checks if buttons was pressed or released this frame by testing against the previous frame.

