import os, sys, struct, time
import os.path
from os.path import join
from serial import Serial
from binascii import hexlify
import inspect

def ERROR(message):
    print("\n[ERROR] {}".format(message))
    exit(1)

def ASSERT(flag, message):
    if flag == False: ERROR(message)

def hexs(s):
    return hexlify(s).decode().upper()

# https://github.com/openpst/libopenpst/blob/master/include/qualcomm/dm_efs.h
kDiagEfsHello           =  0  # Parameter negotiation packet               
kDiagEfsQuery           =  1  # Send information about EFS2 params         
kDiagEfsOpen            =  2  # Open a file                                
kDiagEfsClose           =  3  # Close a file                               
kDiagEfsRead            =  4  # Read a file                                
kDiagEfsWrite           =  5  # Write a file                               
kDiagEfsSymlink         =  6  # Create a symbolic link                     
kDiagEfsReadLink        =  7  # Read a symbolic link                       
kDiagEfsUnlink          =  8  # Remove a symbolic link or file             
kDiagEfsMkdir           =  9  # Create a directory                         
kDiagEfsRmdir           = 10  # Remove a directory                         
kDiagEfsOpenDir         = 11  # Open a directory for reading               
kDiagEfsReadDir         = 12  # Read a directory                           
kDiagEfsCloseDir        = 13  # Close an open directory  

class QDL:
    def __init__(self, ser):
        self.s = ser

    def read(self, command):
        b = b''
        rx = self.s.read(2) # read header
        b += rx  
        ASSERT(2 == len(rx), 'read header len')
        ASSERT(b'\x7E\x01' == rx, 'read header')    
        rx = self.s.read(2) # read packed length  
        b += rx
        ASSERT(2 == len(rx), 'read lenght len')
        L = struct.unpack("<H", rx)
        L = L[0]   
        rx = self.s.read(2) # read 4B3E ???
        b += rx 
        ASSERT(2 == len(rx), 'read 4B3E len')
        #ASSERT(b'\x4B\x3E' == rx, 'read 4B3E: ' + hexs(b)) 
        rx = self.s.read(2) # read command length  
        b += rx 
        ASSERT(2 == len(rx), 'read command len')  
        C = struct.unpack("<H", rx) 
        C = C[0]
        #ASSERT(C == command, 'read wrong command: ' + '[{:02X}] <<<'.format(command))
        buffer = self.s.read(L - 4)
        b += buffer 
        ASSERT(L - 4 == len(buffer), 'read buffer size') 
        rx = self.s.read(1) # read footer
        b += rx
        ASSERT(1 == len(rx), 'read footer len')
        ASSERT(b'\x7E' == rx, 'read futer')    
        print('[{:02X}] <<<'.format(command), hexs(b), '\n',buffer)  
        return buffer     

    def write(self, command, buffer):
        print()
        tx = b'\x7E\x01' + struct.pack("<H", len(buffer) + 4) + b'\x4B\x3E' + struct.pack("<H", command) + buffer + b'\x7E'
        print('[{:02X}] >>>'.format(command), hexs(tx))
        return self.s.write(tx)

    def OpenFile(self, fileName, flag = 0, mode = 0):
        q.write(kDiagEfsOpen, struct.pack("<I", flag) + struct.pack("<I", mode) + fileName ) #41 02 00 00 B6 01 00 00
        q.read(kDiagEfsOpen)        
        pass

    def WriteFile(self, fileNmae):
        fd = open(fileNmae, 'rb')
        data = 1
        while data: 
            data = fd.read(0x400)
            print('data', len(data), hexs(data))    
        fd.close()    

    def CloseFile(self):
        q.write(kDiagEfsClose, b'\x00\x00\x00\x00') 
        q.read(kDiagEfsClose)

    def wr(self, tx, rx):
        print('W>', hexs(tx))
        self.s.write(tx)
        rs = self.s.read(len(rx)) 
        print('R<', hexs(rs))
        ASSERT(len(rs) == len(rx), 'read len')
        ASSERT(rs == rx, 'read data')      
        print()

    def touch(self, folder):
        print('TOUCH', folder)
        tx = b'\x7E\x01\x0C\x00\x4B\x3E\x0B\x00' + folder.encode('utf-8') + b'\x00\x7E'
        print('W>', hexs(tx))
        self.s.write(tx)
        rs = self.s.read(17)    
        print('R<', hexs(rs)) # 7E010C004B3E0B0002000000000000007E
        ASSERT(len(rs) == 17, 'touch len')       
        print() 

    def list(self, n):
        #> 7E 01 0C 00 4B 3E 0C 00 01 00 00 00 01 00 00 00 7E   LIST 1
        print('LIST', n) 
        tx = b'\x7E\x01\x0C\x00\x4B\x3E\x0C\x00\x01\x00\x00\x00' + struct.pack("<I", n) + b'\x7E'
        print('W>', hexs(tx))
        rs = self.s.read(128)    
        print('R<', hexs(rs))
        print() 

    def connect(self):
        self.s.timeout = 1
        self.wr(b'\x0C\x14\x3A\x7E',                b'\x13\x0C\xD2\x7A\x7E')
        self.wr(b'\x4B\x04\x0E\x00\x0D\xD3\x7E',    b'\x13\x4B\x04\x0E\x00\x28\x49\x7E')
        self.wr(b'\x4B\x08\x02\x00\x0E\xDF\x7E',    b'\x4B\x08\x02\x00\x01\x50\x08\x7E')
        self.wr(b'\x4B\x12\x18\x02\x01\x00\xD2\x7E',b'\x4B\x12\x18\x02\x01\x00\xAA\xF0\x7E')
        #self.wr(b'\x7E\x01\x06\x00\x4B\x3E\x0B\x00\x2F\x00\x7E', b'\x7E\x01\x0C\x00\x4B\x3E\x0B\x00\x01\x00\x00\x00\x00\x00\x00\x00\x7E')
        print('CONNECT')   



dir = os.path.dirname( sys.argv[0] )
ser = Serial("COM9", 115200 )

q = QDL(ser)
q.WriteFile( join(dir,'image.png') )

exit(0)
q.connect()
q.write(kDiagEfsOpenDir, b'/datatx\0')
q.read(kDiagEfsOpenDir)

#q.OpenFile(b'/datatx/TEST.txt\0')
#q.CloseFile()



#q.write(kDiagEfsReadDir, b'\x01\x00\x00\x00\x01\x00\x00\x00')
#q.read(kDiagEfsReadDir)
#q.write(kDiagEfsReadDir, b'\x01\x00\x00\x00\x02\x00\x00\x00')
#q.read(kDiagEfsReadDir)

q.write(kDiagEfsCloseDir, b'\x01\x00\x00\x00')
q.read(kDiagEfsCloseDir)


ser.close()
