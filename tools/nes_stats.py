#!/usr/bin/env python

"""
Prints out useful stats about an NES file

Note: Free space is approximate based on a contiguous 8 bytes of zeros.
"""

import argparse
from util.rle import chunks

FILL_BYTE = 0
PRG_BANK_SIZE = 16*1024
CHR_BANK_SIZE = 8*1024
FREE_SPACE_CHUNKS = 8

def calc_free_space(bytes):
    """Approximates free space based on chunks that have zero bytes"""
    return sum([FREE_SPACE_CHUNKS for l in chunks(bytes, FREE_SPACE_CHUNKS) if sum(l) == 0])

class NES(object):

    prg_banks = []
    chr_banks = []

    def __init__(self, filename):
        with open(filename, 'rb') as f:
            self.read_header(f.read(16))

            for i in range(self.prg_size):
                bank = f.read(PRG_BANK_SIZE)
                empty = calc_free_space(bank)
                self.prg_banks.append(PRG_BANK_SIZE - empty)

            for i in range(self.chr_size):
                bank = f.read(CHR_BANK_SIZE)
                empty = calc_free_space(bank)
                self.chr_banks.append(CHR_BANK_SIZE - empty)

    def read_header(self, header):
        header = [b for b in header]
        nes_constant = [0x4E, 0x45, 0x53, 0x1A]
        if header[:4] != nes_constant:
            raise Exception("Invalid NES header")
        self.prg_size = header[4]
        self.chr_size = header[5]
        self.mirroring = 'four' if header[6] & 0x04 else 'vertical' if header[6] & 0x01 else 'horizontal'
        self.battery = header[6] & 2 == 2
        self.mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0)
        self.wram_size = header[8]
        self.ntsc = (header[9] & 1) == 0

    def print_banks(self, banks, size):
        size = float(size)
        print("BANK | USAGE")
        print("---- | -------------")
        for i, used in enumerate(banks):
            usage = "%d%% USED (%d of %d) [%d FREE]" % ((used / size) * 100, used, size, size-used) if used else "FREE"
            print("%4d | %s" % (i + 1, usage))

    def print_stats(self):
        print("Mapper: %d" % self.mapper)
        print("TV: %s" % 'NTSC' if self.ntsc else 'PAL')
        print("Mirroring: %s" % self.mirroring)
        if self.battery:
            print("Contains Battery")
        print("PRG ROM: %dK" % (self.prg_size * 16))
        print("CHR ROM: %dK" % (self.chr_size * 16))

        if self.prg_banks:
            print("")
            print("-- PRG BANKS --")
            self.print_banks(self.prg_banks, PRG_BANK_SIZE)

        if self.chr_banks:
            print("")
            print("-- CHR BANKS --")
            self.print_banks(self.chr_banks, CHR_BANK_SIZE)

def do_cli():
    parser = argparse.ArgumentParser(description='Print stats about NES file')
    parser.add_argument('nes', type=str, nargs=1, help='The NES file to read')
    args = parser.parse_args()

    nes = NES(args.nes[0])
    nes.print_stats()

if __name__ == "__main__":
    do_cli()
