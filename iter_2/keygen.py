from cryptography.hazmat.primitives.ciphers import algorithms
from cryptography.hazmat.backends import default_backend
import os

def generate_aes_key(key_size=256):
    """
    Generates a random AES key.

    :param key_size: The size of the key in bits. Valid values are 128, 192, and 256.
    :return: The AES key as a bytes object.
    """
    if key_size not in {128, 192, 256}:
        raise ValueError("Invalid key size. Valid sizes are 128, 192, or 256 bits.")

    # Generate a random key
    key = os.urandom(key_size // 8)
    return key

def main():
    # Generate a 256-bit AES key
    key_size = 256
    key = generate_aes_key(key_size)

    # Print the key in hexadecimal format
    print(f"AES Key ({key_size}-bit): {key.hex()}")

if __name__ == "__main__":
    main()
