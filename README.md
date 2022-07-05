[![License BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue.svg)](https://opensource.org/licenses/BSD-2-Clause)
[![License MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)


# `SerialPipe`
Welcome to `SerialPipe` 🎉

`SerialPipe` is a simple program that can be used to read from/write to a serial device node.


## Example
```sh
# Dump /dev/tty.usbmodem21201 @115200 to stdout
spipe /dev/tty.usbmodem21201 115200

# Write "Testolope" to /dev/tty.usbmodem21201 @115200 and dump /dev/tty.usbmodem21201 @115200 to stdout
echo "Testolope" | spipe /dev/tty.usbmodem21201 115200

# Dump /dev/tty.usbmodem21201 @9600 to the file a.out
spipe /dev/tty.usbmodem666 9600 > a.out
```

## Usage
```sh
spipe path-to-device-node baudrate
```