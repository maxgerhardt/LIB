import struct, random
from binascii import hexlify

MY_MAC = b''
MY_IP = b'\xC0\xA8\x23\x02'
AZ_IP = b'\xC0\xA8\x23\x01'

def HEX(s):
    return hexlify(s).decode("ascii").upper()

#00FF05F2352600FF8BB0D971 0806000108000604 0001 00FF8BB0D971C0A8230100FF05F23526C0A82302 REQUEST
#00FF8BB0D97100FFAE9C9749 0806000108000604 0002 00FFAE9C9749C0A8230200FF8BB0D971C0A82301 RESPONSE

if __name__ == "__main__":
    MY_MAC = struct.pack("<HI", 0xFF00, random.randint(1, 0xFFFFFFFE))
    #print ( HEX(MY_MAC) )

TEST_WHO_IS = b'\xFF\xFF\xFF\xFF\xFF\xFF\x00\xFF\x8B\xB0\xD9\x71\x08\x06\x00\x01\x08\x00\x06\x04\x00\x01\x00\xFF\x8B\xB0\xD9\x71\xC0\xA8\x23\x01\x00\x00\x00\x00\x00\x00\xC0\xA8\x23\x02'

def isARP(packet):
    if packet[12:14] == b'\x08\x06': # ARP
        if packet[28:32] != b'\xC0\xA8\x23\x01': # is not 192.168.35.1
            return
        if packet[28:32] != AZ_IP : # is not for me
            return
        if packet[20:22] != b'\x00\x01': # is not request
            return
        print ( "ARP-REQUEST")
        packet[ : 6], packet[6:12] = packet[6:12], MY_MAC # swap eth mac
        packet[20:22] = b'\x00\x02' # set response
        packet[22:32], packet[32:42] = packet[32:42], packet[22:32] # swap arp
        packet[22:28] = MY_MAC
        print ( "<", HEX(packet))
        return packet # response   
    return         

def isETH(packet):
    if packet[ : 6] != b'\xFF\xFF\xFF\xFF\xFF\xFF' and packet[ : 6] != MY_MAC: # DST
        return True
    if packet[14:20] != b'\x00\x01\x08\x00\x06\x04': # ETHERNET, PROTO-IPV4, LEN MAC, LEN IP
        return True
    return False


def route(packet = TEST_WHO_IS):
    packet = bytearray(packet)
    print ( ">", HEX(packet))
    if isETH(packet): 
        print ( "REJECT ETH" )
        return   

    res = isARP(packet)
    if res != None: return res

    # TEST FOR HTTPS:443

    print ( "REJECT" )

route()