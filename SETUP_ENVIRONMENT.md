# NEO3 dev environment

Ledger only supports Linux as development environment. This guide uses Ubuntu 20.04.

## Overview
We will create a folder holding the Nano S/X SDKs, an emulator for testing without real hardware and toolchains.
The final structure will look like this

```asm
.
├── app-neo3
├── clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04
├── gcc-arm-none-eabi-10-2020-q4-major
├── nanos-secure-sdk
├── nanox-secure-sdk
├── setup_nanos.sh
├── setup_nanox.sh
├── speculos
└── venv
```

## Prerequisites
- Have Python 3.8 or higher installed.
- Create a folder `/ledger` under `/opt` and `cd` into it

## Setup
### Get the app and emulator
```asm
git clone https://github.com/CityOfZion/ledger-app-neo3.git app-neo3
git clone https://github.com/LedgerHQ/speculos.git
```
### Download the SDKs
```asm
git clone https://github.com/LedgerHQ/nanos-secure-sdk.git
git clone https://github.com/LedgerHQ/nanox-secure-sdk.git
```
### Tool chains
Ledger has specific requirements for their toolchains like requiring Clang 7.0.0 <= version < 10.0.0 with ROPI support. 
Download the following
```asm
wget https://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
```
and extract them in place
```asm
tar xf 
tar xf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 
```
Install additional cross compilation headers
```asm
sudo apt install gcc-multilib g++-multilib
```
and setup `udev` rule to allow loading onto a physical device
```asm
wget -q -O - https://raw.githubusercontent.com/LedgerHQ/udev-rules/master/add_udev_rules.sh | sudo bash
```
Note: If you still cannot connect to your physical device, then modify the udev rules under
`/etc/udev/rules.d/20-hw1.rules` and add your current account as `OWNER` e.g.
```asm
# Nano X
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0004|4000|4001|4002|4003|4004|4005|4006|4007|4008|4009|400a|400b|400c|400d|400e|400f|4010|4011|4012|4013|4014|4015|4016|4017|4018|4019|401a|401b|401c|401d|401e|401f", TAG+="uaccess", TAG+="udev-acl" OWNER="erik"
```

### Python environment
Both the emulator (Speculos) and the app use Python for among others communication. We create a single virtual env for all
our ledger work. 
```asm
python -m venv venv
source venv/bin/activate
pip install python-dev-tools ledgerblue construct jsonschema mnemonic pycrypto pyelftools pbkdf2 pytest Pillow PyQt5
```
Install extra libraries
```asm
sudo apt install libudev-dev libusb-1.0-0-dev
```

### Create environment configuration scripts
Create the file `setup_nanos.sh`
```asm
export PATH="/opt/ledger/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH"
export PATH="/opt/ledger/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04/bin:$PATH"
export BOLOS_SDK="/opt/ledger/nanos-secure-sdk/"
```
and create the file `setup_nanox.sh`
```asm
export PATH="/opt/ledger/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH"
export PATH="/opt/ledger/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04/bin:$PATH"
export BOLOS_SDK="/opt/ledger/nanox-secure-sdk/"
```

## Usage

### building 
From the directory `/opt/ledger/` choose which device you want to build the app for.
1. Enable the right environment `source ./setup_nanos.sh` 
2. Enable the Python venv `source venv/bin/activate`.

Now `cd` into `app-neo3` and run `make` to build the app, or `make load` to build the app and load it onto your physical device.
For the latter make sure your device is unlocked.

Note: The NanoX does not support side-loading. Therefore, your only option is to use the emulator (see next).

### Running the emulator
First follow steps 1 and 2 of the previous building instructions. Now `cd` into the `/speculos` dir. 

To run the `Nano S` app use
```asm
./speculos.py --sdk 2.0 ../app-neo3/bin/app.elf
```

to run the Nano X app use
```asm
./speculos.py -m nanox../app-boilerplate/bin/app.elf
```

