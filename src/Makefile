CC=sdcc
AR=sdar

# CONFIG +=

OPT=--max-allocs-per-node 100000
PEEP_OPTIONS=#--peep-file peep-rules.txt
CFLAGS=-mz80 $(OPT) $(CONFIG)
OUTPUT_LIBS=inlib.lib

SRCS=inlib.c inlib_mdpad.c inlib_paddle.c inlib_sportspad.c inlib_mdmouse.c inlib_sportspad_markIII.c

OBJS=$(patsubst %.c,%.rel,$(SRCS))

all: $(OUTPUT_LIBS)

inlib.lib: $(OBJS)
	$(AR) r $@ $^

# Default rules
%.rel: %.c inlib.h
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $<

clean:
	rm -f $(OUTPUT_LIBS) $(OBJS) *.lst *.sym *.asm
