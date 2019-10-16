import sys
import os, sys, struct, time, inspect
import os.path
from os.path import join
from tuntap import TunTap


t = TunTap(nic_type="Tap", nic_name="Azure Sphere")
#t.config("192.168.35.2","255.255.255.0")
while(1): 
    print(t.read())
    time.sleep(0.001)