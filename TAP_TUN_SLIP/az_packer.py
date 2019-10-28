import os, sys, struct, binascii
from os.path import join
from binascii import hexlify

PAGE_SIZE   = 4096
S_ISOCK     = 49152 # 0xC000
S_ISLNK     = 40960 # 0xA000
S_ISREG     = 32768 # 0x8000
S_ISBLK     = 24576 # 0x6000
S_ISDIR     = 16384 # 0x4000
S_ISCHR     =  8192 # 0x2000
S_IFIFO     =  4096 # 0x1000
S_IFMT      = S_IFIFO | S_ISCHR | S_ISDIR | S_ISREG # 0xF000
S_IRUSR     = 256 # 0x0100
S_IWUSR     = 128 # 0x0080
S_IXUSR     =  64 # 0x0040
S_IRWXU     = S_IXUSR | S_IWUSR | S_IRUSR # 0x01C0
S_IRGRP     = 32 # 0x0020
S_IWGRP     = 16 # 0x0010
S_IXGRP     = 8
S_IRWXG     = S_IXGRP | S_IWGRP | S_IRGRP # 0x0038
S_IROTH     = 4
S_IWOTH     = 2
S_IXOTH     = 1
S_IRWXO     = S_IXOTH | S_IWOTH | S_IROTH # 0x0007
S_ISUID     = 2048 # 0x0800
S_ISGID     = 1024 # 0x0400
S_ISVTX     =  512 # 0x0200
ONLY_PERM   = S_IRWXO | S_IRWXG | S_IRWXU # 0x01FF
CLEAR_PERM  = S_ISVTX | S_ISGID | S_ISUID | S_IFMT # 0xFE00
DEFAULT_DIR_PERM  = 493 # 0x01ED
DEFAULT_FILE_PERM = 420 # 0x01A4

NODE_ROOT = 1
NODE_DIR  = 2
NODE_FILE = 3

nodes = []
fs_info = bytearray()
data_offset = PAGE_SIZE

def roundUp4(num): return num + 3 & -4

def default_header(bin):
    bin += struct.pack("I", 0x28CD3D45)                         # offset[0]
    bin += struct.pack("I", 0)                                  # size of image
    bin += struct.pack("I", 3)                                  # 3
    bin += struct.pack("I", 0)                                  # 0
    bin += bytearray("Compressed ROMFS".encode('utf-8'))        # offset[16:32]
    bin += struct.pack("I", 0)                                  # crc32
    bin += struct.pack("I", 0)                                  # 0
    bin += struct.pack("I", 0)                                  # 0
    bin += struct.pack("I", 0)                                  # root entry count
    bin += bytearray("Compressed\0\0\0\0\0\0".encode('utf-8'))  # offset[48:64]

def add_fs_info(info, node):
    info += struct.pack("H", node.mode)                         # [0, 1]
    info += struct.pack("H", node.uid)                          # [2, 3]
    info += struct.pack("L", node.file_size)[:3]                # [4, 5, 6]    
    info += struct.pack("B", node.gid)                          # [7]
    name_size = ( node.name_round_up >> 2 ) & 0x0000003F
    if node.type == NODE_FILE:
        offset = node.data_offset
    else:
        offset = len(info) + 4 + node.name_round_up
        #print('offset', hex(offset))
    offset = offset >> 2
    offset = ( offset << 6 ) & 0xFFFFFFC0
    info += struct.pack("L", offset | name_size)
    if node.type != NODE_ROOT:
        info += node.fs_name   

class aNODE():
    def __init__(self, path):
        global nodes, fs_info, data_offset
        self.index = len(nodes)
        print('NODE[{}]'.format(self.index), path)
        self.path = path
        self.name = os.path.basename(path)
        self.name_round_up = roundUp4( len(self.name) )
        self.fs_name  = bytearray( self.name.encode('utf-8') ) 
        self.fs_name += ( self.name_round_up  - len(self.name) ) * b'\0'

        self.type = 0
        self.mode = DEFAULT_DIR_PERM | S_ISDIR
        self.uid = 0 # ??????????? allways 0
        self.gid = 0 # ??????????? allways 0
        self.file_size = 0
        self.data_size = 0
        self.data_offset = 0

        if 0 == self.index: 
            print('NODE-ROOT', self.name)
            self.type = NODE_ROOT
            self.file_size = 0x30 # ???????????
            self.data_size = PAGE_SIZE
            self.name_round_up = 0
            default_header(fs_info)

        elif os.path.isdir(self.path):
            print('NODE-DIR', self.name)
            self.type = NODE_DIR 
            self.file_size = 0x10 # ???????????

        elif os.path.isfile(self.path):
            print('NODE-FILE', self.name)
            self.type = NODE_FILE

            if self.name == 'app':
                self.mode = 0x83ED # DEFAULT_FILE_PERM | S_ISREG | S_ISVTX ??????????? set as application
            else: 
                self.mode = DEFAULT_FILE_PERM | S_ISREG | S_ISVTX

            self.file_size = os.path.getsize(path)
            
            self.data_size = ( int( self.file_size / PAGE_SIZE ) + 1 ) * PAGE_SIZE            
            print('DATA-OFFSET', hex(data_offset))
            print('DATA-SIZE  ', hex(self.data_size))
            self.data_offset = data_offset 
            data_offset += self.data_size

            print('FILE-SIZE  ', hex(self.file_size))
            f = open(self.path,'rb') 
            self.data = bytearray( f.read() ) 
            print('READ-SIZE  ', hex( len( self.data ) ))
            f.close()
            self.data += (self.data_size - self.file_size) * b'\0'
            print('TOTL-SIZE  ', hex( len( self.data ) ))

        nodes.append( self )
        add_fs_info( fs_info, self )
        print()

def create_approot(path, image):
    if False == os.path.isdir(path):
        print('[ERROR] approot path not found')
        exit(1)
    for PATH, DIRS, FILES in os.walk(path):
        #print(PATH, DIRS, FILES)
        node = aNODE( PATH )
        for f in FILES:    
            node = aNODE( join(PATH, f) )

    image += fs_info + (PAGE_SIZE - len(fs_info)) * b'\0'
    for node in nodes: 
        if hasattr(node,'data'): 
            image += node.data # add images    

def update_header(image):
    image[ 4: 8] = struct.pack("I", len(image))  
    image[44:48] = struct.pack("I", len(nodes)) 

    crc = binascii.crc32(image)
    print('CRC', hex(crc), hex(0x63AAB320))
    image[32:36] = struct.pack("I", crc) # CRC(3988292384)  # 0xEDB88320    

def write_image(name, image):
    f = open(name,'wb') 
    f.write(image) 
    f.close()  

# scan approot folder
img = bytearray()
create_approot('.\\approot', img)
update_header(img)
write_image('.\\image.bin', img)


