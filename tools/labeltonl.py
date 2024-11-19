#!/usr/bin/env python3

"""
Converts .lbl file to be useable by FCEUX
"""

import sys
import os
assert sys.version_info[0] >= 3, "Python 3 required."

from collections import OrderedDict

def label_to_nl(labels, nl_file, range_min, range_max):
    labs = {}
    sout = ""
    for line in labels:
        words = line.split()
        if (words[0] == "al"):
            adr = int(words[1], 16)
            sym = words[2]
            sym = sym.lstrip('.')
            if (sym[0] == '_' and sym[1] == '_'):
                continue # skip compiler internals
            if (adr >= range_min and adr <= range_max):
                if (adr in labs):
                    # multiple symbol
                    text = labs[adr]
                    textsplit = text.split()
                    if (sym not in textsplit):
                        text = text + " " + sym
                        labs[adr] = text
                else:
                    labs[adr] = sym
    for (adr, sym) in labs.items():
        sout += ("$%04X#%s#\n" % (adr, sym))
    with open(nl_file, "wt") as f:
        f.write(sout)
    print("debug symbols: " + nl_file)

if __name__ == "__main__":
    label_file = sys.argv[1]
    name, ext = os.path.splitext(label_file)
    nes_file = name + '.nes'
    with open(label_file, "rt") as of:
        labels = of.readlines()
    label_to_nl(labels, "{0}.ram.nl".format(nes_file), 0x0200, 0x7FF)
    label_to_nl(labels, "{0}.0.nl".format(nes_file), 0x8000, 0xBFFF)
    label_to_nl(labels, "{0}.1.nl".format(nes_file), 0x8000, 0xBFFF)
    label_to_nl(labels, "{0}.2.nl".format(nes_file), 0x8000, 0xBFFF)
    label_to_nl(labels, "{0}.3.nl".format(nes_file), 0x8000, 0xBFFF)
    label_to_nl(labels, "{0}.4.nl".format(nes_file), 0x8000, 0xBFFF)
    label_to_nl(labels, "{0}.5.nl".format(nes_file), 0x8000, 0xBFFF)
    label_to_nl(labels, "{0}.6.nl".format(nes_file), 0x8000, 0xBFFF)
    label_to_nl(labels, "{0}.7.nl".format(nes_file), 0xC000, 0xFFFF)
