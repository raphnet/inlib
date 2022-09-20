#include <stdio.h>
#include "SMSlib.h"
#include "inlib.h"

static const char *buttonNames[] = {
	"Up ", "Down ",
	"Left ", "Right ",
	"1 ", "2 ",
	"A ", "START ", "Z ", "Y ", "X ", "MODE ",
};

const char *getDeviceName(unsigned char type)
{
	switch (type)
	{
		case INLIB_TYPE_NONE: return "None/Error";
		case INLIB_TYPE_SMS: return "SMS";
		case INLIB_TYPE_MD3: return "MDPad 3";
		case INLIB_TYPE_MD6: return "MDPad 6";
		case INLIB_TYPE_PADDLE: return "Paddle";
		case INLIB_TYPE_SPORTSPAD: return "Sports Pad";
		case INLIB_TYPE_MDMOUSE: return "Mega Mouse";
		case INLIB_TYPE_SPORTSPAD_MARKIII: return "Sports Pad (m3)";
	}
	return "Unknown";
}

void autodetectall()
{
  char i;

  for (i=0; i<INLIB_PORTCOUNT; i++) {
	struct inlibDevice *d = inlib_getPortPtr(i);

	inlib_readMDmouse(i);
	if (d->type == INLIB_TYPE_MDMOUSE) {
		continue; // found a mouse
	}

	inlib_readMDpad(i);
	if ((d->type != INLIB_TYPE_SMS) && (d->type != INLIB_TYPE_NONE) ) {
		continue; // found a MD pad, 3 or 6 buttons
	}

	inlib_readSportsPad_markIII(i);
	if (d->type == INLIB_TYPE_SPORTSPAD_MARKIII) {
		continue; // found a Paddle
	}

	inlib_readPaddle(i);
	if (d->type == INLIB_TYPE_PADDLE) {
		continue; // found a Paddle
	}

	inlib_readSportsPad(i);
	if ((d->buttons == 0) && (d->rel.x == 0) && (d->rel.y == 0)) {
		// most likely found a Sports Pad
		continue;
	}

	// inlib_readSportsPad sets the type to SportsPad. Reset it to none.
	d->type = INLIB_TYPE_NONE;
  }
}


void main(void)
{
  char i, j, y;

  /* Position and visiblity of mouse pointers */
  unsigned char px[2] = { 64, 168 };
  unsigned char py[2] = { 96, 96 };
  char pvisible[2] = { 0, 0 };
  unsigned char prev_x[2];
  unsigned char prev_y[2];

  /* Clear VRAM */
  SMS_VRAMmemsetW(0, 0x0000, 16384);

  inlib_init();

  /* load a standard font character set into tiles 0-95,
   * set BG palette to B/W and turn on the screen */
  SMS_autoSetUpTextRenderer();

  /* Tiles for numbers 1 and 2 are used as sprites for mouse
   * pointers. Configure palette since the above does not
   * set a sprite palette... */
  SMS_setSpritePaletteColor(1, RGB(3,3,0));

  /* We are using tiles from the font (index 17 and 18)
   * which are in the first 256 tiles. */
  SMS_useFirstHalfTilesforSprites(1);

  /* Set the target of the next background write */
  SMS_setNextTileatXY(1,0);

  /* Write text to the background */
  printf("inlib test");

  /* Turn on the display */
  SMS_displayOn();

  SMS_setNextTileatXY(1,3);
  printf("Pre-autodetect pause (1 s)...");
  for (i=0; i<60; i++) {
	  SMS_waitForVBlank();
  }
  autodetectall();
  SMS_setNextTileatXY(1,3);
  printf("                             ");

  /* Do nothing */
  for(;;) {

	SMS_waitForVBlank();
	SMS_copySpritestoSAT();
	SMS_initSprites();

	for (i=0; i<2; i++) {
		struct inlibDevice *d = inlib_getPortPtr(i);

		y = 6 + i * 8;

		if (d->type == INLIB_TYPE_SPORTSPAD) {
			// If a Sports Pad was detected at start, stick with it
			inlib_readSportsPad(i);
		}
		else {
			// Otherwise try everything else for hotswapability
			inlib_readSportsPad_markIII(i);
			if (d->type == INLIB_TYPE_NONE) {
				inlib_readMDmouse(i);
				if (d->type == INLIB_TYPE_NONE) {
					inlib_readPaddle(i);
					if (d->type == INLIB_TYPE_NONE) {
						inlib_readMDpad(i);
					}
				}
			}
		}

		SMS_setNextTileatXY(1,y++);
		printf("Port %d : $%02x (%s)          ", i+1, d->type, getDeviceName(d->type));
		SMS_setNextTileatXY(1,y++);
		printf("  Button bits : $%04x ", d->buttons);
		SMS_setNextTileatXY(3,y++);
		printf("  Keys pressed: $%04x", inlib_keysPressed(i));
		SMS_setNextTileatXY(3,y++);
		for (j=0; j<15; j++) {
			if (d->buttons & (1<<j)) {
				printf(buttonNames[j]);
			}
		}
		printf("                   ");

		SMS_setNextTileatXY(3,y++);
		pvisible[i] = 0;
		if (INLIB_ISPADDLE(d->type)) {
			printf("Paddle: %02x", d->paddle.value);
		} else if (INLIB_ISRELATIVE(d->type)) {
			printf("Relative: %d,%d      ", d->rel.x, d->rel.y);
		    pvisible[i] = 1;
			px[i] += d->rel.x;
			py[i] += d->rel.y;
		} else if (INLIB_ISRELATIVE16(d->type)) {
			printf("Relative 16-bit: %d,%d      ", d->rel16.x, d->rel16.y);
		    pvisible[i] = 1;
			px[i] += d->rel16.x;
			py[i] += d->rel16.y;
		} else if (INLIB_ISABSOLUTE(d->type)) {
			printf("Absolute: %3d,%3d      ", d->abs.x, d->abs.y);
		    pvisible[i] = 1;
			px[i] += d->abs.x - prev_x[i];
			py[i] += d->abs.y - prev_y[i];
			prev_x[i] = d->abs.x;
			prev_y[i] = d->abs.y;
		}

		if (pvisible[i]) {
			SMS_addSprite(px[i], py[i], 16 + i);
			SMS_setNextTileatXY(3,y++);
			printf("Mouse: %03u,%03u", px[i], py[i]);
		}
    }
  }
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"raphnet","inlib demo","A simple controller test using inlib");
