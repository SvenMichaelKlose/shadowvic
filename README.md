# shadowVIC

This is a minimalistic Commodore VIC-20 emulator library for
Linux with framebuffer device.  Other Unices have not been
tested, yet.

You can either use shadowVIC on low–performance devices as a
replacement for other emulators, which are too slow, or to
basically retain the original program cores and upgrading them
with better graphics and sound.


# HELP!!!

You can help out with these things or by telling
how to implement them (please!):

* Keyboard emulation that stops BASIC from hanging.
* 6560/6561 reverse mode.
* Basic VIA interrupts.


# Contributors

* Sven Michael Klose <pixel@hugbox.org>
* Eric Hilaire


# Building

shadowVIC uses the GNU autotools to get built and installed.
Accordingly, you need to have autoconf and automake installed.

```
sudo aptitude install automake autoconf libtool
libtoolize
autoreconf -i
automake --add-missing
./configure
make
```

To get maximum performance out of shadowVIC with gcc, you should
specifiy the following options:

```
./configure CFLAGS="-O3 -flto"
```

shadowVIC disassembles the CPU instructions it executes, if you
configure it like this:
```
./configure CPPFLAGS="-DDISASSEMBLE"
```

Dumping the VIC 6560/6561 settings on each screen update might
also be essential:
```
./configure CPPFLAGS="-DDISASSEMBLE -DDEBUG_VIC"
```

# Installing

After building shadowVIC you can install it by typing:

```
sudo make install
```


## What is installed?

By default the shared and static libraries, headers and binaries
are installed to /usr/local.  The binaries installed are:

* picovic which is shadowVIC trying to boot BASIC,
* pulse, a horizontally scrolling shoot-them-up which you can play with a joystick.

Please note that you might have to run shadowVIC programs as root
for them to be able to access the framebuffer device.  If you're
wondering why you don't see anything: you have to switch to the
console on which the framebuffer is displayed.  Most likely that
is done by pressing Ctrl, Alt and some function key.


## What is emulated?

shadowVIC emulates:

* 6502 CPU without BCD math or undocumented instructions
* most basic 6560/6561 graphics display
* VIA joystick status

It emulates absolutely nothing else.  No keyboard, no sound,
nothing.


### Raster line counter

The high bits of this counter are incremented every 23
instructions as a source of pseudo–random numbers and to keep
loops that wait for certain values from blocking.


### Screen updates

The screen is updated every bunch of instructions or if an
illegal instruction tells the emulator to do so.  That way you
can avoid slowdowns.  See the next section for that.


# Escape codes

The illegal opcode $22 (which would jam the CPU) signals an
emulator call.  It is followed by a byte ID and optional
parameters.


## Test

```
$22 $00 ASCIZ-string accu x y 
```

First, the ASCIIZ string is printed to the console.  Then, the
accumulater and X and Y registers are compared with the
parameters provided and the emulator is halted if they don't
match.


## Exit

```
$22 $01
```

Exits the emulator.


## Screen update

```
$22 $02
```

Makes the emulator update the display and waits to sync
the output with the frames per second specified.


## Memory dump

```
$22 $03 <from address> <to address>
```

Dumps the specified address range like hexdump(1).


# Debugger

The debugger is invoked via Ctrl+C.  It is under construction
and can only exit shadowVIC or print memory dumps.  Enter 'h'
in the debugger to get a command overview.
