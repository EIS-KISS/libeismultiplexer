# libeismultiplexer

libeismultiplexer is a shared library that allows you to control EISmulitplexer devices.

For questions or comments, please write to klemm@rhd-instruments.de

Full Online API documentation can be built with the "doc" target and is also avialable [here](http://uvos.xyz/kiss/libeismultiplexer).
A PDF with API documentation can be found [here](http://uvos.xyz/kiss/libeismultiplexer.pdf).

## Compile/Install

### Requirements

* git
* c11 capable compiler (e.g. GCC, CLANG)
* cmake 3.0 or later
* libusb 1.0 or later
* (optional) doxygen 1.8 or later to generate the documentation

### Procedure

In a console do:

* git clone https://git-ce.rwth-aachen.de/carl_philipp.klemm/libeismultiplexer.git
* cd librelaxisloader
* mkdir build
* cd build
* cmake ..
* make
* sudo make install

to make the documentation:

* make doc

### Licence

librelaxisloader is licenced to you under the Apache License, Version 2.0
