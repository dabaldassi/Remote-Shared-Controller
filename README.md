# Remote-Shared-Controller

[![pipeline status](https://gitlab.com/Sharkalash/Remote-Shared-Controller/badges/master/pipeline.svg)](https://gitlab.com/Sharkalash/Remote-Shared-Controller/commits/dev)

Software which allow to share the mouse and keyboard of one computer with several others.

You can download the Windows version [here](https://drive.google.com/open?id=1K1KrCmf863JrMlhXsjX24yzHQBPvRG0H)

## Prerequisites

* CMake
* A c++-14 compiler (GCC or MSVC)
* libx11-dev libfixes-dev for Linux (graphical mode)
* A Linux kernel version greater than 4.9 or Windows 10
* Qt5 for the graphical user interface
* npcap for Windows

## Build

### Linux build

```bash

mkdir build && cd build

cmake .. [-DCMAKE_BUILD_TYPE={Release|Debug}] [-DINCLUDE_TEST={ON|OFF}] [-DNOCURSOR={ON|OFF}]

make

```

If you want to install, then :

```
sudo cmake install
```

### Windows build

First, you must install Microsoft Visual Studio 2019.

Either you open the CMakeList with visual studio to set up the project and then build from the IDE or you can build from the Microsoft Visual Studio developper command prompt. 

In the last case, open this command prompt and go to the root of the project. Then type : 

```bash

mkdir build && cd build

cmake .. [-DCMAKE_BUILD_TYPE={Release|Debug}]

cmake --build .

```

### CMake options

With the option ``INCLUDE_TEST`` enabled, it will build all the unitary tests.

You can enabled the option ``NO_CURSOR`` if you don't want to use the cursor or if you don't have any graphic environment.

By default, ``INCLUDE_TEST`` and ``NOCURSOR`` are OFF.

If you choose to build the tests, you can run all of them from the build directory with the command ``ctest``

## Launch Remote-Share-Controller service

On Linux, you need to be root to launch remote-shared-controller.

On Windows, you must install [npcap](https://nmap.org/npcap/) first.

```bash

./remote-shared-controller -i if_index -k key

```

You can specify the index of the network interface you want to listen on.

If this is not an existing index, it will raise an exception.

It will run in a forever loop as a daemon.

## rsccli

rsccli is a command line interface to communicate with the service.

To see all the available commands :

```bash

./rsccli help

```

## rscgui

rscgui is a graphic user interface designed to use remote-shared-controller.

You can just run rscgui from the build directory with the following :

```bash

./rscgui

```

## Authors

[alpapin](https://github.com/alpapin/)

[dabaldassi](https://github.com/dabaldassi/)
