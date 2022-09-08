/* Code by Raphael Assenat [raphnet] */
#include "inlib.h"

__sfr __at 0x3F IOPortCtrl;

#pragma disable_warning 85

/*
 * The Sports Pad (at least the Japanese model SP-500)
 * will send its status continuously in the following format
 * if TH is low at startup. This as probably intended for Mark III
 * compatiblity, since the Mark III has pin 7 (TH) connected
 * to GND according to [1].
 *
 * On Japanese SMS systems, where the BIOS quickly lets
 * the game sofware run, it is possible to set TH to an output low
 * state in time to use the Sports Pad in this mode, however on
 * export systems with logos, it is too late and the handshake mode
 * must be used...
 *
 * In this format, the Sports Pad returns 5 nibbles.
 *
 * X high, X low, Y high, Y low, Buttons
 *
 * The X/Y values are a 8-bit position (i.e. usable directly
 * to set the pointer sprite position on screen). Those are NOT
 * signed values reprenting movement since the last read, which
 * would make no sense here given that the Sports Pad is continuously
 * talking and has not way to know which "packet" was actually
 * taken into account by the console.
 *
 * Here is what the Sports Pad output looks like
 *
 *    Nibble:           1     2     3     4      5
 *         -->| 50us |<------  190 us ------->|
 *             ______                          ____
 * TR ________|      |________________________|
 *
 *                -->| 39us|<--
 *    _______________       _____       ___________
 * TL                |_____|     |_____|
 *    _______________ _____ _____ _____ _____ _____
 * D3 _______________X_____X_____X_____X_____X_____
 *    _____________ _____ _____ _____ _____ _____
 * D2 _____________X_____X_____X_____X_____X_____
 *    _____________ _____ _____ _____ _____ _____
 * D1 _____________X_____X_____X_____X_____X_____
 *    _____________ _____ _____ _____ _____ _____
 * D0 _____________X_____X_____X_____X_____X_____
 *
 * Notice how D0-D2 change before D3, and D3 changes in sync with
 * TL. When polling TL for a change, D0-D3 must be resampled at least
 * once to avoid occasional pointer jumps (yes, it happens on real hardware)
 *
 * [1] https://www.smspower.org/Development/PeripheralPorts
 */

void inlib_readSportsPad_markIII(unsigned char port) __z88dk_fastcall __naked
{
  __asm

  ld a, l
  or a
  jp NZ, read_m3_sportspad_B

read_m3_sportspad_A:
  ld bc, (#_inlib_portA + 1) ; Read current button bits
  ld (#_inlib_portA + 3), bc ; Copy to previous button bits

  ld d, #0xff           ; timeout value

  ;;; Synchronize
  ld b, d
1$:                     ; Wait TR high
  dec b
  jp Z, $99             ; timeout
  in a,(#0xDC)
  bit 5,a
  jp Z, 1$


  ;;; read nibble 1 (X upper bits)
  ld b, d
$3:                     ; Wait TL low
  dec b
  jp Z, $99             ; timeout
  in a,(#0xDC)
  bit 4,a
  jp NZ, $3

  ; TL and D3 (right) fall at the same time, whereas
  ; D0, D1 and D2 fall a bit before TL. Repoll to make sure
  ; we get a stable D3 value.
  in a,(#0xDC)

  add a,a               ; Move nibble to bits 7-3
  add a,a
  add a,a
  add a,a
  ld c,a                ; Save it in C for later

  ;;; read nibble 2 (X lower bits)
  ld b, d
$4:                     ; Wait TL High
  dec b
  jp Z, $99             ; timeout
  in a,(#0xDC)
  bit 4,a
  jp Z, $4
  in a,(#0xDC)          ; repoll

  and a, #0x0f          ; Keep only low 4 bits
  or a,c                ; Combine with previous nibble

  ld (_inlib_portA + 5), a

  ;;; read nibble 3 (Y upper bits)
  ld b, d
$5:                     ; Wait TL low
  dec b
  jp Z, $99             ; timeout
  in a,(#0xDC)
  bit 4,a
  jp NZ, $5
  in a,(#0xDC)          ; repoll

  add a,a
  add a,a
  add a,a
  add a,a
  ld c,a

  ;;; read nibble 4 (Y lower bits)
  ld b, d
$6: ; Wait TL high
  dec b
  jp Z, $99             ; timeout
  in a,(#0xDC)
  bit 4,a
  jp Z, $6
  in a,(#0xDC)          ; repoll

  and a, #0x0f

  or a,c
  ld (_inlib_portA + 6), a

  ;;; read nibble 5 (Buttons)
  ld b, d
$7:                     ; Wait TR high
  dec b
  jp Z, $99             ; timeout
  in a,(#0xDC)
  bit 5,a
  jp Z, $7
  in a,(#0xDC)          ; repoll

  and a, #0x03          ; Only keep used bits (buttons 1/2)
  xor a, #0x03          ; Invert buttons
  rlca                  ; Align buttons to bits 4 and 5
  rlca
  rlca
  rlca
  ld (_inlib_portA + 1), a
  xor a                 ; Clear button high byte
  ld (_inlib_portA + 2), a

  ld a, #INLIB_TYPE_SPORTSPAD_MARKIII
  ld (_inlib_portA), a
  ret

$99:
  ld a, #INLIB_TYPE_NONE
  ld (_inlib_portA), a
  ret


read_m3_sportspad_B:
  ld bc, (#_inlib_portB + 1) ; Read current button bits
  ld (#_inlib_portB + 3), bc ; Copy to previous button bits

  ld d, #0xff           ; timeout value

  ;;; Synchronize
  ld b, d
8$:                     ; Wait TR high
  dec b
  jp Z, $98             ; timeout
  in a,(#0xDD)
  ;bit 3,a
  and a, #0x08
  jp Z, 8$


  ;;; read nibble 1 (X upper bits)
  ld b, d
$10:                    ; Wait TL low
  dec b
  jp Z, $98             ; timeout
  in a,(#0xDD)
  bit 2,a
  jp NZ, $10

  ; TL and D3 (right) fall at the same time, whereas
  ; D0, D1 and D2 fall a bit before TL. Repoll to make sure
  ; we get a stable D3 value.
  in a,(#0xDD)

  ld e, a               ; Save DD in in e
  in a,(#0xDC)          ; Get nibble bits 0-1 from DC
  and a, #0xC0          ; (keep upper 2 bits)
  ld b, a               ; Save in b
  ld a, e               ; Retrive value from port DD
  and a, #0x03          ; (keep lower 2 bits)
  or a, b               ; Combine with upper bits

  rrca                  ; Rotate so the nibble occupies
  rrca                  ; the upper 4 bits
  ld c,a                ; Save the result for later

  ;;; read nibble 2 (X lower bits)
  ld b, d
$11:                    ; Wait TL High
  dec b
  jp Z, $98             ; timeout
  in a,(#0xDD)
  bit 2,a
  jp Z, $11
  in a,(#0xDD)          ; repoll for stability

  ld e, a               ; Save DD in e
  in a,(#0xDC)          ; Get nibble bits 0-1
  and a, #0xC0          ; (keep upper 2 bits)
  ld b, a               ; Save in b
  ld a, e               ; Retrive value from port DD
  and a, #0x03          ; (keep lower 2 bits)
  or a, b               ; Combine with upper bits

  rlca                  ; Rotate so the nibble occupies
  rlca                  ; the lower 4 bits
  or a, c               ; Combine with upper nibble

  ld (_inlib_portB + 5), a

;;; read nibble 3 (Y upper bits)
  ld b, d
$12:                    ; Wait TL low
  dec b
  jp Z, $98             ; timeout
  in a,(#0xDD)
  bit 2,a
  jp NZ, $12
  in a,(#0xDD)          ; repoll for stability

  ld e, a               ; Save DD in in e
  in a,(#0xDC)          ; Get nibble bits 0-1 from DC
  and a, #0xC0          ; (keep upper 2 bits)
  ld b, a               ; Save in b
  ld a, e               ; Retrive value from port DD
  and a, #0x03          ; (keep lower 2 bits)
  or a, b               ; Combine with upper bits

  rrca                  ; Rotate so the nibble occupies
  rrca                  ; the upper 4 bits
  ld c,a                ; Save the result for later

  ;;; read nibble 4 (Y lower bits)
  ld b, d
$13:                    ; Wait TL High
  dec b
  jp Z, $98             ; timeout
  in a,(#0xDD)
  bit 2,a
  jp Z, $13
  in a,(#0xDD)          ; repoll for stability

  ld e, a               ; Save DD in e
  in a,(#0xDC)          ; Get nibble bits 0-1
  and a, #0xC0          ; (keep upper 2 bits)
  ld b, a               ; Save in b
  ld a, e               ; Retrive value from port DD
  and a, #0x03          ; (keep lower 2 bits)
  or a, b               ; Combine with upper bits

  rlca                  ; Rotate so the nibble occupies
  rlca                  ; the lower 4 bits
  or a, c               ; Combine with upper nibble

  ld (_inlib_portB + 6), a

  ;;; read nibble 5 (Buttons)
  ld b, d
$14:                    ; Wait TR high
  dec b
  jp Z, $98             ; timeout
  in a,(#0xDD)
  bit 3,a
  jp Z, $14
  in a,(#0xDD)          ; repoll for stability

  and a, #0xc0          ; Only keep used bits (buttons 1/2)
  rrca                  ; Normalize button bit positions
  rrca                  ; (use bits 4 and 5)
  xor a, #0x30          ; Invert buttons
  ld (_inlib_portB + 1),a
  xor a                 ; Clear button high byte
  ld (_inlib_portB + 2),a

  ld a, #INLIB_TYPE_SPORTSPAD_MARKIII
  ld (_inlib_portB + 0),a
  ret

$98:
  ld a, #INLIB_TYPE_NONE
  ld (_inlib_portB + 0),a
  ret

  __endasm;
}

