from typing import List

def hex_string_to_int_list(hex: str) -> List[int]:
    return list(bytes.fromhex(hex))

def invert_bits(word: int, wordsize: int = None) -> int:
    size: int = wordsize if wordsize != None else word.bit_length() if word != 0 else 1
    return (1 << size) - 1 - word

def calc_checksum(hex: str) -> int:
    numbers: List[int] = hex_string_to_int_list(hex)
    if (len(numbers) % 2 == 1):
        numbers.append(0)
    mod_numbers: List[int] = [numbers[i] * 256 + numbers[i + 1] for i in range(0, len(numbers), 2)]
    num_sum: int = sum(mod_numbers)
    return invert_bits((num_sum % 65536) + (num_sum >> 16), 16)

# Checksum calculation
msg: str = ''
print(hex(calc_checksum(msg)), calc_checksum(msg))

# Show every step in checksum calculation
msg: str = '00 11 00 12 11 11 11 11 22 22 22 22 de 60 00 35 00 12 10 ce aa 00 00 00 00 00 00 00 00 00'
msgs: List[str] = [msg[0:i + 5] for i in range(0, len(msg), 6)]
for i, m in enumerate(msgs):
    print(i, calc_checksum(m), hex(calc_checksum(m)))

# 1 2 3 2 2 3 3 2 = 10111001 10111110 = b9 be
# 3 1 0 2 3 2 0 2 = 10000111 10001011 = 87 8b