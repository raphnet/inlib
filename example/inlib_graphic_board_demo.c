#include <stdio.h>
#include "SMSlib.h"
#include "inlib.h"

static const char *buttonNames[] = {
	"PEN ", "1   ",
	"2   ", "3   ",
};


#define DEFAULT_BG_COLOR  RGB(1,2,3)
#define DEFAULT_TEXT_COLOR  RGB(0,0,0)
#define HIGH_BG_COLOR RGB(3,3,3)

void main(void)
{
  char i, y;
  char bright_bg = 0;
  char controllerPort = 99;
  struct inlibDevice *device;

  /* Position and visiblity of mouse pointers */
  unsigned char px[2] = { 64, 168 };
  unsigned char py[2] = { 96, 96 };
  char pvisible[2] = { 0, 0 };

  /* Clear VRAM */
  SMS_VRAMmemsetW(0, 0x0000, 16384);

  inlib_init();

  /* load a standard font character set into tiles 0-95,
   * set BG palette to B/W and turn on the screen */
  SMS_autoSetUpTextRenderer();

  /* For this light gun demo, modify the palette to display black
   * on light grey */
  SMS_setBGPaletteColor(0, DEFAULT_BG_COLOR);
  SMS_setBGPaletteColor(1, DEFAULT_TEXT_COLOR);

  /* Tiles for numbers 1 and 2 are used as sprites for mouse
   * pointers. Configure palette since the above does not
   * set a sprite palette... */
  SMS_setSpritePaletteColor(1, RGB(0,0,3));

  /* We are using tiles from the font (index 17 and 18)
   * which are in the first 256 tiles. */
  SMS_useFirstHalfTilesforSprites(1);

  /* Set the target of the next background write */
  SMS_setNextTileatXY(1,0);

  /* Write text to the background */
  printf("inlib Graphic Board test");

  /* Turn on the display */
  SMS_displayOn();

  SMS_setNextTileatXY(1, 2);
  printf("Press a button to begin");

  while(controllerPort == 99) {
    for (i=0; i<2; i++) {
      inlib_readGraphicBoard(i);
      if (inlib_getPortPtr(i)->type == INLIB_TYPE_GRAPHIC_BOARD) {
        if (inlib_keysStatus(i) & (INLIB_BTN_GRAF_1 | INLIB_BTN_GRAF_2 | INLIB_BTN_GRAF_3)) {
          controllerPort = i;
          break;
        }
      }
    }
  }

  SMS_setNextTileatXY(1, 2);
  printf("Using graphic board in port %d", controllerPort);

  device = inlib_getPortPtr(controllerPort);

  for(;;) {
    SMS_waitForVBlank();
    SMS_copySpritestoSAT();
    SMS_initSprites();

    if (!bright_bg) {
        SMS_setBGPaletteColor(0, DEFAULT_BG_COLOR);
    }

    bright_bg = 0;
    y = 6;

    inlib_readGraphicBoard(controllerPort);

    SMS_setNextTileatXY(3,y++);
    printf("Buttons: 0x%02x", device->buttons);
    SMS_setNextTileatXY(3,y++);
    for (i=0; i<4; i++) {
      if (device->buttons & (INLIB_BTN_1 << i)) {
        printf(buttonNames[i]);
      } else {
        printf("    ");
      }
    }


    if (device->type == INLIB_TYPE_GRAPHIC_BOARD) {
      SMS_setNextTileatXY(3,y++);
      printf("Pressure: %02x", device->graph.pressure); 
      SMS_setNextTileatXY(3,y++);
      printf("Position: %02x,%02x", device->graph.x, device->graph.y); 

      if (device->graph.pressure >= 0xFD) {
        px[controllerPort] = device->graph.x;
        py[controllerPort] = device->graph.y;
        pvisible[controllerPort] = 1;
      }
    }
   
    if (pvisible[controllerPort]) {
      SMS_addSprite(px[controllerPort], py[controllerPort], 16 + controllerPort);
      SMS_setNextTileatXY(3,y++);
    }

  }
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"raphnet","inlib demo","A simple Graphic Board test using inlib");
