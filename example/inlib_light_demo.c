#include <stdio.h>
#include "SMSlib.h"
#include "inlib.h"

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
  printf("inlib light phaser test");

  /* Turn on the display */
  SMS_displayOn();

  SMS_setNextTileatXY(1, 2);
  printf("Pull trigger to begin...");

  while(controllerPort == 99) {

    for (i=0; i<2; i++) {
      inlib_pollLightPhaser_trigger(i);
      if (inlib_keysStatus(i) & INLIB_BTN_1) {
        controllerPort = i;
        break;
      }
    }
  }

  SMS_setNextTileatXY(1, 2);
  printf("Using light phaser in port %d", controllerPort);

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

    inlib_pollLightPhaser_trigger(controllerPort);
    if (inlib_keysStatus(controllerPort) & INLIB_BTN_1) {
      bright_bg = 1;
      SMS_setBGPaletteColor(0, HIGH_BG_COLOR);
      SMS_waitForVBlank();
      inlib_pollLightPhaser_position(i);
    }

    SMS_setNextTileatXY(3,y++);
    if (inlib_keysStatus(controllerPort) & INLIB_BTN_1) {
      printf("Trigger ON ");
    } else {
      printf("Trigger OFF");
    }


    if (device->type == INLIB_TYPE_PHASER_HIT) {

      // Center sprite 8x8 sprite on the position
      if (device->abs.x > 4) { px[i] = device->abs.x - 4; } else { px[i] = 0; }
      if (device->abs.y > 4) { py[i] = device->abs.y - 4; } else { py[i] = 0; }

      pvisible[i] = 1;
    }

    if (pvisible[i]) {
      SMS_addSprite(px[i], py[i], 16 + i);
      SMS_setNextTileatXY(3,y++);
      printf("Position: %03u,%03u", px[i], py[i]);
    }

  }
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"raphnet","inlib demo","A simple controller test using inlib");
