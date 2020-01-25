# Remote-Shared-Controller

[![pipeline status](https://gitlab.com/Sharkalash/Remote-Shared-Controller/badges/master/pipeline.svg)](https://gitlab.com/Sharkalash/Remote-Shared-Controller/commits/master)

Software which allow to share the mouse and keyboard of one computer with several others.

## Prerequisites

* CMake
* A c++-14 compiler
* libx11-dev and libfixes-dev

## Build

```bash

mkdir build && cd build

cmake .. [-DCMAKE_BUILD_TYPE={Release|Debug}] [-DINCLUDE_TEST={ON|OFF}] [-DNOCURSOR={ON|OFF}]

make

```

With the option ``INCLUDE_TEST`` enabled, it will build all the unitary tests.

You can enabled the option ``NO_CURSOR`` if you don't want to use the cursor or if you don't have any graphic environment.

By default, ``INCLUDE_TEST`` and ``NOCURSOR`` are OFF.

If you choose to build the tests, you can run all of them from the build directory with the command ``ctest``

## Setup

You need to create the base directory first.

```bash

mkdir -p /var/rsc/localcom

```

## Launch Remote-Share-Controller service

You need to be root to launch remote-shared-controller.

```bash

./remote-shared-controller [if_index]

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

You need to be root to use rsccli.

## Remarks

This project only works on Linux at the moment.

## Authors

[alpapin](https://github.com/alpapin/)
[dabaldassi](https://github.com/dabaldassi/)
