inlib : A controller input library for SMS
==========================================

inlib is an input library for SMS, supporting standard controllers,
megadrive controllers, but also less common controllers such as the
Paddle, the Sports Pad, and even the Mega Mouse (a mouse for megadrive).

inlib is compiled using sdcc, but everything is coded in inline assembly.
Most of the code could be easily reused in pure assembly projects.

The calling convention for all inlib functions is `__z88dk_fastcall`.


Theory of operation
----------------------

inlib.h declares struct inlibDevice like this:

```
struct inlibDevice {
	unsigned char type;
	unsigned short buttons;
	unsigned short prev_buttons;
	union {
		struct inlib_16bit_relative_data rel16;
		struct inlib_relative_data rel;
		struct inlib_absolute_data abs;
		struct inlib_paddle_data paddle;
	};
};
```

There are two global instances of this structure, one for port A and one for port B,
namely inlib_portA and inlib_portB.

inlib provides functions to poll different kind of peripherals, and
those functions will update inlib_portA or inlib_portB with new data.

The inlibDevice structure has a type argument. Functions to read
controllers will always update this field. Existing types are:

 - INLIB_TYPE_NONE
 - INLIB_TYPE_SMS
 - INLIB_TYPE_MD3
 - INLIB_TYPE_MD6
 - INLIB_TYPE_PADDLE
 - INLIB_TYPE_SPORTSPAD
 - INLIB_TYPE_MDMOUSE
 - INLIB_TYPE_SPORTSPAD_MARKIII

SMS controllers cannot be detected (there is no difference between connected
and disconnected controllers), so `inlib_pollSMS(INLIB_PORTA)` will always
set the type to INLIB_TYPE_SMS.

However more complex controllers, such as Megadrive controllers, Paddles,
and the Mega Mouse, can be detected and the type field will therefore
be set according to the result.

For instance `inlib_readMDpad(INLIB_PORTA)` will try reading the status
of a MegaDrive controller in port A and will set the type to INLIB_TYPE_MD6
if a 6 Button controller was found, INLIB_TYPE_MD3 if it saw a 3 Button controller,
and otherwise will set the type to INLIB_TYPE_SMS.

The paddle and mega mouse functions will return the type PADDLE or MDMOUSE, but
if a timeout occurs due to a disconnection (no data from paddle or mouse handshake timeout)
the type will be set to INLIB_NONE. See inlib.h for specifics.

You can access the `buttons` member to check the current button status of the controllers, 
or use inlib_keysStatus(). This is equivalent to SMS_getKeysStatus() in SMSlib.
The button definitions are:

```
#define INLIB_BTN_UP       0x01
#define INLIB_BTN_DOWN     0x02
#define INLIB_BTN_LEFT     0x04
#define INLIB_BTN_RIGHT    0x08
#define INLIB_BTN_1        0x10 // Also MDpad B, Paddle button, mouse Left button
#define INLIB_BTN_2        0x20 // Also MDpad C, mouse right button
#define INLIB_BTN_MD_A     0x40 // Also mouse Middle butotn
#define INLIB_BTN_MD_START 0x80 // Also Mouse start button
#define INLIB_BTN_MD_Z     0x100
#define INLIB_BTN_MD_Y     0x200
#define INLIB_BTN_MD_X     0x400
#define INLIB_BTN_MD_MODE  0x800

// Extra definitions for convenience
#define INLIB_BTN_START    (INLIB_BTN_1)
#define INLIB_BTN_ANYDIR   (INLIB_BTN_UP|INLIB_BTN_DOWN|INLIB_BTN_LEFT|INLIB_BTN_RIGHT)
```

The `prev_buttons` member holds the state of the buttons the last time the port was
polled. Combined with buttons, button presses can be detected. inlib provides
a function to help doing this.

```
unsigned short inlib_keysPressed(unsigned char port) __z88dk_fastcall __naked;
```

Individual bits that are set in the returned value will represent the buttons that became
pressed just now, similar to SMS_getKeysPressed().

For controllers that are not only buttons (i.e. mouse, paddle, etc) the additional
values will be placed in one of the union members. You could look at the type member to
determine which one to access, but inlib also provides the following macros:

```
INLIB_ISGAMEPAD(type)    // Only buttons
INLIB_ISPADDLE(type)     // Paddle type (values in paddle.value)
INLIB_ISRELATIVE(type)   // Relative 8 bit signed (values in rel.x and rel.y)
INLIB_ISRELATIVE16(type) // Relative 16 bit signed (values in rel16.x and rel16.y)
INLIB_ISABSOLUTE(type)   // Absolute 8 bit position (values in abs.x and abs.y)
```

How to use in your project
--------------------------

You need to do two things.

1) Add the src directory to your include path so inlib.h
can be found when you #include "inlib." in your source code.

2) Pass inlib.lib when linking.

For instance, assuming inlib is in the parent directory of your game source
code like this:

```
somedir/yourgame/
somedir/inlib/
```

You should add the  `-I../inlib/src` command-line argument when compiling,
and when linking, simply pass ../inlib/inlib.lib after your object files.



Controller Auto detection
-------------------------

Inlib does not need to provide functions specifically for detection, since
the actual read functions for the supported type of controllers will set the
type field according to the result. See the table below:


| Type           | Timeouts?  | Read Function               | Detection        |
| ---------------|------------|-----------------------------|----------------- |
| SMS            | No         | inlib_pollSMS or inlib_readMDpad    | Not detectable   |
| MD3            | Yes        | inlib_readMDpad             | Check ->type     |
| MD6            | Yes        | inlib_readMDpad             | Check ->type     |
| Paddle         | Yes        | inlib_readPaddle            | Check ->type     |
| Sports Pad     | No         | inlib_readSportsPad         | Not perfect [1]  |
| Mega Mouse     | Yes        | inlib_readMDmouse           | Check ->type     |
| Sports Pad (mark III mode)    | Yes        | inlib_readSportsPad_markIII | Check ->type     |
| Light phaser   | Yes[2]     | inlib_pollLightPhaser_trigger and inlib_pollLightPhaser_position | With user cooperation[2] |


[2] The light phaser position read code will not hang even with defective light
phasers (for instance, one with a shorted cable which pulls TH low indefinitely).
The code timeouts at the end of the frame. The returned type will be 
INLIB_TYPE_PHASER or INLIB_TYPE_PHASER_HIT, depending on whether light was
sensed, but this cannot be used to detect the light phaser itself without
user cooperation. For instance, one could display something such as "shoot at 
the screen to play with a light phaser", wait for button 1 / trigger to be pushed,
and then, if a position is reported, conclude that a light phaser is present and
enable the corresponding game mode.

[1] The sports pad returns only zeros if not moving. It can be detected
at start by calling inlib_readSportsPad and checking that all values
are zero (no buttons and no X/Y motion).

The Paddle will send zeroes if set to its minimum position, but the difference
with a Sports Pad is that the paddle continuously toggles a button. If your game
must support Paddle AND Sports Pad, check for the Paddle first.

The Sports Pad in Mark III mode (TH low at startup) should be detected
before the Paddle. The difference is that the Sports Pad toggles both TL and TR,
but the Paddle only uses only TR.

Not that the sports pad needs some time to initialize. The Sega BIOS screen adds
more than enough, but on Japanese systems or systems with a modified BIOS which boots
directly, detection may be done too early. The best method to detect the sports
Pad is to wait 1 second after power up. After displaying your logo for instance.

If all supported devices are to be auto-detected by your game, here is a suggested
order that should work well:

1) Call inlib_readMDmouse. Type is a mouse? - > Done
2) Call inlib_readMDpad. Type is != SMS and != NONE? -> Done! (Controller is MD3 or MD6, see type)
3) Call inlib_readSportsPad_markIII. Type is mark III sportspad? Done
3) Call inlib_readPaddle. Type is Paddle? -> Done
4) Call inlib_readSportsPad. X/Y/Buttons are zero? -> Very likely a Sport Pad
5) Assume controller is a standard SMS pad.

See the example program for more information.


TODO
---------------------

Ideas and things to test, review, etc.

 - Test using a Sega Mouse (the mouse with two colored buttons)
 - The using the larger North American Sports Pad
 - Add an optimised function to read Megadrive controllers in both ports in parallel
 - The implementation is probably too careful about not touching the other
 port's bits in port 3F (see inlib_port3F_last and the related set of functions...)
 - Add Terebi Oekaki and/or drawing board support.


License
--------

This is free and unencumbered software released into the public domain. See the file UNLICENSE for details.

