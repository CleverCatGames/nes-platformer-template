#!/usr/bin/env python3

"""
Compreses a byte list into RLE and bitmask values
"""

MAX_RUN_LENGTH = 0x1E
FLAG_SPECIAL = 0x80
FLAG_BITMAP = 0x40
FLAG_NUM = 0x20

STOP_BYTE = 0xFF

def compress_list(byte_list, print_ratio=False):
    compressed = list()
    i = 0
    while i < len(byte_list):
        # Check for RLE
        run_length = 1
        while i + run_length < len(byte_list) and byte_list[i + run_length] == byte_list[i]:
            run_length += 1

        run_length = min(run_length, MAX_RUN_LENGTH)

        if byte_list[i] == 0 and run_length > 2:
            compressed.append(FLAG_SPECIAL | run_length)  # RLE of zeros
            i += run_length
        elif run_length > 3:  # RLE is beneficial if over 3 of the same number
            compressed.append(FLAG_SPECIAL | FLAG_NUM | run_length)  # RLE of non-zero
            compressed.append(byte_list[i])
            i += run_length
        else:
            # Check for zeros
            zero_count = 0
            zero_bitmap = 0
            if i+6 < len(byte_list): # don't go out of bounds
                for j in range(6):
                    if byte_list[i+j] == 0:
                        zero_count += 1
                    else:
                        # store 1 when number is present
                        zero_bitmap |= (1 << (5 - j))

            if zero_count > 1:
                compressed.append(FLAG_SPECIAL | FLAG_BITMAP | zero_bitmap) # zero bitmap mask
                for j in range(6):
                    if byte_list[i] != 0:
                        compressed.append(byte_list[i])
                    i += 1
            else:
                # Regular value
                compressed.append(byte_list[i])
                i += 1

    compressed.append(STOP_BYTE) # final byte

    if print_ratio:
        ratio = len(compressed) / float(len(byte_list)) * 100
        print("Compression (%d to %d bytes) %d%% of original" % (len(byte_list), len(compressed), int(ratio)))

    return compressed

def decompress_list(compressed):
    decompressed = []
    i = 0
    while i < len(compressed):
        byte = compressed[i]
        if byte == 0xFF:
            break
        if byte & FLAG_SPECIAL: # special byte
            if byte & FLAG_BITMAP:  # Zero bitmap
                zero_bitmap = byte & 0x3F
                for bit in range(6):
                    if zero_bitmap & (1 << (5 - bit)):
                        i += 1
                        byte = compressed[i]
                    else:
                        byte = 0
                    decompressed.append(byte)
            else:  # RLE
                run_length = (byte & 0x1F)
                if byte & FLAG_NUM: # RLE of zeroes
                    i += 1 # get next byte
                    byte = compressed[i]
                else:  # RLE of non-zero
                    byte = 0
                decompressed.extend([byte] * run_length)
        else:
            decompressed.append(byte)
        i += 1
    return decompressed

if __name__ == '__main__':
    # Example usage
    original_data = [
        0x0b, 0x0b, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32,
        0x09, 0x09, 0x0a, 0x0a,
        0x0a, 0x0a, 0x0a, 0x0a,
        0x0a, 0x0d, 0x0a, 0x12,
        0x0a, 0x12, 0x0a, 0x12,
        0x0b, 0x13, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x09, 0x11, 0x0b, 0x13,
        0x0a, 0x0a, 0x0a, 0x0d,
        0x0a, 0x0c, 0x0a, 0x0a,
        0x0a, 0x0a, 0x0b, 0x0b,
        0x09, 0x11, 0x0a, 0x12,
        0x11, 0x32, 0x12, 0x32,
        0x12, 0x32, 0x12, 0x32,
        0x0c, 0x09, 0x0a, 0x0a,
        0x0b, 0x0b, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x53,
        0x00, 0x54, 0x00, 0x55,
        0x00, 0x56, 0x00, 0x53,
        0x48, 0x56, 0x40, 0x00,
        0x49, 0x00, 0x00, 0x00,
    ]

    compressed = compress_list(original_data)
    decompressed = decompress_list(compressed)

    #print(f"Data comparison:\n{original_data}\n{decompressed}")
    print(f"Compressed data: {list(compressed)}")
    print(f"Original size: {len(original_data)} bytes")
    print(f"Compressed size: {len(compressed)} bytes")
    print(f"Compression ratio: {len(compressed) / len(original_data)*100:.2f}%")
    print(f"Decompressed matches original: {original_data == decompressed}")

