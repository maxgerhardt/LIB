# Copyright 2014-present PlatformIO <contact@platformio.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from os.path import join
from SCons.Script import (AlwaysBuild, Builder, COMMAND_LINE_TARGETS, Default, DefaultEnvironment)

env = DefaultEnvironment()
platform = env.PioPlatform()

env.Replace(
    BUILD_DIR = env.subst("$BUILD_DIR").replace("\\", "/"),
    AR="arm-none-eabi-ar",
    AS="arm-none-eabi-as",
    CC="arm-none-eabi-gcc",
    GDB="arm-none-eabi-gdb",
    CXX="arm-none-eabi-g++",
    OBJCOPY="arm-none-eabi-objcopy",
    RANLIB="arm-none-eabi-ranlib",
    SIZETOOL="arm-none-eabi-size",
    ARFLAGS=["rc"],
    SIZEPROGREGEXP=r"^(?:\.text|\.data|\.bootloader)\s+(\d+).*",
    SIZEDATAREGEXP=r"^(?:\.data|\.bss|\.noinit)\s+(\d+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD='$SIZETOOL --mcu=$BOARD_MCU -C -d $SOURCES',
    PROGSUFFIX=".elf"
)

####################################################
# Target: Build executable and linkable program
####################################################
elf = env.BuildProgram() ### run cores/board.py
src = env.ElfToBin( join("$BUILD_DIR", "program"), elf )
env.GFH("program")
Default(src)




#from pprint import pprint
#pprint(vars(env))
