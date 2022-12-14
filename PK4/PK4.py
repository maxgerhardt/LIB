import usb.core
import usb.util
import json 
import time

SCR = {}
with open('PIC24FJ256GB206.json') as json_file:
    SCR = json.load(json_file)
#print(SCR)

DEV = None
SCRIPT_NO_DATA          = 256;
SCRIPT_WITH_DOWNLOAD    = 0xC0000101
SCRIPT_WITH_UPLOAD      = 0x80000102

SCR_holdInReset             = [0xB1]
SCR_releaseFromReset        = [0x42, 0xB0]
SCR_shutDownPowerSystem     = [68]
SCR_powerGetSystemStatus    = [71] # SCRIPT_NO_DATA

def SCR_initPowerScript(): # 3,250 V
# 64, (byte)(Vdd & 0xFF), (byte)(Vdd >> 8 & 0xFF), 0, 0, (byte)(VppOperation & 0xFF), (byte)(VppOperation >> 8 & 0xFF), 0, 0, (byte)(Vpp_op & 0xFF), (byte)(Vpp_op >> 8 & 0xFF), 0, 0, 66, 67 };    
    return [ 0x40, 0xB2, 0x0C, 0x00, 0x00, 0xB2, 0x0C, 0x00, 0x00, 0xB2, 0x0C, 0x00, 0x00, 0x42, 0x43 ]

def SCR_applyLedIntensity(level):
    return [0xCF, level & 0xFF]

def EnterTMOD_LV():
    print('EnterTMOD_LV')
    write(DEV, SCRIPT_NO_DATA, scr=SCR['EnterTMOD_LV'])

def ExitTMOD():
    print('ExitTMOD')
    write(DEV, SCRIPT_NO_DATA, scr=SCR['ExitTMOD'])

def PRINT_HEX(TXT, ar, maxSize=64):
    if None == ar:
        print('HEX NONE')
        return
    size = len(ar)
    if size > maxSize: size = maxSize
    ar = ar[0:size]
    txt = ''
    for a in ar:
        txt += '%02X ' % a
    print(TXT, txt)

def ERROR(msg):
    global DEV
    print('[ERROR]', msg)
    #if DEV: disconnect(DEV)
    exit(-1)

def hid_write(dev, buffer, timeout=3000):
    if dev:
        try:
            PRINT_HEX('WRITE:\t', buffer)
            dev.write(0x02, buffer, timeout) 
        except:
            ERROR('HID WRITE')
    else: 
        ERROR('HID DEVICE')

def hid_read(dev, size=0x200, timeout=1000):
    if dev:
        try:
            res = dev.read(0x81, size, timeout)
            PRINT_HEX('READ:\t', res)
            print()
            return res
        except:
            ERROR('HID READ')            
    else: 
        ERROR('HID DEVICE')        

def write(dev, cmd, readSize=0, prm=None, scr=None):
    a = []
    a.append(cmd     & 0xFF)
    a.append(cmd>>8  & 0xFF)
    a.append(cmd>>16 & 0xFF)
    a.append(cmd>>24 & 0xFF)
    a.append(0)
    a.append(0)
    a.append(0)
    a.append(0)

    fullSize = 16 + 4
    if prm: fullSize += len(prm)
    if scr: fullSize += len(scr) + 4
    a.append(fullSize     & 0xFF)
    a.append(fullSize>>8  & 0xFF)
    a.append(fullSize>>16 & 0xFF)
    a.append(fullSize>>24 & 0xFF)  

    a.append(readSize     & 0xFF)
    a.append(readSize>>8  & 0xFF)
    a.append(readSize>>16 & 0xFF)
    a.append(readSize>>24 & 0xFF) 

    if prm:
        size = len(prm)
        a.append(size     & 0xFF)
        a.append(size>>8  & 0xFF)
        a.append(size>>16 & 0xFF)
        a.append(size>>24 & 0xFF)         
        for d in prm: 
            a.append(d)  
    else:
        a.append(0)
        a.append(0)
        a.append(0)
        a.append(0)  

    if scr:
        size = len(scr)
        a.append(size     & 0xFF)
        a.append(size>>8  & 0xFF)
        a.append(size>>16 & 0xFF)
        a.append(size>>24 & 0xFF)         
        for d in scr: 
            a.append(d)

    #print(len(a), a)
    hid_write(dev, a)
    R = hid_read(dev)
    if [] == R:
        ERROR('ANSWER')    
    if len(R) < 0x100:
        ERROR('ANSWER SIZE = ' + str(len(R)))
    if 13 != R[0]: 
        ERROR('ANSWER = ' + str(R[0]))
    return R

def connect():
    global DEV
    DEV = usb.core.find(idVendor=0x04D8, idProduct=0x9012)
    if None == DEV: 
        ERROR("PK4 not found")
    print('DRIVER SERIAL', usb.util.get_string(DEV, DEV.iSerialNumber) )    
    DEV.set_configuration()  

    try: DEV.detach_kernel_driver(0) #Operation not supported or unimplemented on this platform
    except: pass


    hid_write(DEV, [0xE1]) # GET_FIRMWARE_INFO
    res = hid_read(DEV)
    print('BOOT INFO', str(res[32:47], 'utf-8') )    

def disconnect(dev):
    if dev:
        print('disconnect')
        write(dev, SCRIPT_NO_DATA, scr=[0x44])        
        dev.reset()
        


connect()
EnterTMOD_LV()
ExitTMOD()

#write(DEV, SCRIPT_NO_DATA, scr=SCR_applyLedIntensity(1))
#time.sleep(.1)


disconnect(DEV)