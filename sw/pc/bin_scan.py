import math
from heapq import nlargest

def calculate_entropy(data):
    if not data:
        return 0
    frequency = [0]*256
    for byte in data:
        frequency[byte] += 1
    entropy = 0
    for freq in frequency:
        if freq > 0:
            prob = freq / len(data)
            entropy -= prob * math.log2(prob)
    return entropy

def find_high_entropy_chunks(file_path, chunk_size=32, top_n=10):
    chunk_offsets = []  # List to store (entropy, offset) tuples

    with open(file_path, 'rb') as file:
        offset = 0  # This will hold the current offset in the file
        while True:
            chunk = file.read(chunk_size)
            if not chunk:
                break

            # Calculate entropy of the chunk
            entropy = calculate_entropy(chunk)
            chunk_offsets.append((entropy, offset))

            # Move to the next chunk
            offset += len(chunk)

    # Sort the list by entropy values in descending order and return the top N
    top_chunks = nlargest(top_n, chunk_offsets, key=lambda x: x[0])
    return top_chunks

# Replace 'path_to_your_file.bin' with your actual file path
top_entropy_chunks = find_high_entropy_chunks('badge.X.production.bin')
for entropy, offset in top_entropy_chunks:
    print(f'Offset: 0x{offset:08X}, Entropy: {entropy:.4f}')
