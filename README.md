Aether
======

Intelligent, wireless plant monitor system.

Sensor
------

Data is gathered by a remote sensor 'mote'.  This is an Arduino compatible
board with a Ciseco SRF radio module and a selection of sensors.

The firmware for the sensor mote is built using a simple Makefile (the process
is quite specific to AVR code making CMake a little cumbersome).  To build the
firmware, cd to 'sensor', copy Makefile.config.example to Makefile.config, edit
the configuration variables as required and run 'make'.  The firmware can be
uploaded to the board with 'make upload' and serial output monitored with 'make
monitor'.

