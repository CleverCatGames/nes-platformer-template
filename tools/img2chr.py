#!/usr/bin/env python

"""
Converts a grayscale image to 2bpp format

This script outputs two files. One for tall sprites (8x16) and another for square (8x8).

Both of these files get ran through donut compression but you could potentially use this output for building CHR ROM.

Gray values are approximate so you don't need a specific palette. However, it might be easiest to copy one of the existing files in the `assets/gfx` folder.
"""

import os
import argparse
import subprocess
from PIL import Image
from util.rle import run_length_encode, chunks

def bitplane(x, y, chr_data, output, plane):
    for j in range(8):
        c = 0
        for i in range(8):
            c = (c * 2) | ((chr_data[y + j][x + i] >> plane) & 1)
        output.append(c)

def toCHRArray(data, colors, tall=False):
    if (colors & (colors - 1)) != 0:
        error("colors value must be a power of 2")
    output = []
    w, h, chr_data = data
    if tall and h % 16 != 0:
        return False

    def write_bits(x, y):
        c = int(colors / 2)
        plane = 0
        while c > 0:
            bitplane(x, y, chr_data, output, plane)
            plane += 1
            c = int(c / 2)

    for cell_y in range(0, h, 16 if tall else 8):
        for cell_x in range(0, w, 8):
            write_bits(cell_x, cell_y)
            if tall:
                write_bits(cell_x, cell_y + 8)

    return output

def values_for_rgb(image, colors):
    import colorsys

    if colors == 8:
        bpp = (2, 50, 100, 150, 200, 230, 245, 256)
    elif colors == 4:
        bpp = (2, 150, 230, 256)
    elif colors == 2:
        bpp = (128, 256)
    else:
        error("Number of colors needs to be provided")

    values = []
    width, height = image.size
    for pixel_y in range(height):
        row = []
        for pixel_x in range(width):
            red, green, blue = image.getpixel((pixel_x, pixel_y))
            _, _, value = colorsys.rgb_to_hsv(red, green, blue)
            for i in range(len(bpp)):
                if value < bpp[i]:
                    row.append(i)
                    break
        values.append(row)
    return values

def convert_image_to_chr_data(filename, colors):
    try:
        img = Image.open(filename)
        rgb_img = img.convert('RGB')
    except Exception:
        exit('Failure attempting to open files')

    width, height = rgb_img.size
    values = values_for_rgb(rgb_img, colors)
    return (width, height, values)

def name_from_filename(filename):
    return os.path.basename(filename).split(".")[0]

def write_include(filename, chr_data, rle, colors):
    short_data = toCHRArray(chr_data, colors)
    tall_data = toCHRArray(chr_data, colors, tall=True) if chr_data[1] % 16 == 0 else None

    rle_used = False
    if rle:
        compressed_short_data = run_length_encode(short_data)
        if len(compressed_short_data) < len(short_data):
            rle_used = True
            short_data = compressed_short_data
            if tall_data:
                tall_data = run_length_encode(tall_data)

    def write_chunk(data):
        chr_chunks = chunks(map(lambda x: '0x{:02x}'.format(x), data), 8)
        return ',\n'.join([', '.join(chunk) for chunk in chr_chunks])

    with open(filename, 'w') as out:
        name = name_from_filename(filename)
        out.write('const unsigned char chr_%s[] = {\n' % name)
        if tall_data:
            out.write('#if defined(TALL_SPRITES)\n')
            out.write(write_chunk(tall_data))
            out.write('\n#define chr_%s_size %d\n' % (name, len(tall_data)))
            out.write('\n#else\n')
            out.write(write_chunk(short_data))
            out.write('\n#define chr_%s_size %d\n' % (name, len(short_data)))
            out.write('\n#endif')
        else:
            out.write('#if defined(TALL_SPRITES)\n#error "Tall sprites not generated for %s.h"\n#endif\n' % name)
            out.write(write_chunk(short_data))
            out.write('\n#define chr_%s_size %d\n' % (name, len(short_data)))
        out.write('\n};\n')
        out.write('#define load_chr_{name}() {func}((u8*)chr_{name}, chr_{name}_size)\n'.format(
            name=name,
            func='vram_unrle' if rle_used else 'vram_write'))


def donut_file(bin_file):
    donut_file = bin_file.replace(".bin", ".donut")
    name = name_from_filename(bin_file)
    subprocess.run(["donut", bin_file, donut_file])
    subprocess.run(["bin2c", "-o", donut_file + ".h",
                    "-macro", # define size as a macro
                    "-name", "chr_"+name,
                    donut_file])


def write_bin_file(header, chr_data, colors, tall=False, donut=True):
    ext = ".tall.bin" if tall else ".bin"
    bin_file = header.replace(".h", ext)
    with open(bin_file, 'wb') as out:
        data = toCHRArray(chr_data, colors, tall)
        out.write(bytearray(data))
    if donut:
        donut_file(bin_file)


def write_chr_data(chr_data, header, rle, colors):
    #write_include(header, chr_data, rle, colors)
    write_bin_file(header, chr_data, colors, tall=False)
    write_bin_file(header, chr_data, colors, tall=True)

def do_cli():
    parser = argparse.ArgumentParser(description='Convert PNG image to CHR format')
    parser.add_argument('-r', '--rle', action='store_true', help='Output using Run-Length-Encoding')
    parser.add_argument('image', type=str, nargs=1, help='The image file to convert')
    parser.add_argument('chr', type=str, nargs=1, help='The output CHR file')
    parser.add_argument('-c', '--colors', action='store_true', default=4, help='Number of colors')
    args = parser.parse_args()

    chr_data = convert_image_to_chr_data(args.image[0], args.colors)
    write_chr_data(chr_data, args.chr[0], args.rle, args.colors)

if __name__ == "__main__":
    do_cli()
