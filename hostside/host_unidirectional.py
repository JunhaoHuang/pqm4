#!/usr/bin/env python3
import serial
import sys
import platform

if platform.system() == "Darwin":
    dev = serial.Serial("/dev/tty.usbserial-0001", 38400, timeout=100)
else:
    dev = serial.Serial("/dev/ttyACM0", 38400, timeout=100)

print("> Returned data:", file=sys.stderr)

x=dev.read()
while (len(x)==1):
    sys.stdout.buffer.write(x)
    sys.stdout.flush()
    x=dev.read()
dev.close()
