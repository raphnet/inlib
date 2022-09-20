#ifndef _inlib_h__
#define _inlib_h__

#define INLIB_TYPE_NONE      0x00
#define INLIB_TYPE_SMS       0x01
#define INLIB_TYPE_MD3	     0x02
#define INLIB_TYPE_MD6	     0x03
#define INLIB_TYPE_PADDLE    0x10
#define INLIB_TYPE_SPORTSPAD 0x20
#define INLIB_TYPE_MDMOUSE   0x40
#define INLIB_TYPE_SPORTSPAD_MARKIII 0x80

// Macros to check device types
#define INLIB_ISGAMEPAD(type)  (!(type & 0xC0))
#define INLIB_ISPADDLE(type) (type & 0x10)
#define INLIB_ISRELATIVE(type) (type & 0x20)
#define INLIB_ISRELATIVE16(type) (type & 0x40)
#define INLIB_ISABSOLUTE(type) (type & 0x80)

struct inlib_absolute_data {
	unsigned char x, y;
};

struct inlib_relative_data {
	signed char x, y;
};

struct inlib_16bit_relative_data {
	signed short x, y;
};

struct inlib_paddle_data {
	unsigned char value;
};

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

#define INLIB_BTN_UP       0x01
#define INLIB_BTN_DOWN     0x02
#define INLIB_BTN_LEFT     0x04
#define INLIB_BTN_RIGHT    0x08
#define INLIB_BTN_1        0x10 // Also MDpad B, Paddle button, mouse Left button
#define INLIB_BTN_START    (INLIB_BTN_1)
#define INLIB_BTN_2        0x20 // Also MDpad C, mouse right button
#define INLIB_BTN_MD_A     0x40 // Also mouse Middle butotn
#define INLIB_BTN_MD_START 0x80 // Also Mouse start button
#define INLIB_BTN_MD_Z     0x100
#define INLIB_BTN_MD_Y     0x200
#define INLIB_BTN_MD_X     0x400
#define INLIB_BTN_MD_MODE  0x800

#define INLIB_BTN_ANYDIR   (INLIB_BTN_UP|INLIB_BTN_DOWN|INLIB_BTN_LEFT|INLIB_BTN_RIGHT)

// Initialize library internals
void inlib_init(void);

// All read functions update inlib_portA or inlib_portB according to the port argument
// and set the type field depending on the outcome.
#define INLIB_PORTCOUNT	2

// Those can be used for the port argument of most functions.
#define INLIB_PORTA	0
#define INLIB_PORTB	1

// Convenience macro to get a pointer to one of the ports
#define inlib_getPortPtr(id) ((id) ? &inlib_portB : &inlib_portA)

extern volatile struct inlibDevice inlib_portA;
extern volatile struct inlibDevice inlib_portB;

// Looks at current a previous buttons bits for specified port and returns
// bits set for newly pressed buttons (button down events).
//
// Bit field of INLIB_BTN_*
unsigned short inlib_keysPressed(unsigned char port) __z88dk_fastcall __naked;


/* Type           | Timeouts?  | Read Function               | Detection
 * ---------------+------------+-----------------------------+-----------------
 * SMS            | No         | inlib_pollSMS or            | Not detectable
 *                |            | inlib_readMDpad             |
 * ---------------+------------+-----------------------------+-----------------
 * MD3            | Yes        | inlib_readMDpad             | Check ->type
 * ---------------+------------+-----------------------------+-----------------
 * MD6            | Yes        | inlib_readMDpad             | Check ->type
 * ---------------+------------+-----------------------------+-----------------
 * Paddle         | Yes        | inlib_readPaddle            | Check ->type
 * ---------------+------------+-----------------------------+-----------------
 * Sports Pad     | No         | inlib_readSportsPad         | Not perfect [1]
 * ---------------+------------+-----------------------------+-----------------
 * Mega Mouse     | Yes        | inlib_readMDmouse           | Check ->type
 * ---------------+------------+-----------------------------+-----------------
 * Sports Pad     | Yes        | inlib_readSportsPad_markIII | Check ->type
 * (mark III mode)|            |                             |
 * ---------------+------------+-----------------------------+-----------------
 *
 * [1] The sports pad returns only zeros if not moving. It can be detected
 * at start by calling inlib_readSportsPad and checking that all values
 * are zero (no buttons and no X/Y motion).
 *
 * The Paddle will send zeroes if set to its minimum position, but the difference
 * with a Sports Pad is that the paddle continuously toggles a button. If your game
 * must support Paddle AND Sports Pad, check for the Paddle first.
 *
 * The Sports Pad in Mark III mode (TH low at startup) should be detected
 * before the Paddle. The difference is that the Sports Pad toggles both TL and TR,
 * but the Paddle only uses only TR.
 *
 * Not that the sports pad needs some time to initialize. The Sega BIOS screen adds
 * more than enough, but on Japanese systems or systems with a modified BIOS which boots
 * directly, detection may be done too early. The best method to detect the sports
 * Pad is to wait 1 second after power up. After displaying your logo for instance.
 *
 * If all supported devices are to be auto-detected by your game, here is a suggested
 * order that should work well:
 *
 * 1) Call inlib_readMDmouse. Type is a mouse? - > Done
 * 2) Call inlib_readMDpad. Type is != SMS and != NONE? -> Done! (Controller is MD3 or MD6, see type)
 * 3) Call inlib_readSportsPad_markIII. Type is mark III sportspad? Done
 * 3) Call inlib_readPaddle. Type is Paddle? -> Done
 * 4) Call inlib_readSportsPad. X/Y/Buttons are zero? -> Very likely a Sport Pad
 * 5) Assume controller is a standard SMS pad.
 *
 * See the example program for more information.
 */


// Reads a standard SMS gamepad. Cannot fail.
void inlib_pollSMS(unsigned char port) __naked __z88dk_fastcall;

// Sets the type field to INLIB_TYPE_NONE on timeout
void inlib_readPaddle (unsigned char port) __z88dk_fastcall __naked;

// Hotswapping SMS, 3 Button and 6 Button controllers will work,
// the type field will change accordingly.
void inlib_readMDpad(unsigned char port) __naked __z88dk_fastcall;

// Read a Sports Pad. Cannot fail / detect disconnection.
void inlib_readSportsPad(unsigned char port) __z88dk_fastcall __naked;

// Reads a Mega Mouse. Sets the type to INLIB_TYPE_NONE on timeout
void inlib_readMDmouse(unsigned char port) __naked __z88dk_fastcall;

// Read a Sports Pad in mark III mode. This will only work on mark III,
// or on Japanese SMS if the game quickly sets TH low at startup.
void inlib_readSportsPad_markIII(unsigned char port) __naked __z88dk_fastcall;

/* From this point, mostly internal functions and vars */

// Returns port A raw value (bits 0-6 from port DC)
void inlib_getportA(void) __naked __z88dk_fastcall __preserves_regs(b,c,d,e,iyl,iyh);

// Returns port B raw value, bit aligned as for port A
// (i.e. combines bits 6-7 of port DC with bits 0-3 of port DC, then rotate left 2 times)
void inlib_getportB(void) __naked __z88dk_fastcall __preserves_regs(b,c,d,e,iyl,iyh);

// Functions to configure TH and TR in particular ways. Use inlib_port3F_last to avoid
// side effects.
void inlib_portA_TH_in_TR_in(void) __naked __z88dk_fastcall;
void inlib_portB_TH_in_TR_in(void) __naked __z88dk_fastcall;
void inlib_portA_TH_out_TR_in(void) __naked __z88dk_fastcall;
void inlib_portB_TH_out_TR_in(void) __naked __z88dk_fastcall;
void inlib_portA_TH_out_TR_out(void) __naked __z88dk_fastcall;
void inlib_portB_TH_out_TR_out(void) __naked __z88dk_fastcall;
void inlib_portA_TH_low(void) __naked __z88dk_fastcall;
void inlib_portA_TH_high(void) __naked __z88dk_fastcall;
void inlib_portB_TH_low(void) __naked __z88dk_fastcall;
void inlib_portB_TH_high(void) __naked __z88dk_fastcall;
void inlib_portA_TR_low(void) __naked __z88dk_fastcall;
void inlib_portA_TR_high(void) __naked __z88dk_fastcall;
void inlib_portB_TR_low(void) __naked __z88dk_fastcall;
void inlib_portB_TR_high(void) __naked __z88dk_fastcall;

// Holds the last value written to port 3F. Used internally to avoid resetting direction
// configuration (e.g: For instance, mouse code configures TR as an output on port A, if reading
// a Megadrive controller on port B it should not change this accidentally when toggling
// TH...
extern unsigned char inlib_port3F_last;

#endif // _inlib_h__
