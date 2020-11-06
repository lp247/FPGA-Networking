from typing import Callable
import zlib

# Ethernet crc32 magic number:       0xC704DD7B (11000111000001001101110101111011)
# CRC32 divider polynomial (normal): 0x04C11DB7 (00000100110000010001110110110111)

# XOR properties: A^A = 0 | A^0 = A | A^B = B^A | A^B=C => A^C=B

#         0^X=Y 1^X=Y 2^X=Y 3^X=Y 4^X=Y 5^X=Y 6^X=Y 7^X=Y 8^X=Y 9^X=Y a^X=Y b^X=Y c^X=Y d^X=Y e^X=Y f^X=Y
#        ------------------------------------------------------------------------------------------------
# A^0=B |   0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
# A^1=B |   1     0   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
# A^2=B |   2   1^2=3   0   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
# A^3=B |   3   ----- -----   0   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
# A^4=B |   4   1^4=5 2^4=6 3^4=7   0   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
# A^5=B |   5   ----- 2^5=7 3^5=6 -----   0   ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
# A^6=B |   6   1^6=7 ----- ----- ----- -----   0   ----- ----- ----- ----- ----- ----- ----- ----- -----
# A^7=B |   7   ----- ----- ----- ----- ----- -----   0   ----- ----- ----- ----- ----- ----- ----- -----
# A^8=B |   8   1^8=9 2^8=a 3^8=b 4^8=c 5^8=d 6^8=e 7^8=f   0   ----- ----- ----- ----- ----- ----- -----
# A^9=B |   9   ----- 2^9=b 3^9=a 4^9=d 5^9=c 6^9=f 7^9=e -----   0   ----- ----- ----- ----- ----- -----
# A^a=B |   a   1^a=b ----- ----- 4^a=e 5^a=f 6^a=c 7^a=d ----- -----   0   ----- ----- ----- ----- -----
# A^b=B |   b   ----- ----- ----- 4^b=f 5^b=e 6^b=d 7^b=c ----- ----- -----   0   ----- ----- ----- -----
# A^c=B |   c   1^c=d 2^c=e 3^c=f ----- ----- ----- ----- ----- ----- ----- -----   0   ----- ----- -----
# A^d=B |   d   ----- 2^d=f 3^d=e ----- ----- ----- ----- ----- ----- ----- ----- -----   0   ----- -----
# A^e=B |   e   1^e=f ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----   0   -----
# A^f=B |   f   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----   0

# ==============================================================================
#     CRC Register:
# ==============================================================================

# Ri' = Ri-1 ^ (Pi and R31)
# R0' = NIN ^ (P0 and R31)

#           28   24   20   16   12    8    4    0
# Pi = (1_0000_0100_1100_0001_0001_1101_1011_0111)

# R31' = R30       | R23' = R22 ^ R31 | R15' = R14       | R07' = R06 ^ R31 |
# R30' = R29       | R22' = R21 ^ R31 | R14' = R13       | R06' = R05       |
# R29' = R28       | R21' = R20       | R13' = R12       | R05' = R04 ^ R31 |
# R28' = R27       | R20' = R19       | R12' = R11 ^ R31 | R04' = R03 ^ R31 |
# R27' = R26       | R19' = R18       | R11' = R10 ^ R31 | R03' = R02       |
# R26' = R25 ^ R31 | R18' = R17       | R10' = R09 ^ R31 | R02' = R01 ^ R31 |
# R25' = R24       | R17' = R16       | R09' = R08       | R01' = R00 ^ R31 |
# R24' = R23 ^ R31 | R16' = R15 ^ R31 | R08' = R07 ^ R31 | R00' = NIN ^ R31 |

# R31 R30 R29 R28 R27 R26 R25 R24 R23 R22 R21 R20 R19 R18 R17 R16 R15 R14 R13 R12 R11 R10 R09 R08 R07 R06 R05 R04 R03 R02 R01 R00 | NIN | N
# =============================================================================================================================== | === | =
# C31 C30 C29 C28 C27 C26 C25 C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 | B07 | 0
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C30 C29 C28 C27 C26 C25 C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 | B06 | 1
#                     C31     C31 C31 C31                     C31             C31 C31 C31     C31 C31     C31 C31     C31 C31 C31 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C29 C28 C27 C26 C25 C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 B06 | B05 | 2
#                 C31     C31 C31 C31                     C31             C31 C31 C31     C31 C31     C31 C31     C31 C31 C31     |     |
#                     C30     C30 C30 C30                     C30             C30 C30 C30     C30 C30     C30 C30     C30 C30 C30 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C28 C27 C26 C25 C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 B06 B05 | B04 | 3
#             C31     C31 C31 C31                     C31             C31 C31 C31     C31 C31     C31 C31     C31 C31 C31         |     |
#                 C30     C30 C30 C30                     C30             C30 C30 C30     C30 C30     C30 C30     C30 C30 C30     |     |
#                     C29     C29 C29 C29                     C29             C29 C29 C29     C29 C29     C29 C29     C29 C29 C29 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C27 C26 C25 C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 B06 B05 B04 | B03 | 4
#         C31     C31 C31 C31                     C31             C31 C31 C31     C31 C31     C31 C31     C31 C31 C31             |     |
#             C30     C30 C30 C30                     C30             C30 C30 C30     C30 C30     C30 C30     C30 C30 C30         |     |
#                 C29     C29 C29 C29                     C29             C29 C29 C29     C29 C29     C29 C29     C29 C29 C29     |     |
#                     C28     C28 C28 C28                     C28             C28 C28 C28     C28 C28     C28 C28     C28 C28 C28 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C26 C25 C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 B06 B05 B04 B03 | B02 | 5
#     C31     C31 C31 C31                     C31             C31 C31 C31     C31 C31     C31 C31     C31 C31 C31                 |     |
#         C30     C30 C30 C30                     C30             C30 C30 C30     C30 C30     C30 C30     C30 C30 C30             |     |
#             C29     C29 C29 C29                     C29             C29 C29 C29     C29 C29     C29 C29     C29 C29 C29         |     |
#                 C28     C28 C28 C28                     C28             C28 C28 C28     C28 C28     C28 C28     C28 C28 C28     |     |
#                     C27     C27 C27 C27                     C27             C27 C27 C27     C27 C27     C27 C27     C27 C27 C27 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C25 C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 B06 B05 B04 B03 B02 | B01 | 6
# C31     C31 C31 C31                     C31             C31 C31 C31     C31 C31     C31 C31     C31 C31 C31                     |     |
#     C30     C30 C30 C30                     C30             C30 C30 C30     C30 C30     C30 C30     C30 C30 C30                 |     |
#         C29     C29 C29 C29                     C29             C29 C29 C29     C29 C29     C29 C29     C29 C29 C29             |     |
#             C28     C28 C28 C28                     C28             C28 C28 C28     C28 C28     C28 C28     C28 C28 C28         |     |
#                 C27     C27 C27 C27                     C27             C27 C27 C27     C27 C27     C27 C27     C27 C27 C27     |     |
#                     C26     C26 C26 C26                     C26             C26 C26 C26     C26 C26     C26 C26     C26 C26 C26 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C24 C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 B06 B05 B04 B03 B02 B01 | B00 | 7
#     C31 C31 C31     C31     C31 C31                 C31 C31         C31 C31 C31                     C31 C31 C31     C31 C31 C31 |     |
# C30     C30 C30 C30                     C30             C30 C30 C30     C30 C30     C30 C30     C30 C30 C30                     |     |
#     C29     C29 C29 C29                     C29             C29 C29 C29     C29 C29     C29 C29     C29 C29 C29                 |     |
#         C28     C28 C28 C28                     C28             C28 C28 C28     C28 C28     C28 C28     C28 C28 C28             |     |
#             C27     C27 C27 C27                     C27             C27 C27 C27     C27 C27     C27 C27     C27 C27 C27         |     |
#                 C26     C26 C26 C26                     C26             C26 C26 C26     C26 C26     C26 C26     C26 C26 C26     |     |
#                     C25     C25 C25 C25                     C25             C25 C25 C25     C25 C25     C25 C25     C25 C25 C25 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -
# C23 C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12 C11 C10 C09 C08 C07 C06 C05 C04 C03 C02 C01 C00 B07 B06 B05 B04 B03 B02 B01 B00 |     | 8
# C31 C31 C31     C31     C31 C31                 C31 C31         C31 C31 C31                     C31 C31 C31     C31 C31 C31     |     |
#     C30 C30 C30     C30     C30 C30                 C30 C30         C30 C30 C30                     C30 C30 C30     C30 C30 C30 |     |
# C29     C29 C29 C29                     C29             C29 C29 C29     C29 C29     C29 C29     C29 C29 C29                     |     |
#     C28     C28 C28 C28                     C28             C28 C28 C28     C28 C28     C28 C28     C28 C28 C28                 |     |
#         C27     C27 C27 C27                     C27             C27 C27 C27     C27 C27     C27 C27     C27 C27 C27             |     |
#             C26     C26 C26 C26                     C26             C26 C26 C26     C26 C26     C26 C26     C26 C26 C26         |     |
#                 C25     C25 C25 C25                     C25             C25 C25 C25     C25 C25     C25 C25     C25 C25 C25     |     |
#                     C24     C24 C24 C24                     C24             C24 C24 C24     C24 C24     C24 C24     C24 C24 C24 |     |
# ------------------------------------------------------------------------------------------------------------------------------- | --- | -

from typing import List


def print_hex_dec(number: int):
    print(hex(number), number)


def rev_word_bits(word: int, wordsize: int = None) -> int:
    if word.bit_length() == 0:
        return word
    size: int = wordsize if wordsize != None else word.bit_length()
    rev: int = 0
    for i in range(0, size):
        rev = (rev << 1) ^ ((word >> i) & 1)
    return rev


def rev_word_byte_bits(word: int) -> int:
    if word.bit_length() == 0:
        return word
    num_bytes: int = ((word.bit_length() - 1) // 8) + 1
    rev: int = 0
    for i in range(0, num_bytes):
        rev = (rev << 8) ^ rev_word_bits(
            ((word >> ((num_bytes - 1 - i) * 8)) & 0xFF), 8)
    return rev


def rev_word_bytes(word: int, wordsize: int = None) -> int:
    if word.bit_length() == 0:
        return word
    size: int = wordsize if wordsize != None else word.bit_length()
    num_bytes: int = ((size - 1) // 8) + 1
    rev: int = 0
    for i in range(0, num_bytes):
        rev = (rev << 8) ^ ((word >> (i * 8)) & 0xFF)
    return rev


def invert_bits(word: int, wordsize: int = None) -> int:
    if word.bit_length() == 0:
        return word
    size: int = wordsize if wordsize != None else word.bit_length()
    return (1 << size) - 1 - word


def hex_string_to_int_list(hex: str) -> List[int]:
    return [x for x in bytes.fromhex(hex)]


def int_to_bool_list(num: int, num_digits: int) -> List[bool]:
    bin_str: str = format(num, '0' + str(num_digits) + 'b')[::-1]
    return [x == '1' for x in bin_str]


def bool_list_to_int(lst: List[bool]) -> int:
    return int("".join(map(lambda b: str(int(b)), lst[::-1])), 2)


def inv_crc32_decorator(data: List[int], func: Callable[[List[int]], int]) -> int:
    mod_data: List[int] = data.copy()
    for i in range(min(len(mod_data), 4)):
        mod_data[i] = invert_bits(mod_data[i], 8)
    crc = invert_bits(func(mod_data), 32)
    return crc


def rev_crc32_decorator(data: List[int], func: Callable[[List[int]], int]) -> int:
    mod_data: List[int] = [rev_word_bits(x, 8) for x in data]
    crc = rev_word_bits(func(mod_data), 32)
    return crc


def bytewise_bit_rev_crc32_decorator(data: List[int], func: Callable[[List[int]], int]) -> int:
    crc = rev_word_bytes(func(data), 32)
    return crc


# SERIAL CRC32

def calc_vanilla_serial_crc32(data: List[int]) -> int:
    poly = 0x104C11DB7
    crc = 0
    padded_data = data + [0xFF if len(data) < 4 else 0, 0xFF if len(
        data) < 3 else 0, 0xFF if len(data) < 2 else 0, 0xFF if len(data) < 1 else 0]
    for num in padded_data:
        for bit in range(8):
            crc = crc << 1
            new_bit = (num >> (7 - bit)) & 1
            crc = crc | new_bit
            if (crc >> 32) == 1:
                crc = crc ^ poly
    return crc


def calc_inv_serial_crc32(data: List[int]) -> int:
    return inv_crc32_decorator(data, calc_vanilla_serial_crc32)


def calc_bit_rev_inv_serial_crc32(data: List[int]) -> int:
    return rev_crc32_decorator(data, calc_inv_serial_crc32)


def calc_bytewise_bit_rev_inv_serial_crc32(data: List[int]) -> int:
    return bytewise_bit_rev_crc32_decorator(data, calc_bit_rev_inv_serial_crc32)


# PARALLEL CRC32

def calc_vanilla_parallel_crc32_step(data: int, prev: int) -> int:
    d: List[bool] = int_to_bool_list(data, 8)
    c: List[bool] = int_to_bool_list(prev, 32)
    next_crc: List[bool] = []
    next_crc.append(d[6] ^ d[0] ^ c[24] ^ c[30])
    next_crc.append(d[7] ^ d[6] ^ d[1] ^ d[0] ^ c[24] ^ c[25] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[6] ^ d[2] ^ d[1] ^ d[0] ^
                    c[24] ^ c[25] ^ c[26] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[3] ^ d[2] ^ d[1] ^ c[25] ^ c[26] ^ c[27] ^ c[31])
    next_crc.append(d[6] ^ d[4] ^ d[3] ^ d[2] ^ d[0] ^
                    c[24] ^ c[26] ^ c[27] ^ c[28] ^ c[30])
    next_crc.append(d[7] ^ d[6] ^ d[5] ^ d[4] ^ d[3] ^ d[1] ^ d[0]
                    ^ c[24] ^ c[25] ^ c[27] ^ c[28] ^ c[29] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[6] ^ d[5] ^ d[4] ^ d[2] ^ d[1] ^
                    c[25] ^ c[26] ^ c[28] ^ c[29] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[5] ^ d[3] ^ d[2] ^ d[0] ^
                    c[24] ^ c[26] ^ c[27] ^ c[29] ^ c[31])
    next_crc.append(d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[0]
                    ^ c[24] ^ c[25] ^ c[27] ^ c[28])
    next_crc.append(d[5] ^ d[4] ^ d[2] ^ d[1] ^ c[1]
                    ^ c[25] ^ c[26] ^ c[28] ^ c[29])
    next_crc.append(d[5] ^ d[3] ^ d[2] ^ d[0] ^ c[2]
                    ^ c[24] ^ c[26] ^ c[27] ^ c[29])
    next_crc.append(d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[3]
                    ^ c[24] ^ c[25] ^ c[27] ^ c[28])
    next_crc.append(d[6] ^ d[5] ^ d[4] ^ d[2] ^ d[1] ^ d[0] ^
                    c[4] ^ c[24] ^ c[25] ^ c[26] ^ c[28] ^ c[29] ^ c[30])
    next_crc.append(d[7] ^ d[6] ^ d[5] ^ d[3] ^ d[2] ^ d[1] ^
                    c[5] ^ c[25] ^ c[26] ^ c[27] ^ c[29] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[6] ^ d[4] ^ d[3] ^ d[2] ^ c[6]
                    ^ c[26] ^ c[27] ^ c[28] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[5] ^ d[4] ^ d[3] ^ c[7]
                    ^ c[27] ^ c[28] ^ c[29] ^ c[31])
    next_crc.append(d[5] ^ d[4] ^ d[0] ^ c[8] ^ c[24] ^ c[28] ^ c[29])
    next_crc.append(d[6] ^ d[5] ^ d[1] ^ c[9] ^ c[25] ^ c[29] ^ c[30])
    next_crc.append(d[7] ^ d[6] ^ d[2] ^ c[10] ^ c[26] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[3] ^ c[11] ^ c[27] ^ c[31])
    next_crc.append(d[4] ^ c[12] ^ c[28])
    next_crc.append(d[5] ^ c[13] ^ c[29])
    next_crc.append(d[0] ^ c[14] ^ c[24])
    next_crc.append(d[6] ^ d[1] ^ d[0] ^ c[15] ^ c[24] ^ c[25] ^ c[30])
    next_crc.append(d[7] ^ d[2] ^ d[1] ^ c[16] ^ c[25] ^ c[26] ^ c[31])
    next_crc.append(d[3] ^ d[2] ^ c[17] ^ c[26] ^ c[27])
    next_crc.append(d[6] ^ d[4] ^ d[3] ^ d[0] ^ c[18]
                    ^ c[24] ^ c[27] ^ c[28] ^ c[30])
    next_crc.append(d[7] ^ d[5] ^ d[4] ^ d[1] ^ c[19]
                    ^ c[25] ^ c[28] ^ c[29] ^ c[31])
    next_crc.append(d[6] ^ d[5] ^ d[2] ^ c[20] ^ c[26] ^ c[29] ^ c[30])
    next_crc.append(d[7] ^ d[6] ^ d[3] ^ c[21] ^ c[27] ^ c[30] ^ c[31])
    next_crc.append(d[7] ^ d[4] ^ c[22] ^ c[28] ^ c[31])
    next_crc.append(d[5] ^ c[23] ^ c[29])
    return bool_list_to_int(next_crc)


def calc_vanilla_parallel_crc32(data: List[int]) -> int:
    crc: int = 0
    for num in data:
        crc = calc_vanilla_parallel_crc32_step(num, crc)
    return crc


def calc_inv_parallel_crc32(data: List[int]) -> int:
    return inv_crc32_decorator(data, calc_vanilla_parallel_crc32)


def calc_bit_rev_inv_parallel_crc32(data: List[int]) -> int:
    return rev_crc32_decorator(data, calc_inv_parallel_crc32)


def calc_bytewise_bit_rev_inv_parallel_crc32(data: List[int]) -> int:
    return bytewise_bit_rev_crc32_decorator(data, calc_bit_rev_inv_parallel_crc32)


# ZLIB CRC32

def calc_bit_rev_inv_zlib_crc32(data: List[int]) -> int:
    return zlib.crc32(bytes(data)) & 0xFFFFFFFF


def calc_bytewise_bit_rev_inv_zlib_crc32(data: List[int]) -> int:
    return bytewise_bit_rev_crc32_decorator(data, calc_bit_rev_inv_zlib_crc32)


def calc_inv_zlib_crc32(data: List[int]) -> int:
    return rev_crc32_decorator(data, calc_bit_rev_inv_zlib_crc32)


def calc_vanilla_zlib_crc32(data: List[int]) -> int:
    return inv_crc32_decorator(data, calc_inv_zlib_crc32)


lst: List[int] = hex_string_to_int_list('')

# # CRC32 without inversion of input and output bits. Pure polynomial division.
# print("\nTYPE 1: VANILLA CRC32")
# print_hex_dec(calc_vanilla_zlib_crc32(lst))
# print_hex_dec(calc_vanilla_serial_crc32(lst))
# print_hex_dec(calc_vanilla_parallel_crc32(lst))

# # CRC32 with first four input bytes and output inverted.
# print("\nTYPE 2: TYPE 1 + INPUT WITH FIRST 4 BYTES INVERTED & OUTPUT INVERTED")
# print("Use this for appending to message")
# print_hex_dec(calc_inv_zlib_crc32(lst))
# print_hex_dec(calc_inv_serial_crc32(lst))
# print_hex_dec(calc_inv_parallel_crc32(lst))

# # CRC32 with inversion from before and bytewise bit-reverse inputs and full
# # bit-reverse output.
# print("\nTYPE 3: TYPE 2 + INPUT BYTEWISE REVERSED & OUTPUT REVERSED")
# print_hex_dec(calc_bit_rev_inv_zlib_crc32(lst))
# print_hex_dec(calc_bit_rev_inv_serial_crc32(lst))
# print_hex_dec(calc_bit_rev_inv_parallel_crc32(lst))

# print("\nTYPE 4: TYPE 3 + OUTPUT BYTEWISE REVERSED")
# print("OUTPUT BYTES REVERSED COMPARED TO TYPE 2")
# print("This must be finally transmitted")
# print_hex_dec(calc_bytewise_bit_rev_inv_zlib_crc32(lst))
# print_hex_dec(calc_bytewise_bit_rev_inv_serial_crc32(lst))
# print_hex_dec(calc_bytewise_bit_rev_inv_parallel_crc32(lst))

# # Show every step in crc32 calculation
# all_sub_sentences: List[List[int]] = [lst[0:i+1] for i in range(0, len(lst))]
# for i, sub_sentence in enumerate(all_sub_sentences):
#     checksum = calc_bytewise_bit_rev_inv_serial_crc32(sub_sentence)
#     print('0x{:02x}: 0x{:08x}'.format(lst[i], checksum))
