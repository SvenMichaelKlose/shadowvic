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


# Building

shadowVIC uses the GNU autotools to get built and installed.
Accordingly, you need to have autoconf and automake installed.

```
sudo aptitude install automake autoconf
automake --add-missing
autoconf
./configure
make
```

The make script generates picovic, which is shadowVIC trying to
boot BASIC as well as the VIC-20 game pulse, which you can play
with a joystick.

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

Please note that you might have to run shadowVIC as root for
it to be able to access the framebuffer device.


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
the output with FRAMES_PER_SECOND.
