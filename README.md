Microde 0.1
=============

Microde is an IDE for programming AVR microcontrollers written in C and GTK+.
It depends on additional softwares such as avr-gcc and avrdude to do the
compiling and uploading of code.

Prequisites
-----------
* autotools (autoreconf, autoconf, etc..)
* pkg-config
* gtk+-2.0
* gtksourceview-2.0
* avr-binutils
* avr-gcc
* avrdude

Installation
------------
	sh autogen.sh
	./configure && make
	make install
