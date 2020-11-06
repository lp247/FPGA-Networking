from typing import List

def hex_string_to_int_list(hex: str) -> List[int]:
    return list(bytes.fromhex(hex))

def int_list_to_hex_string(lst: List[int]) -> str:
    return ' '.join(['{:02x}'.format(i) for i in lst])

def split_bytes(lst: List[int]) -> List[int]:
    splits: List[int] = []
    for i in lst:
        splits.append((i % 4))
        splits.append((i % 16) >> 2)
        splits.append((i % 64) >> 4)
        splits.append(i >> 6)
    return splits

def join_dibits(lst: List[int]) -> List[int]:
    return [lst[i] + (lst[i + 1] << 2) + (lst[i + 2] << 4) + (lst[i + 3] << 6) for i in range(0, len(lst), 4)]


def split_byte_string(hex: str) -> str:
    return int_list_to_hex_string(split_bytes(hex_string_to_int_list(hex)))

# # Split
# msg: str = ''
# print(split_byte_string(msg))

# # Join
# msg: str = ''
# print(int_list_to_hex_string(join_dibits(hex_string_to_int_list(msg))))
