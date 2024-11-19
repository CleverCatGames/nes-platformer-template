"""
Convert byte array to RLE array
"""

UPPER_LIMIT = 0xE0

def run_length_encode(byte_array, print_ratio=False):
    rle = []
    if byte_array:
        length = 0
        last = byte_array[0]

        for byte in byte_array:
            if byte == last and length < (0xFF-UPPER_LIMIT+1):
                length += 1
            else:
                rle.append((length, last))
                length = 1
            last = byte
        rle.append((length, last))

    bytes_out = []
    for length, byte in rle:
        if length == 1 and byte < UPPER_LIMIT:
            bytes_out.append(byte)
        else:
            bytes_out.append((UPPER_LIMIT-1) + length)
            bytes_out.append(byte)

    if print_ratio:
        ratio = len(bytes_out) / float(len(byte_array)) * 100
        print("Compression (%d to %d bytes) %d%% of original" % (len(byte_array), len(bytes_out), int(ratio)))
    return bytes_out

def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    l = list(l)
    for i in range(0, len(l), n):
        yield l[i:i+n]
