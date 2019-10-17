import os, sys, struct, time, socket, threading, subprocess, logging
from ctypes import *
from ctypes.wintypes import *
from ctypes import wintypes

hDll = ctypes.WinDLL ("kernel32.dll")

GENERIC_READ            = 0x80000000
GENERIC_WRITE           = 0x40000000

FILE_SHARE_READ         = 1 
FILE_SHARE_WRITE        = 2 
OPEN_EXISTING           = 3
FILE_ATTRIBUTE_SYSTEM   = 4
FILE_FLAG_OVERLAPPED    = 0x40000000

# Setup the DeviceIoControl function arguments and return type.
# See ctypes documentation for details on how to call C functions from python, and why this is important.
ctypes.windll.kernel32.DeviceIoControl.argtypes = [
    wintypes.HANDLE,                    # _In_          HANDLE hDevice
    wintypes.DWORD,                     # _In_          DWORD dwIoControlCode
    wintypes.LPVOID,                    # _In_opt_      LPVOID lpInBuffer
    wintypes.DWORD,                     # _In_          DWORD nInBufferSize
    wintypes.LPVOID,                    # _Out_opt_     LPVOID lpOutBuffer
    wintypes.DWORD,                     # _In_          DWORD nOutBufferSize
    ctypes.POINTER( wintypes.DWORD ),   # _Out_opt_     LPDWORD lpBytesReturned
    wintypes.LPVOID                     # _Inout_opt_   LPOVERLAPPED lpOverlapped
]
ctypes.windll.kernel32.DeviceIoControl.restype = wintypes.BOOL

ULONG_PTR = ULONG

class _OVERLAPPED(Structure):
#https://docs.microsoft.com/bg-bg/windows/win32/api/minwinbase/ns-minwinbase-overlapped    
    pass


class _OVERLAPPED(Structure):# ?????????
    _fields_ = [
        ("Internal",        ULONG_PTR),
        ("InternalHigh",    ULONG_PTR),
        ("Offset",          DWORD),
        ("OffsetHigh",      DWORD),
        ("hEvent",          HANDLE)
    ]
    def __init__(self):
        self.Offset         =  0
        self.OffsetHigh     =  0

OVERLAPPED   = _OVERLAPPED
LPOVERLAPPED = POINTER(_OVERLAPPED)


_stdcall_libraries = {}
_stdcall_libraries['kernel32'] = WinDLL('kernel32')
#https://www.rose-hulman.edu/class/se/archive/csse120-old/csse120-old-terms/201130/Resources/Tools/Python/PutContentsInSitePackages/PutContentsInSitePackages/serial/win32.py


LPDWORD = POINTER(DWORD)

ResetEvent = _stdcall_libraries['kernel32'].ResetEvent
ResetEvent.restype = BOOL
ResetEvent.argtypes = [HANDLE]

ReadFile = _stdcall_libraries['kernel32'].ReadFile
ReadFile.restype = BOOL
ReadFile.argtypes = [HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED]

GetOverlappedResult = _stdcall_libraries['kernel32'].GetOverlappedResult
GetOverlappedResult.restype = BOOL
GetOverlappedResult.argtypes = [HANDLE, LPOVERLAPPED, LPDWORD, BOOL]


def TunTap(nic_type, nic_name = None):
    if not sys.platform.startswith("win"):
        tap = Tap(nic_type,nic_name)
    else:
        tap = WindowsTap(nic_type)
    tap.create()
    return tap

class Tap(object):
    def __init__(self, nic_type, nic_name = None):
        self.nic_type = nic_type
        self.name = nic_name
        self.mac = b"\x00"*6
        self.handle = None
        self.ip = None
        self.mask = None
        self.gateway = None
        self.read_lock = threading.Lock()
        self.write_lock = threading.Lock()
        self.quitting = False


class WindowsTap(Tap):
    def __init__(self,nic_type):
        super().__init__(nic_type)
        self.component_id = "tap0901"
        self.adapter_key = r'SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002BE10318}'
        self.TAP_IOCTL_GET_MAC                  = self._TAP_CONTROL_CODE( 1, 0)
        self.TAP_IOCTL_GET_VERSION              = self._TAP_CONTROL_CODE( 2, 0)
        self.TAP_IOCTL_GET_MTU                  = self._TAP_CONTROL_CODE( 3, 0)
        self.TAP_IOCTL_GET_INFO                 = self._TAP_CONTROL_CODE( 4, 0)
        self.TAP_IOCTL_CONFIG_POINT_TO_POINT    = self._TAP_CONTROL_CODE( 5, 0)
        self.TAP_IOCTL_SET_MEDIA_STATUS         = self._TAP_CONTROL_CODE( 6, 0)
        self.TAP_IOCTL_CONFIG_DHCP_MASQ         = self._TAP_CONTROL_CODE( 7, 0)
        self.TAP_IOCTL_GET_LOG_LINE             = self._TAP_CONTROL_CODE( 8, 0)
        self.TAP_IOCTL_CONFIG_DHCP_SET_OPT      = self._TAP_CONTROL_CODE( 9, 0)
        self.TAP_IOCTL_CONFIG_TUN               = self._TAP_CONTROL_CODE(10, 0)
        self.read_overlapped                    = OVERLAPPED()
        self.read_overlapped.hEvent             = ctypes.windll.kernel32.CreateEventW(None, True, False, None)
        #print('read_overlapped.hEvent', self.read_overlapped.hEvent)
        self.write_overlapped                   = OVERLAPPED()
        self.write_overlapped.hEvent            = ctypes.windll.kernel32.CreateEventW(None, True, False, None)
        #print('write_overlapped.hEvent', self.write_overlapped.hEvent) 

    def _CTL_CODE(self,device_type, function, method, access):
        return (device_type << 16) | (access << 14) | (function << 2) | method;

    def _TAP_CONTROL_CODE(self,request, method):
        return self._CTL_CODE(34, request, method, 0)

    def OpenKey(self, root, key):
        KEY_QUERY_VALUE = 0x0001 
        KEY_READ = 0x00020019
        INVALID_HANDLE_VALUE = 4294967295
        val = HKEY(INVALID_HANDLE_VALUE)
        err = ctypes.windll.advapi32.RegOpenKeyExW( root, key, 0, KEY_READ, byref( val ) )
        if 0 == err: 
            return HKEY(val.value)
        print('[ERROR] OpenKey', key)
        return None       

    def QueryValueString(self, hkey, name):
        size = 256
        lpcbData = DWORD(size)
        buf = create_string_buffer(size)
        err = ctypes.windll.advapi32.RegQueryValueExW( hkey, name, 0, b'REG_SZ', cast(buf, LPBYTE), byref( lpcbData ) )
        if 0 == err:
            return buf[::2].decode().rstrip('\x00')
        print('[ERROR] QueryValueString', name)
        return None

    def _get_device_guid(self):
        HKEY_LOCAL_MACHINE = 2147483650 
        adapters = self.OpenKey(HKEY_LOCAL_MACHINE, self.adapter_key)
        if adapters == None: 
            return None
        for i in range(30):
            enum_name = create_unicode_buffer(32) # 00XX
            err = ctypes.windll.advapi32.RegEnumKeyW(adapters, i, enum_name, 32)
            if 0 == err:
                adapter = self.OpenKey(adapters, enum_name)
                if c_void_p(0) == adapter: continue
                #print('ADAPTER', i, adapter)
                if self.component_id == self.QueryValueString(adapter, 'ComponentId'):
                    guid = self.QueryValueString(adapter, 'NetCfgInstanceId')
                    print('FOUND', self.component_id, guid)
                    return guid
            else: break
        return None

    def _mac2string(self, mac):
        print('mac', mac)
        mac_string = ""
        for i in range( len(mac) ):
            mac_string += "%02X"%mac[i]
            if i< len(mac)-1:
                mac_string +="-"
        return mac_string

    def _getNameByMac(self, mac):
        result = str(subprocess.check_output("ipconfig/all", shell = True))#.decode().encode().decode()
        res = result.split("adapter")
        for i in range(1, len(res)):
            if res[i].find( self._mac2string(mac) ) > 0:
                res = res[i].split(":")[0].strip()
                print('FIND ADAPTER', res)
                return res
        print('NO ADAPTER')



    def create(self):
        guid =  self._get_device_guid()
        if None == guid:
            print('[ERROR] TAP not found')
            return None
        name = "\\\\.\\Global\\%s.tap"%guid
        #print('FILE', name.encode())
        self.handle = ctypes.windll.kernel32.CreateFileA(
            name.encode(),
            GENERIC_READ | GENERIC_WRITE, 
            0,
            None, OPEN_EXISTING, 
            FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
            None
        )        
        if self.handle == -1: # INVALID_HANDLE_VALUE
            print('TAP_TUN_SLIP SERVICE WORK')
            return None # [WinError 31] A device attached to the system is not functioning.
            raise ctypes.WinError()
            return None
        if self.handle:
            print('TAP-TUN FILE HANDLE =', self.handle)
            return self
        return None


    def config(self, ip, mask, gateway = "0.0.0.0"):
        self.ip = ip
        self.mask = mask
        self.gateway = gateway
        try:
            input  = b'\x01\x00\x00\x00'
            output = create_string_buffer(512)
            res = DWORD(0)
            ctypes.windll.kernel32.DeviceIoControl(self.handle, self.TAP_IOCTL_SET_MEDIA_STATUS, input, 4, output, 512, byref(res), None)
            print('output', output)
            ipnet  = struct.pack("I",struct.unpack("I", socket.inet_aton(self.ip))[0]&struct.unpack("I", socket.inet_aton(self.mask))[0])
            ipcode = socket.inet_aton(self.ip) + ipnet + socket.inet_aton(self.mask)
            if self.nic_type=="Tap":
                flag = self.TAP_IOCTL_CONFIG_POINT_TO_POINT
            if self.nic_type=="Tun":
                flag =  self.TAP_IOCTL_CONFIG_TUN
            ctypes.windll.kernel32.DeviceIoControl(self.handle, flag, ipcode, 16, None, 0, byref(res), None)
            self.mac= create_string_buffer(6)
            ctypes.windll.kernel32.DeviceIoControl(self.handle, self.TAP_IOCTL_GET_MAC, self.mac, 6, self.mac, 6, byref(res), None) # ???
            print('self.mac', self.mac)
            self.name = self._getNameByMac(self.mac)          
        except Exception as exp:
            print('[ERROR]', exp)
            logging.debug(exp)
            ctypes.windll.kernel32.CloseHandle(self.handle)
            exit(1)
        print('self.name', self.name)
        sargs = r"netsh interface ip set address name=NAME source=static addr=ADDRESS mask=MASK gateway=GATEWAY"
        sargs = sargs.replace("NAME", "\"%s\""%self.name)
        sargs = sargs.replace("ADDRESS", self.ip)
        sargs = sargs.replace("MASK", self.mask)
        if self.gateway == "0.0.0.0":
            sargs = sargs.replace("gateway=GATEWAY","")
        else:
            sargs = sargs.replace("GATEWAY", self.gateway)
        subprocess.check_call(sargs, shell = True)

    def read(self):
        self.read_lock.acquire()
        result = None
        try:
            err = ResetEvent(self.read_overlapped.hEvent) # ok (TRUE)
            if 0 == err: raise ctypes.WinError()

            rd = DWORD()
            buf = create_string_buffer(2000)  
            err = ReadFile(
                self.handle, 
                cast(buf, LPBYTE), 
                2000, 
                byref(rd), 
                self.read_overlapped
            ) # ok (TRUE).
            # print('ReadFile ERROR =', GetLastError())
            if GetLastError() == 997: # ERROR_IO_PENDING
                n = DWORD()
                err = GetOverlappedResult(self.handle, self.read_overlapped, byref(n), True)
                if 0 == err: raise ctypes.WinError()
                data = buf[:-1]
                result = buf[:n.value]
            elif err == 0:
                result = buf
            else: raise ctypes.WinError()
        finally:
            self.read_lock.release()
        print('GetOverlappedResult', result)
        return result

    def close(self):
        err = ctypes.windll.kernel32.CloseHandle(self.handle)
        if 0 == err: raise ctypes.WinError()
        print('close', self.handle, err)


class thRead(threading.Thread):
    def __init__(self, t, s):
        threading.Thread.__init__(self)
        self.t = t
        self.s = s
    def run(self):
        while 1:
            buf = self.t.read() 
            #slip.write(s, buf)       

def isAzureSphereAdapter():
    r = str( subprocess.check_output("ipconfig/all", shell = True) )
    #print(r.split('\\r\\n'))
    r = r.split("adapter")
    for i in range(1, len(r)): 
        if r[i].find('Azure Sphere') > 0:
            res = r[i].split(":")[0].strip()
            print( res )
            return res # 'Azure Sphere'
    return None            
#isAzureSphereAdapter()

t = WindowsTap('Tap')
if None != t.create():
    s = None # start serial
    r = thRead(t, s)
    r.start()
else:
    print('DO REST API')
#t.close()