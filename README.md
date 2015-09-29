# shadowVIC

This is a minimalistic Commodore VIC-20 emulator that is
compiled with the native binaries it runs.

You can either use shadowVIC on low–performance devices as a
replacement for other emulators, which are too slow, or to
basically retain the original program cores and upgrading it
with better graphics and sound.


# HELP!!!

You can help out with these things or by telling
how to implement them (please!):

* A proper make script with library target.
* Keyboard emulation that stops BASIC from hanging.
* 6560/6561 reverse mode.
* Basic VIA interrupts.


# Contributors

* Sven Michael Klose <pixel@hugbox.org>


# Building

If your framebuffer device is set to 32 bits per pixel, you
need to tell shadowVIC:


```
./make.sh -DHAVE_32_BPP
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
