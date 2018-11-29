import sys
import os
import struct
from base64 import b64decode
from os import listdir
from os.path import isdir, isfile, join
from SCons.Script import ARGUMENTS, DefaultEnvironment, Builder
from platformio import util

env = DefaultEnvironment()

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-opencpu")
assert isdir(FRAMEWORK_DIR)

folder_cores = join(FRAMEWORK_DIR, "cores")
folder_core = join(folder_cores, env.BoardConfig().get("build.core")) 

print "CORES: ", folder_cores
print "THIS CORE: ", folder_core

def makeHeader(s_name):       
    dst_name = env.subst( join("$BUILD_DIR", "APPGS3MDM32A01.bin") )
    src_name = env.subst( join("$BUILD_DIR", s_name+".bin") )

    print "SRC: ", src_name
    print "DST: ", dst_name

    dst = open(dst_name, "wb")
    arr = [0x4D, 0x4D, 0x4D, 0x01, 0x40, 0x00, 0x00, 0x00, 0x46, 0x49, 0x4C, 0x45, 0x5F, 0x49, 0x4E, 0x46]
    data = bytearray(arr)
    dst.write(data) 
    arr = [0x4F, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x70, 0x07, 0x00, 0x00, 0x20, 0x29, 0x08]
    data = bytearray(arr)
    dst.write(data) 
    
    src_size = os.stat( src_name ).st_size
    print "SIZE: ", src_size
    dst.write( struct.pack('<i', src_size) ) # write bin size 

    arr = [                        0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
    data = bytearray(arr)
    dst.write(data)     
    arr = [0x40, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
    data = bytearray(arr)
    dst.write(data)   

    src = open(src_name, "rb")
    dst.write( src.read() )

    src.close()
    dst.close()  



env.Append(
    CFLAGS=[
        "-mcpu=%s" % env.BoardConfig().get("build.cpu"),
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-fsingle-precision-constant",        
        "-mthumb",
        "-mthumb-interwork",        
        "-std=c99",
        "-O0",  
        "-Wall",
        "-fno-builtin",
        "-Wstrict-prototypes",
        "-Wshadow",
        "-fno-strict-aliasing",
        "-fno-strength-reduce",
        "-fomit-frame-pointer",
        "-Wp,-w",           
        "-Wno-implicit-function-declaration",
        "-I" + folder_core
    ],

    LINKFLAGS=[        
        "-mcpu=%s" % env.BoardConfig().get("build.cpu") ,
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-mthumb",
        "-mthumb-interwork",   
        "-nostartfiles",     
        "-Rbuild",
        "-Wl,--gc-sections,--relax",
        "-T" + folder_core + "\\linkscript.ld",
        "-L" + folder_core
    ],   

    LIBS=[
        "gcc",
        "_app_start"
    ], 

    BUILDERS=dict(
        ElfToBin=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY",
                "-O",
                "binary",
                "$SOURCES",
                "$TARGET"
            ]), "Building $TARGET"),
            suffix=".bin"
        ),
        GFH=Builder(action=makeHeader)      
    )
)    

libs = []
libs.append(
    env.BuildLibrary(
        join("$BUILD_DIR", "framework"),
        join( folder_cores, env.BoardConfig().get("build.core") ),
        src_filter="-<*> +<*.c>"
    )
)
env.Append(LIBS=libs)

env.AddMethod(makeHeader)
