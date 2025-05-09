# Multiprocess Griffion

_This document describes usage of the multiprocess feature. For design information, see the [design/multiprocess.md](design/multiprocess.md) file._

## Build Option

On unix systems, the `--enable-multiprocess` build option can be passed to `./configure` to build new `griffion-node`, `griffion-wallet`, and `griffion-gui` executables alongside existing `griffiond` and `griffion-qt` executables.

## Debugging

The `-debug=ipc` command line option can be used to see requests and responses between processes.

## Installation

The multiprocess feature requires [Cap'n Proto](https://capnproto.org/) and [libmultiprocess](https://github.com/chaincodelabs/libmultiprocess) as dependencies. A simple way to get started using it without installing these dependencies manually is to use the [depends system](../depends) with the `MULTIPROCESS=1` [dependency option](../depends#dependency-options) passed to make:

```
cd <GRIFFION_SOURCE_DIRECTORY>
make -C depends NO_QT=1 MULTIPROCESS=1
CONFIG_SITE=$PWD/depends/x86_64-pc-linux-gnu/share/config.site ./configure
make
src/griffion-node -regtest -printtoconsole -debug=ipc
GRIFFIOND=griffion-node test/functional/test_runner.py
```

The configure script will pick up settings and library locations from the depends directory, so there is no need to pass `--enable-multiprocess` as a separate flag when using the depends system (it's controlled by the `MULTIPROCESS=1` option).

Alternately, you can install [Cap'n Proto](https://capnproto.org/) and [libmultiprocess](https://github.com/chaincodelabs/libmultiprocess) packages on your system, and just run `./configure --enable-multiprocess` without using the depends system. The configure script will be able to locate the installed packages via [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/). See [Installation](https://github.com/chaincodelabs/libmultiprocess/blob/master/doc/install.md) section of the libmultiprocess readme for install steps. See [build-unix.md](build-unix.md) and [build-osx.md](build-osx.md) for information about installing dependencies in general.

## Usage

`griffion-node` is a drop-in replacement for `griffiond`, and `griffion-gui` is a drop-in replacement for `griffion-qt`, and there are no differences in use or external behavior between the new and old executables. But internally after [#10102](https://github.com/GriffionProject/Griffion/pull/10102), `griffion-gui` will spawn a `griffion-node` process to run P2P and RPC code, communicating with it across a socket pair, and `griffion-node` will spawn `griffion-wallet` to run wallet code, also communicating over a socket pair. This will let node, wallet, and GUI code run in separate address spaces for better isolation, and allow future improvements like being able to start and stop components independently on different machines and environments.
[#19460](https://github.com/GriffionProject/Griffion/pull/19460) also adds a new `griffion-node` `-ipcbind` option and an `griffiond-wallet` `-ipcconnect` option to allow new wallet processes to connect to an existing node process.
And [#19461](https://github.com/GriffionProject/Griffion/pull/19461) adds a new `griffion-gui` `-ipcconnect` option to allow new GUI processes to connect to an existing node process.
