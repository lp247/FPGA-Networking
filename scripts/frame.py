from typing import List

def hex_string_to_int_list(hex: str) -> List[int]:
    return list(bytes.fromhex(hex))

def int_list_to_hex_string(lst: List[int]) -> str:
    return ' '.join([str(i) for i in lst])

def split_bytes(lst: List[int]) -> List[int]:
    splits: List[int] = []
    for i in lst:
        splits.append((i % 4))
        splits.append((i % 16) >> 2)
        splits.append((i % 64) >> 4)
        splits.append(i >> 6)
    return splits

def split_byte_string(hex: str) -> str:
    return int_list_to_hex_string(split_bytes(hex_string_to_int_list(hex)))

# Split
msg: str = '40 40 cc cc'
print(split_byte_string(msg))
