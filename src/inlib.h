#ifndef _inlib_h__
#define _inlib_h__

#define INLIB_TYPE_NONE      0x00
#define INLIB_TYPE_SMS       0x01
#define INLIB_TYPE_MD3	     0x02
#define INLIB_TYPE_MD6	     0x03
#define INLIB_TYPE_PADDLE    0x10
#define INLIB_TYPE_GRAPHIC_BOARD 0x11
#define INLIB_TYPE_SPORTSPAD 0x20
#define INLIB_TYPE_MDMOUSE   0x40
#define INLIB_TYPE_SPORTSPAD_MARKIII 0x80

// The light phaser is a bit special - the type
// changes depending on if light was detected
// or not. Absolute coordinates are present only if
// the type is INLIB_TYPE_PHASER_HIT
#define INLIB_TYPE_PHASER      0x04
#define INLIB_TYPE_PHASER_HIT  0x81

// Macros to check device types
#define INLIB_ISGAMEPAD(type)  (!(type & 0xC0))
#define INLIB_ISPADDLE(type) (type == INLIB_TYPE_PADDLE)
#define INLIB_ISGRAPHICBOARD(type) (type == INLIB_GRAPHIC_BOARD)
#define INLIB_ISRELATIVE(type) (type & 0x20)
#define INLIB_ISRELATIVE16(type) (type & 0x40)
#define INLIB_ISABSOLUTE(type) (type & 0x80)

struct inlib_graphicboard_data {
  unsigned char pressure;
  unsigned char x;
  unsigned char y;
};

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
    struct inlib_graphicboard_data graph;
	};
};

#define INLIB_BTN_UP       0x01
#define INLIB_BTN_DOWN     0x02
#define INLIB_BTN_LEFT     0x04
#define INLIB_BTN_RIGHT    0x08
#define INLIB_BTN_1        0x10 // Also MDpad B, Paddle button, mouse Left button, phase trigger
#define INLIB_BTN_START    (INLIB_BTN_1)
#define INLIB_BTN_2        0x20 // Also MDpad C, mouse right button
#define INLIB_BTN_MD_A     0x40 // Also mouse Middle butotn
#define INLIB_BTN_MD_START 0x80 // Also Mouse start button
#define INLIB_BTN_MD_Z     0x100
#define INLIB_BTN_MD_Y     0x200
#define INLIB_BTN_MD_X     0x400
#define INLIB_BTN_MD_MODE  0x800

// When active, it means the pen is touching the board
#define INLIB_BTN_GRAF_PEN_DOWN  INLIB_BTN_1
// Mode Select Switches (order needs to be confirmed)
#define INLIB_BTN_GRAF_1         INLIB_BTN_2
#define INLIB_BTN_GRAF_2         INLIB_BTN_MD_A
#define INLIB_BTN_GRAF_3         INLIB_BTN_START

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

// Returns the current status of the button bits for the specified ports.
//
// Bit field of INLIB_BTN_*
unsigned short inlib_keysStatus(unsigned char port) __z88dk_fastcall __naked;

// Looks at current a previous buttons bits for specified port and returns
// bits set for newly pressed buttons (button down events).
//
// Bit field of INLIB_BTN_*
unsigned short inlib_keysPressed(unsigned char port) __z88dk_fastcall __naked;


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

// Read the light phaser button status. This is equivalent to reading a SMS
// controller, but only Button 1 exists and the type will always be set
// to INLIB_TYPE_PHASER.
//
// This function, unlike inlib_pollLightPhaser_frame() returns very quickly.
void inlib_pollLightPhaser_trigger(unsigned char port) __naked __z88dk_fastcall;

// Read the light phaser position. Due to how this works, this function runs
// for a full frame.
//
// You should call it before Vblank ends and it will return after the end
// of active display. In many games this will consume too much time to fully run
// a frame of logic, so you may want to only call this function only once after
// inlib_pollLightPhaser_trigger reports that the trigger was been pulled.
//
// If light is seen, the type will be set to INLIB_TYPE_PHASER_HIT and in addition to
// the trigger status in the button member, the sensed screen coordinates will be stored
// in abs.x/abs.y. Otherwise the type will be set to INLIB_TYPE_PHASER and only the trigger
// status will be updated.
//
// Depending on your game, before calling this you may want to brigten the colors or draw a
// solid box over the target to make sure it is detected reliably by the light gun.
void inlib_pollLightPhaser_position(unsigned char port) __naked __z88dk_fastcall;

// Reads the status of a Graphic Board.
//
// When no Graphic board is present, or a SMS controller is connected,
// The X,Y coordinates read as 0xFF. In this case, this function
// returns INLIB_TYPE_NONE as this is deemed impossible on real hardware,
// but this simple test is far from perfect. The Sports Pad, Paddles
// (except when reading 0xFF) and SMS controllers (when buttons are pressed)
// will all get past this simple test.
//
// The pressure returned in the graph->pressure member is typically in the
// 0xFD to 0xFF range when the pen is touching the surface. Also, when the pen
// is touching the surface, the INLIB_BTN_1 / INLIB_BTN_GRAF_PEN_DOWN bit will
// be set in the button status.
//
// The "Mode Select" buttons are defined as INLIB_BTN_GRAF_1/2/3 but I am
// uncertain if they match the order of their physical counterparts.
void inlib_readGraphicBoard(unsigned char port) __z88dk_fastcall __naked;

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
