# Platform Library

Here is all the platform specific code.

## nes

Contains the startup code, PPU/OAM/palettes, and scrolling.

This is based off of Shiru's NESlib. https://shiru.untergrund.net/code.shtml

## util

These are functions for CC65. I'm only supporting the most useful but you can certainly add more from CC65's code repository.

## mapper

Here is where the mapper specific code is at. For the most part these are bank switching functions that have been abstracted to be mixed and matched.

## Donut compression

I'm utilizing Donut compression for CHR data loading. You can delete this if you are using CHR ROM but you'll need to modify some other sections of the code.

https://jroatch.nfshost.com/donut-nes/

