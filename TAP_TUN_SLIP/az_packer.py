import os, sys, struct, time, threading, subprocess, logging, random
from binascii import hexlify

def HEX(s):
    return hexlify(s).decode("ascii").upper()

S_IXOTH = 1
S_IXGRP = 8
S_IXUSR = 64
S_ISREG = 32768

DEFAULT_FILE_PERM = 420

def pack_inode():
    pass

def pack_file(fileName):
    pass

def pack_file(folder):
    pass

def pack_header(size = 0):
    bin  = struct.pack("I", 0x28CD3D45)                         # offset[0]
    bin += struct.pack("I", size)                               # size of image
    bin += struct.pack("I", 3)                                  # 3
    bin += struct.pack("I", 0)                                  # 0
    bin += bytearray("Compressed ROMFS".encode('utf-8'))        # offset[16:32]
    bin += struct.pack("I", 0)                                  # crc?
    bin += struct.pack("I", 0)                                  # 0
    bin += struct.pack("I", 0)                                  # 0
    bin += struct.pack("I", 0)                                  # root entry count
    bin += bytearray("Compressed\0\0\0\0\0\0".encode('utf-8'))  # offset[48:64]
    print(HEX(bin))
    pass

def pack_all(rootFolder):
    pack_header()
    #pack_inodes(rootFolder)
    #pack_files()
    #CRC(0xEDB88320, img)
    return img

pack_header()