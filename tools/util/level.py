import os
from . import tmx
from datetime import datetime
from math import ceil, floor
from collections import defaultdict
from .compress import compress_list
from .rle import run_length_encode, chunks

# number of tiles horizontally in a page
PAGE_HORIZ_TILES = 16

def to_hex(i):
    return '0x%02x' % i

def transpose_tiles(original_tiles, size):
    w, h = size
    tiles = [0] * len(original_tiles)
    for i, tile in enumerate(original_tiles):
        x = i % w
        y = floor(i / w)
        index = int(x * h + y)  # transpose index so we encode in vertical strips
        tiles[index] = 0 if tile < 0 else tile  # only positive tiles
    return tiles

def to_megatiles(tiles, size):
    w, h = size
    if w % 2 == 1 or h % 2 == 1:
        raise Exception("Level width and height must be divisible by 2: {0}, {1}".format(w, h))
    meta_blocks = []
    indexes = []
    for y in range(0, h, 2):
        for x in range(0, w, 2):
            tl = tiles[y * w + x]
            tr = tiles[(y+1) * w + x]
            bl = tiles[y * w + (x+1)]
            br = tiles[(y+1) * w + (x+1)]
            block = (tl, tr, bl, br)
            try:
                indexes.append(meta_blocks.index(block))
            except ValueError:
                indexes.append(len(meta_blocks))
                meta_blocks.append(block)
    num_blocks = len(meta_blocks)
    if num_blocks > 255:
        raise Exception("Level has generated too many meta tiles: " + str(num_blocks))
    return meta_blocks, indexes

def convert_map_layer(tiles, size, rle=False):
    if rle:
        bytes = run_length_encode(transpose_tiles(tiles, size))
        return len(bytes), bytes_to_hex(bytes)
    else:
        w, h = size
        if h % 2 == 1:
            print("--TRUNCATING TOP LINE--")
            h -= 1
            size = (w, h)
            tiles = tiles[w:]
        blocks, indexes = to_megatiles(tiles, size)
        block_bytes = []
        for block in blocks:
            tl, tr, bl, br = block
            if True:
                block_bytes.append(tl)
                block_bytes.append(tr)
                block_bytes.append(bl)
                block_bytes.append(br)
            else:  # packed
                val = (tl << 18) | (tr << 12) | (bl << 6) | br
                block_bytes.append((val >> 16) & 0xFF)
                block_bytes.append((val >> 8) & 0xFF)
                block_bytes.append(val & 0xFF)
        block_bytes = compress_list(block_bytes, print_ratio=True)
        num_bytes = len(block_bytes) + len(indexes)
        indexes = transpose_tiles(indexes, (w/2, h/2))
        lines = ['// {0} MEGAtiles - 2x2 metatiles or 32x32 pixels'.format(len(blocks))]
        lines += ['//tl, tr  , bl  , br']
        lines += bytes_to_hex(block_bytes, 4)
        return num_bytes, lines, bytes_to_hex(indexes), len(blocks)

def bytes_to_hex(bytes, width=16):
    return [', '.join(l) + ',' for l in list(chunks(map(to_hex, bytes), width))]

# Converts object dictionary to enemy/object data
def convert_objects(objects):
    # sort horizontally
    def sort_horiz(o):
        p, x, _, _, _ = o
        return p * PAGE_HORIZ_TILES + x
    last_page = 0
    num_bytes = 0
    final = []
    for (page, x, y, gid, flags) in sorted(objects, key=sort_horiz):
        if page - last_page > 1:
            raise Exception("Object page %s is empty!" % (last_page+1))
        out = ['OBJ_POS({0:2}, {1:2})'.format(x, y)]
        tile = ['{0:3}'.format(gid)]
        if page > last_page:
            tile.append('OBJ_NEW_PAGE')
        if flags:
            tile.append('OBJ_EXTRA_DATA')
            out.append('|'.join(flags))
        out.insert(1, '|'.join(tile))
        final.append(', '.join(out) + ',')
        num_bytes += len(out)
        last_page = page
    # add final byte for the object end byte
    return num_bytes+1, final

def find_tileset_name(gid, tilesets):
    found = None
    for ts in tilesets:
        if ts.firstgid > gid:
            break

        found = ts
    return found

def get_map_offsets(m):
    mapx = m.properties.get("mapx", 0)
    mapy = m.properties.get("mapy", 0)
    pages = int(m.width / PAGE_HORIZ_TILES)
    offsets = [(mapx+page, mapy) for page in range(pages)]
    for layer in m.layers:
        if layer.name == "map":
            for obj in layer.objects:
                start = int(obj.x / 256)
                end = start + int(obj.width / 256)
                x = obj.properties.get("mapx", mapx)
                y = obj.properties.get("mapy", mapy)
                for i in range(start, end):
                    offsets[i] = (x, y)
                    x += 1

    for i,offset in enumerate(offsets):
        x, y = offset
        offsets[i] = 'MAPXY({}, {})'.format(x, y)
    return offsets

def range_convert(r, current_page, max_page):
    return eval(r.replace("S", "0").replace("C", f"{current_page}").replace("E", f"{max_page}"))

def get_objects_from_layer(m, layer, objects, max_page):
    for obj in layer.objects:
        # ignore text objects (used for annotations)
        if obj.text:
            continue
        x = int(obj.x / m.tilewidth)
        page = int(x / PAGE_HORIZ_TILES)
        obj_x = x % PAGE_HORIZ_TILES
        obj_y = int(obj.y / m.tileheight) - 1 # for some reason objects are off by one
        flags = []

        if obj.fliph:
            flags.append("OAM_FLIP_H")

        if obj.flipv:
            flags.append("OAM_FLIP_V")

        # this is a fake name to catch errors
        name = "INVALID_OBJ_{}".format(obj.id)

        # check if this is a level_zone object
        ts = find_tileset_name(obj.gid, m.tilesets)
        if ts:
            tile = ts.tiles[obj.gid - ts.firstgid]
            if 'name' in tile.properties:
                tile_name = tile.properties['name']
                if tile_name == "level_zone":
                    flags.append("LVL_{0}".format(str(obj.name).upper()))

        for key, value in obj.properties.items():
            key = key.lower()
            if key == "alternate":
                if value:
                    flags.append("LVL_ALTERNATE")
            elif key == "behind":
                if value:
                    flags.append("OAM_BEHIND")
            elif key == "name":
                name = 'LEVEL_OBJ_{}'.format(str(value).upper())
            elif key == "range":
                rmin, rmax = value.split(",")
                rmin = range_convert(rmin, page, max_page)
                rmax = range_convert(rmax, page, max_page)
                flags.append("OBJ_RANGE({}, {})".format(rmin, rmax))
            elif isinstance(value, int):
                print("Converting {} to int".format(key))
                flags.append(to_hex(value))

        objects.append((page, obj_x, obj_y, name, flags))

def get_tiles_from_layer(m, layer, objects, tilesets):
    for i, tile in enumerate(layer.tiles):
        ts = find_tileset_name(tile.gid, m.tilesets)
        if ts is None:
            continue
        gid = tile.gid - ts.firstgid
        if ts.name == "objects":
            x = i % m.width
            page = int(x / PAGE_HORIZ_TILES)
            obj_x = x % PAGE_HORIZ_TILES
            obj_y = int(i / m.width)
            objects.append((page, obj_x, obj_y, gid, None))
        else:
            tilesets[ts.name][i] = gid  # normalize tile offset

def get_level_props(m):
    flags = []
    for key, value in m.properties.items():
        name = key.lower()
        if name == 'exit' and value:
            flags.append('LEVEL_FLAGS_EXIT|LEVEL_FLAGS_EXIT_{0}'.format(value.upper()))
        elif name == 'pagelock' and value:
            flags.append('LEVEL_FLAGS_PAGE_LOCK')
        elif name == 'dark' and value:
            flags.append('LEVEL_FLAGS_PAL_DARK')
        elif name == 'liquid':
            flags.append('LIQUID_{}'.format(value.upper()))
    return '|'.join(flags) if flags else 'LEVEL_FLAGS_NONE'

def convert(mapfile, asmfile):
    m, map_name = load_map(mapfile)

    size = (m.width, m.height)

    pages = int(m.width / PAGE_HORIZ_TILES)

    num_bytes = 3
    objects, tilesets = extract_objects_and_tiles(m)

    offsets = get_map_offsets(m)

    byte_count, objects = convert_objects(objects)
    num_bytes += byte_count

    tiles, tileset_name = background_tileset(tilesets)
    tile_bytes, map_info, map_indexes, num_megatiles = convert_map_layer(tiles, size)
    num_bytes += tile_bytes + len(offsets)
    print('{} Pages={} Megatiles={} Objects={} Bytes={}'.format(
        map_name.ljust(16),
        str(pages).ljust(2),
        str(num_megatiles).ljust(3),
        str(len(objects)).ljust(2),
        num_bytes
    ))

    flags = get_level_props(m)

    with open(asmfile, 'w') as f:
        f.write("""/*
 * Tilemap data include for {map}
 * Autogenerated on {today}
 * Zone: {tileset}
 * Bank: {bank}
 * Bytes: {bytes}
 * Width: {pages} pages
 * MEGAtiles: {num_megatiles} (32x32 pixels or 2x2 metatiles)
 */

#include "level.h"

#pragma rodata-name(push, "{bank}")
const unsigned char {name}_map_info[] = {{
    {pages}, // num pages wide
    {tileset}, // zone tileset
    {level_flags}, // flags
    // offsets
    {offsets},
    {map_info}
}};

// map data containing indexes to MEGAtile data
const unsigned char {name}_map_data[] = {{
    {map_data}
    0xff // end of level data
}};

// object data for level (2-3 bytes per object)
// xxxx yyyy    x = page x pos
//              y = page y pos
// pdtt tttt    p = new page
//              d = has flag data
//              t = object type (0-63)
// ffff ffff    f = flag data
const unsigned char {name}_obj[] = {{
    {obj_data}
    0xff // end of object data
}};
#pragma rodata-name(pop)

""".format(
            today = datetime.today().strftime("%Y-%m-%d %I:%M:%S %p"),
            bank=tileset_name.upper(),
            tileset='ZONE_{}'.format(tileset_name.upper()),
            num_megatiles=num_megatiles,
            map=mapfile,
            offsets=',\n\t'.join(map(str, offsets)),
            bytes=num_bytes,
            name=map_name,
            pages=pages-1,
            level_flags=flags,
            map_info='\n\t'.join(map_info),
            map_data='\n\t'.join(map_indexes),
            obj_data='\n\t'.join(objects)))
    return tileset_name, get_tileset_props(m, tileset_name)

def background_tileset(tilesets):
    tileset_names = list(tilesets.keys())
    if len(tileset_names) > 1:
        raise Exception("Should not have multiple backgrounds!")

    background_tileset = tileset_names[0]
    return tilesets[background_tileset], background_tileset

def extract_objects_and_tiles(m):
    num_tiles = m.width * m.height
    tilesets = defaultdict(lambda: [-1] * num_tiles)
    max_page = floor(m.width / 16) - 1

    # group tiles by tileset
    objects = []
    for layer in m.layers:
        if isinstance(layer, tmx.ObjectGroup):
            if layer.name.startswith("object"):
                get_objects_from_layer(m, layer, objects, max_page)
        elif layer.name != "grid":
            get_tiles_from_layer(m, layer, objects, tilesets)
    return objects, tilesets

def get_tileset_props(m, background_tileset):
    for tileset in m.tilesets:
        if tileset.name == background_tileset:
            return tileset.properties
    return dict()

def load_map(mapfile):
    filename, extension = os.path.splitext(mapfile)
    if extension != '.tmx':
        raise Exception("Invalid input map")
    map_name = os.path.basename(filename)
    m = tmx.TileMap.load(mapfile)
    return m, map_name
