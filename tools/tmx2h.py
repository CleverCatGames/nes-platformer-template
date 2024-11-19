#!/usr/bin/env python

from util.level import convert

parser = argparse.ArgumentParser(description='Convert TMX map to C header')
parser.add_argument('map', type=str, nargs=1, help='The map file to convert')
parser.add_argument('asm', type=str, nargs=1, help='The output header file')
args = parser.parse_args()
convert(args.map[0], args.asm[0])
