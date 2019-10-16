import os, sys, struct, socket, threading, subprocess, logging
from ctypes import *
from ctypes.wintypes import *
from ctypes import wintypes

hDll = ctypes.WinDLL ("kernel32.dll")

GENERIC_READ  = 2147483648 
GENERIC_WRITE = 1073741824 
FILE_SHARE_READ = 1 
FILE_SHARE_WRITE = 2 
OPEN_EXISTING = 3
FILE_ATTRIBUTE_SYSTEM = 4
FILE_FLAG_OVERLAPPED = 1073741824

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
OVERLAPPED   = _OVERLAPPED
LPOVERLAPPED = POINTER(_OVERLAPPED)

class H_OVERLAPPED():         # ?????????
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
        self.hEvent         = -1

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
        eventhandle                             = ctypes.windll.kernel32.CreateEventW(None, True, False, None)
        self.read_overlapped.hEvent             = eventhandle
        self.write_overlapped                   = OVERLAPPED()
        eventhandle                             = ctypes.windll.kernel32.CreateEventW(None, True, False, None)
        self.write_overlapped.hEvent            = eventhandle
        self.buffer                             = b'0'*2048 # AllocateReadBuffer(2000)    

    def _CTL_CODE(self,device_type, function, method, access):
        return (device_type << 16) | (access << 14) | (function << 2) | method;

    def _TAP_CONTROL_CODE(self,request, method):
        return self._CTL_CODE(34, request, method, 0)

    def _get_device_guid(self):
        pass

    def create(self):
        guid = '{97188393-19A4-47D9-A1D8-C90F9591776A}' # self._get_device_guid()
        name = "\\\\.\\Global\\%s.tap"%guid
        self.handle = ctypes.windll.kernel32.CreateFileA(
            name.encode("ascii"),
            GENERIC_READ | GENERIC_WRITE, 
            0, 
            None, 
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
            None
        )        
        #if self.handle == -1: raise ctypes.WinError()
        if self.handle:
            return self
        else:
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
        #print(result)
        res = result.split("adapter")
        for i in range(1, len(res)):
            if res[i].find( self._mac2string(mac) ) > 0:
                res = res[i].split(":")[0].strip()
                print('FIND ADAPTER', res)
                return res
        print('NO ADAPTER')

    def config(self, ip, mask, gateway = "0.0.0.0"):
        self.ip = ip
        self.mask = mask
        self.gateway = gateway
        try:
            input  = b'\x01\x00\x00\x00'
            output = b'0'*512
            res = wintypes.DWORD(0)
            ctypes.windll.kernel32.DeviceIoControl(self.handle, self.TAP_IOCTL_SET_MEDIA_STATUS, input, 4, output, 512, ctypes.pointer(res), None)
            print('output', output)
            ipnet  = struct.pack("I",struct.unpack("I", socket.inet_aton(self.ip))[0]&struct.unpack("I", socket.inet_aton(self.mask))[0])
            ipcode = socket.inet_aton(self.ip) + ipnet + socket.inet_aton(self.mask)
            if self.nic_type=="Tap":
                flag = self.TAP_IOCTL_CONFIG_POINT_TO_POINT
            if self.nic_type=="Tun":
                flag =  self.TAP_IOCTL_CONFIG_TUN
            ctypes.windll.kernel32.DeviceIoControl(self.handle, flag, ipcode, 16, None, 0, ctypes.pointer(res), None)
            self.mac= b'0'*6
            ctypes.windll.kernel32.DeviceIoControl(self.handle, self.TAP_IOCTL_GET_MAC, self.mac, 6, self.mac, 6, ctypes.pointer(res), None) # ???
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
            ResetEvent(self.read_overlapped.hEvent)
            rd = b'0'*4
            ReadFile(self.handle, self.buffer, 2048, ctypes.pointer(rd), self.read_overlapped)
            print('READ', rd)

            err = 0
            if err == 997:#ERROR_IO_PENDING
                n = GetOverlappedResult(self.handle,self.read_overlapped,True)
                result = bytes(data[:n])
            else:
                result = bytes(data)
        finally:
            self.read_lock.release()
        return result

    def close(self):
        ctypes.windll.kernel32.CloseHandle(self.handle)

#t = TunTap('Tap', 'Azure Sphere')
#t.config("192.168.35.1","255.255.255.0")

r = str( subprocess.check_output("ipconfig/all", shell = True) )
r = r.split("\\r\\n")
for i in range(1, len(r)): print(r[i])


t = WindowsTap('Tap')
t.read()