CC=sdcc
IHX2SMS=ihx2sms
DEVKITSMS_BASE=../../devkitSMS
SMSLIB_BASE=$(DEVKITSMS_BASE)/SMSlib
SMSLIB_INCDIR=$(SMSLIB_BASE)/src
PEEP_RULES=$(SMSLIB_BASE)/src/peep-rules.txt
CRT0=$(DEVKITSMS_BASE)/crt0/crt0_sms.rel
SMSLIB_LIB=$(SMSLIB_BASE)/SMSlib.lib       # Use distributed lib
#SMSLIB_LIB=$(SMSLIB_BASE)/src/SMSlib.lib   # Use locally compiled lib
INLIB_BASE=../src
INLIB_INCDIR=$(INLIB_BASE)
INLIB_LIB=$(INLIB_BASE)/inlib.lib

CFLAGS=-mz80 -I$(SMSLIB_INCDIR) --peep-file $(PEEP_RULES) -I$(INLIB_INCDIR)
LDFLAGS=--no-std-crt0 --data-loc 0xC000

ROMS=inlib_demo.sms inlib_light_demo.sms inlib_graphic_board_demo.sms

all: $(ROMS)

%.rel: %.c
	$(CC) $(CFLAGS) -c $<

%.rel: %.c %.h
	$(CC) $(CFLAGS) -c $<

%.ihx: %.rel $(SMSLIB_LIB) $(INLIB_LIB)
	$(CC) -o $@ $(LDFLAGS) $(CRT0) $(SMSLIB_LIB) $(INLIB_LIB) $<

%.sms: %.ihx
	$(IHX2SMS) $< $@

clean:
	rm -f *.rel *.ihx *.asm *.sym *.lst *.noi *.lk *.map *.sms

