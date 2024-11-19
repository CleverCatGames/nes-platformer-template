#!/usr/bin/env python

"""
Converts Tiled TSX format to CHR and metatile data

PNG format uses specific palette (Aseprite NES NTSC) but you can change the nes_hex values to anything

1. The first step is to chunk the tileset into 8x8 tiles and generate palettes.

It attempts to create palettes by converting everything to a 2bpp format. There is potential that it will duplicate tile data so be aware of that.

If too many palettes are created or there are too many colors in a tile, the script will fail and give an error message.

2. The second step is to generate metatile data.

Using the type or class attribute in the TSX file it will generate types starting with MT_TYPE_.

For example, a class of `solid` turns into `MT_TYPE_SOLID`. `oneway` => `MT_TYPE_ONEWAY`.

These types need to be defined in `source/include/level.h`.
"""

import os
import argparse
import subprocess
import xml.etree.ElementTree as et
from img2chr import values_for_rgb, toCHRArray
from PIL import Image
from math import ceil
from collections import defaultdict
from util.rle import run_length_encode, chunks

def convert_to_rgb(color):
    return ((color & 0xFF0000) >> 16, (color & 0xFF00) >> 8, (color & 0xFF))

# aseprite nes ntsc palette
nes_hex = [
#      0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F
    0x525252, 0x011A51, 0x0F0F65, 0x230663, 0x36034B, 0x400426, 0x3F0904, 0x321300, 0x1F2000, 0x0B2A00, 0x002F00, 0x002E0A, 0x00262D, 0x000000, 0x000000, 0x000000,
    0xA0A0A0, 0x1E4A9D, 0x3837BC, 0x5828B8, 0x752194, 0x84235C, 0x822E24, 0x6F3F00, 0x515200, 0x316300, 0x1A6B05, 0x0E692E, 0x105C68, 0x000000, 0x000000, 0x000000,
    0xF8F8F8, 0x699EFC, 0x8987FF, 0xAE76FF, 0xCE6DF1, 0xE070B2, 0xDE7C70, 0xC8913E, 0xA6A725, 0x81BA28, 0x63C446, 0x54C17D, 0x56B3C0, 0x3C3C3C, 0x000000, 0x000000,
    0xFFFFFF, 0xBED6FD, 0xCCCCFF, 0xDDC4FF, 0xEAC0F9, 0xF2C1DF, 0xF1C7C2, 0xE8D0AA, 0xD9DA9D, 0xC9E29E, 0xBCE6AE, 0xB4E5C7, 0xB5DFE4, 0xA9A9A9, 0x000000, 0x000000,
]
nes_rgb = [convert_to_rgb(x) for x in nes_hex]

def rgb_dist(a, b):
    return sum([pow(x - y, 2) for x, y in zip(a, b)])

# keep a color dictionary to speed up the process
nes_color_lookup = dict((c, i) for i, c in enumerate(nes_hex))
nes_color_lookup[0x000000] = 0x0f # since there is more than one "black", pick 0x0f as the definitive version
def color_to_nes(color):
    if color not in nes_color_lookup:
        rgb = convert_to_rgb(color)
        dist = [rgb_dist(h, rgb) for h in nes_rgb]
        nes_color_lookup[color] = dist.index(min(dist))
    return nes_color_lookup[color]

def write_chunks(l, n):
    return ',\n'.join([', '.join(chunk) for chunk in chunks(l, n)])

def get_cell(pixels, x, y):
    cell = []
    s = ""
    for pixel_y in range(y, y + 8):
        for pixel_x in range(x, x + 8):
            r, g, b = pixels[pixel_x, pixel_y]
            pixel = (r << 16) | (g << 8) | b
            color = color_to_nes(pixel)
            s += str(((x % 8) << 9) | ((y % 8) << 6) | color)
            cell.append(color)
    return cell, hash(s)

def image_to_cells(image):
    cells = []
    cell_lookup = []
    cell_hashes = {}
    width, height = image.size
    pixels = image.load()
    for y in range(0, height, 8):
        for x in range(0, width, 8):
            cell, h = get_cell(pixels, x, y)
            if h in cell_hashes:
                cell_index = cell_hashes[h]
            else:
                cell_index = len(cells)
                cell_hashes[h] = cell_index
                cells.append(cell)
            cell_lookup.append(cell_index)
    return cell_lookup, cells

def cells_to_metatiles(cells, cols, rows):
    metatiles = []
    for y in range(0, rows, 2):
        for x in range(0, cols, 2):
            metatiles.append((
                cells[y * cols + x],
                cells[y * cols + x + 1],
                cells[(y + 1) * cols + x],
                cells[(y + 1) * cols + x + 1]
            ))
    return metatiles

def create_tileset_from_cells(cells, palettes):
    width = 128
    height = int(ceil(len(cells) / 16.0) * 8)
    x, y = 0, 0
    grayscale = [(0, 0, 0), (115, 115, 115), (210, 204, 203), (255, 255, 255)]
    pixels = [0] * (width * height)
    for cell in cells:
        palette = palettes.find_palette_for_colors(cell)
        gray_cell = [palette.get_color_index(color) for color in cell]
        for cy in range(8):
            for cx in range(8):
                ci = cy * 8 + cx  # cell index
                pi = (cy + y) * width + (cx + x)
                pixels[pi] = grayscale[gray_cell[ci] % 4]
        x += 8
        if x >= width:
            x = 0
            y += 8
    ts = Image.new("RGB", (width, height))
    ts.putdata(pixels)
    return ts

class Palette:
    def __init__(self, colors, pid):
        if len(colors) > 4:
            raise Exception("Too many colors in palette: " + str(colors))
        self.colors = list(colors)
        self.pid = pid

    def get_color_index(self, color):
        return self.colors.index(color)

    def add_color(self, color):
        if len(self.colors) >= 4:
            return False
        if color not in self.colors:
            self.colors.append(color)
        return True

    def match(self, colors):
        for color in colors:
            if color not in self.colors and not self.add_color(color):
                return False
        return True

    def set_bkg_color(self, color):
        if color in self.colors:
            self.colors.pop(self.colors.index(color))
        self.colors.insert(0, color)

    def __str__(self):
        return '[%s]' % ', '.join(map(lambda x: '#%06x' % x, self.colors))

    __repr__ = __str__

class PaletteList:
    palettes = []
    bkg_color = None

    def generate_palettes_from_metatiles(self, metatiles, cells):
        color_sets = []
        for tile in metatiles:
            colors = frozenset([color for cell in set(tile) for color in cells[cell]])
            # verify there are no more than 4 colors in each 8x8 cell
            if len(colors) > 4:
                raise Exception("Too many colors in cell {0}".format(tile))
            color_sets.append(colors)

        # sort unique color sets by number of colors in decreasing order
        sorted_sets = sorted(set(color_sets), key=lambda x: len(list(x)), reverse=True)
        # find or generate new palettes for each set of colors
        for colors in sorted_sets:
            self.find_palette_for_colors(colors)

        # match the metatile colors with the newly generated palettes
        mt_palettes = [self.find_palette_for_colors(colors).pid for colors in color_sets]
        self.bkg_color = list(color_sets[0])[0]
        self.__set_background_color(self.bkg_color)
        return mt_palettes

    def generate_nes(self):
        result = []
        for palette in self.palettes:
            colors = ['0x%02x' % color for color in palette.colors]
            while len(colors) < 4:
                colors.append('0x00')
            result.append(', '.join(colors))
        while len(result) < 4:
            result.append('0x%02x, 0x00, 0x00, 0x00' % self.bkg_color)
        return ',\n'.join(result)

    def find_palette_for_colors(self, colors):
        # sort colors so darker is earlier
        colors = sorted(list(set(colors)),
                        key=lambda x: 0 if x == 0x0F else (0xFF if x == 0x30 else x)) # make sure black and white are sorted appropriately

        for palette in self.palettes:
            if palette.match(colors):
                return palette

        palette = Palette(colors, len(self.palettes))
        self.palettes.append(palette)
        self.bkg_color = None # invalidate background color when we add a palette
        if len(self.palettes) > 4:
            print(self.palettes)
            raise Exception("Too many palettes in image!")
        return palette

    def __set_background_color(self, color):
        for palette in self.palettes:
            palette.set_bkg_color(color)

    def __find_background_color(self):
        colors = defaultdict(int)
        num_full_palettes = 0
        for palette in self.palettes:
            if len(palette.colors) == 4:
                for color in palette.colors:
                    colors[color] += 1
                num_full_palettes += 1
        background_options = [color for color, count in colors.items() if count == num_full_palettes]
        if len(background_options) == 0:
            raise Exception("Could not find a common background color")
        return background_options[0]

class Tileset:
    def __init__(self, filename):
        self.dir = os.path.dirname(os.path.realpath(filename))
        root = et.parse(filename).getroot()
        self.name = root.attrib['name']
        if int(root.attrib['tilewidth']) != 16 or int(root.attrib['tileheight']) != 16:
            raise Exception('Tile size MUST be 16x16')
        tilecount = int(root.attrib['tilecount'])
        if tilecount > 128:
            raise Exception('Tile count MUST be 128 tiles or less')
        self.tile_types = ['MT_TYPE_NOCOL'] * tilecount
        self.tile_defines = {}
        self.tile_graphics = {}
        for child in root:
            if child.tag == 'image':
                self.load_image(child.attrib['source'])
            elif child.tag == 'tile':
                properties = self.get_properties(child)
                tile_id = int(child.attrib['id'])
                if 'graphic' in properties:
                    self.tile_graphics[tile_id] = int(properties['graphic'])
                if 'define' in properties:
                    self.tile_defines[tile_id] = properties['define']
                if 'type' in child.attrib:
                    self.tile_types[tile_id] = 'MT_TYPE_' + child.attrib['type']
                if 'class' in child.attrib:
                    self.tile_types[tile_id] = 'MT_TYPE_' + child.attrib['class']

    def get_properties(self, child):
        properties = {}
        for el in child:
            if el.tag == 'properties':
                for prop in el:
                    value = prop.attrib['value']
                    value = True if value == 'true' else False if value == 'false' else value
                    properties[prop.attrib['name']] = value
        return properties

    def delete_unused_cells(self):
        for tile_id, graphic_id in self.tile_graphics.items():
            self.metatiles[tile_id] = self.metatiles[graphic_id]
        cell_indexes = set(i for sub in self.metatiles for i in sub)
        cells = dict(zip(cell_indexes, range(len(cell_indexes))))
        self.metatiles = [(cells[a], cells[b], cells[c], cells[d]) for a, b, c, d in self.metatiles]
        return cell_indexes

    def do_stuff(self):
        width, height = self.image.size
        cell_lookup, cells = image_to_cells(self.image)

        self.metatiles = cells_to_metatiles(cell_lookup, int(width / 8), int(height / 8))
        cell_indexes = self.delete_unused_cells()
        cells = [cell for i, cell in enumerate(cells) if i in cell_indexes]
        self.palettes = PaletteList()
        mt_palettes = self.palettes.generate_palettes_from_metatiles(self.metatiles, cells)
        self.tileset = create_tileset_from_cells(cells, self.palettes)
        colors = 4
        self.chr_data = toCHRArray((self.tileset.width, self.tileset.height,
                                    values_for_rgb(self.tileset, colors)), colors)
        self.metatile_attrs = zip(mt_palettes, self.tile_types)

    def load_image(self, image):
        try:
            img = Image.open(os.path.join(self.dir, image))
            self.image = img.convert('RGB')
        except Exception:
            exit('Failure attempting to open files')

    def save_chr(self, file):
        self.tileset.save(file)

    def write(self, header, compress):
        def write_array(array_name, array):
            f.write('const unsigned char %s_%s[] = {\n' % (array_name, self.name))
            f.write('\t' + '\n\t'.join(array.split('\n')))
            f.write('\n};\n\n')

        def write_chr(data):
            chr_chunks = chunks(map(lambda x: '0x{:02x}'.format(x), data), 16)
            return ',\n'.join([', '.join(chunk) for chunk in chr_chunks])

        def write_tile_defines():
            for tile_id, define in self.tile_defines.items():
                name = self.name.upper()
                define = define.upper()
                tl, tr, bl, br = self.metatiles[tile_id]
                f.write('#define %s_%s_TL 0x%02x\n' % (name, define, tl))
                f.write('#define %s_%s_TR 0x%02x\n' % (name, define, tr))
                f.write('#define %s_%s_BL 0x%02x\n' % (name, define, bl))
                f.write('#define %s_%s_BR 0x%02x\n' % (name, define, br))
                f.write('\n')

        bin_file = header.replace(".h", ".chr.bin")
        with open(bin_file, 'wb') as f:
            f.write(bytearray(self.chr_data))

        donut_file = header.replace(".h", ".donut");
        subprocess.run(["donut", bin_file, donut_file])
        subprocess.run(["bin2c", "-o", donut_file + ".h",
                        "-macro", # define size as a macro
                        "-name", "chr_"+self.name,
                        donut_file])

        with open(header.replace(".h", ".rle.h"), 'w') as f:
            if compress:
                self.chr_data = run_length_encode(self.chr_data)
            f.write('#define chr_%s_size %d\n' % (self.name, len(self.chr_data)))
            write_array('chr', write_chr(self.chr_data))

        with open(header.replace(".h", ".meta.h"), 'w') as f:
            metatile_hex = map(lambda x: '0x%02x, 0x%02x, 0x%02x, 0x%02x,' % x, self.metatiles)

            write_array('metatiles', '\n'.join(['%s // tile %d' % (x, i) for i, x in enumerate(metatile_hex)]))
            write_tile_defines()

        with open(header.replace(".h", ".pal.h"), 'w') as f:
            write_array('palette', self.palettes.generate_nes())

        with open(header.replace(".h", ".attr.h"), 'w') as f:
            metatile_attrs = list(map(lambda x: 'MT_PAL%d|%s' % x, self.metatile_attrs))
            # aligns to 128 bits
            #metatile_attrs += ['0'] * (128 - len(metatile_attrs))
            write_array('attrs', write_chunks(metatile_attrs, 8))

def convert_tileset_to_metatile(filename, header, chr, compress):
    tileset = Tileset(filename)
    tileset.do_stuff()
    if chr:
        tileset.save_chr(header.replace('.h', '.chr.png'))
    tileset.write(header, compress)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert tileset to metatile format')
    parser.add_argument('-c', '--chr', action='store_true', help='Output CHR image file')
    parser.add_argument('-r', '--rle', action='store_true', help='Output Run-Length-Encoded CHR data')
    parser.add_argument('tsx', type=str, nargs=1, help='The tileset file to convert')
    parser.add_argument('header', type=str, nargs=1, help='The output header file')
    args = parser.parse_args()

    convert_tileset_to_metatile(args.tsx[0], args.header[0], args.chr, args.rle)
