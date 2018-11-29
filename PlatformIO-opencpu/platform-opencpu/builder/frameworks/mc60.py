import sys
from base64 import b64decode
from os import listdir
from os.path import isdir, isfile, join

from SCons.Script import ARGUMENTS, DefaultEnvironment

from platformio import util

env = DefaultEnvironment()

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-tizenrt")
assert isdir(FRAMEWORK_DIR)