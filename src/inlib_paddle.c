// Code adapted from SMSlib, part of devkitSMS - github.com/sverx/devkitSMS

#include "inlib.h"

// (original asm code from SMS Test Suite)
// (improved by Raphaël Assenat [raphnet])
// references:
// https://www.smspower.org/Development/Paddle
// https://www.smspower.org/Development/PeripheralPorts

#define DETECT_MIN  0x60
#define DETECT_MAX  0xA0

#pragma disable_warning 85

void inlib_readPaddle (unsigned char port) __z88dk_fastcall __naked {
  __asm
    ld a,l
    or a
    jr nz, read_second_pad

    ; First, synchronize by waiting until port A key 2 is high.
    ;
    ; Without this, the values occasionally glitches on real hardware,
    ; because the bits may be in the middle of changing and are therefore
    ; not reliable. (Remember the real world is analog)
    ;

    ld d, #255
wait_5_set_sync:
    dec d
    jp Z, paddle_timeout1
    in a,(#0xDC)
    bit 5,a
    jr z, wait_5_set_sync    ; wait until bit 5 is 1

    ld d, #255
wait_5_reset:
    dec d
    jp Z, paddle_timeout1
    in a,(#0xDC)
    bit 5,a
    jr nz, wait_5_reset     ; wait until bit 5 is 0
    and #0x0F
    ld l,a                  ; save lower 4 bits into l

    ld d, #255
wait_5_set:
    dec d
    jp Z, paddle_timeout1
    in a,(#0xDC)
    bit 5,a
    jr z, wait_5_set        ; wait until bit 5 is 1
    and #0x0F               ; save lower 4 bits
    add a,a
    add a,a
    add a,a
    add a,a
    or l                    ; move to high nibble
    ld l,a                  ; together with lower part, save in L

    ld a,#INLIB_TYPE_PADDLE
    ld (_inlib_portA+0), a

    in a,(#0xDC)            ; Get button status (bit 4)
    or a,#0xef              ; Set unused bits
    cpl                     ; Invert bits
    ld (_inlib_portA+1), a  ; Buttons lower byte
    ld a,#0x00
    ld (_inlib_portA+2), a  ; Buttons upper byte
    ld a, l                 ; Retrive value from L
    ld (_inlib_portA+5), a  ; Write paddle value to structure

    ret

paddle_timeout1:
    ld a,#INLIB_TYPE_NONE   ; Signal a disconnected paddle
    ld (_inlib_portA+0), a
    ret

read_second_pad:
    ld c,#0xDC

    ld bc, (#_inlib_portB + 1) ; Read current button bits
    ld (#_inlib_portB + 3), bc ; Copy to previous button bits
    ; First, synchronize by waiting until port B key 2 is high.
    ;
    ; Without this, the values occasionally glitches on real hardware,
    ; because the bits may be in the middle of changing and are therefore
    ; not reliable. (Remember the real world is analog)

    ld d, #255
wait_3_set_sync:
    dec d
    jp Z, paddle_timeout2
    in a,(#0xDD)
    bit 3,a
    jr z, wait_3_set_sync   ; wait until bit 3 is 1

    ld d, #255
wait_3_reset:
    dec d
    jp Z, paddle_timeout2
    in a,(#0xDD)            ; ensure we are reading both ports same moment
    ld e,a
    in b,(c)
    in a,(#0xDD)
    or e
    bit 3,a
    jr nz, wait_3_reset     ; wait until bit 5 is 0
    ld a,b
    and #0xC0               ; save upper 2 bits
    rlca
    rlca
    ld l,a                  ; into l (bits 0,1)
    ld a,e
    and #0x03               ; save lower 2 bits
    rlca
    rlca
    or l                    ; together with l
    ld l,a                  ; into l (bits 2,3)

    ld d, #255
wait_3_set:
    dec d
    jp Z, paddle_timeout2
    in a,(#0xDD)            ; ensure we are reading both ports same moment
    ld e,a
    in b,(c)
    in a,(#0xDD)
    and e
    bit 3,a
    jr z, wait_3_set        ; wait until bit 5 is 1
    ld a,b
    and #0xC0               ; save upper 2 bits
    rrca
    rrca
    ld h,a                  ; into h (bits 4,5)
    ld a,e
    and #0x03               ; save lower 2 bits
    rrca
    rrca
    or h                    ; together with h (bits 6,7)
    or l                    ; together with lower part
    ld l,a

    ld a,#INLIB_TYPE_PADDLE
    ld (_inlib_portB+0), a

    in a,(#0xDD)            ; Get button status (bit 2)
    rlca                    ; Move to bit 4
    rlca
    or a,#0xef              ; Set unused bits
    cpl                     ; Invert bits
    ld (_inlib_portB+1), a  ; Buttons lower byte
    ld a,#0x00
    ld (_inlib_portB+2), a  ; Buttons upper byte
    ld a, l                 ; Retrive value from L
    ld (_inlib_portB+5), a  ; Write paddle value to structure

    ret

paddle_timeout2:
    ld a,#INLIB_TYPE_NONE   ; Signal a disconnected paddle
    ld (_inlib_portB+0), a
    ret

  __endasm;
}
