# sxw - simple X widgets
# See LICENSE file for copyright and license details.

include config.mk

SRC = $(wildcard ./*.c) #drw.c util.c time.c battery.c 
OBJ = $(SRC:.c=.o)
OUT = $(SRC:.c=)

options:
	@echo date build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

.c.o:
	$(CC) -c $(CFLAGS) $<

$(OBJ): config.h config.mk drw.h util.h

time: time.o drw.o util.o
	$(CC) -o $@ time.o drw.o util.o $(LDFLAGS)

clean:
	rm -f $(OUT) $(OBJ) 

battery: battery.o drw.o util.o 
	$(CC) -o $@ battery.o drw.o util.o $(LDFLAGS)

weather: weather.o drw.o util.o 
	$(CC) -o $@ weather.o drw.o util.o $(LDFLAGS)

mpdplay: mpdplay.o drw.o util.o 
	$(CC) -o $@ mpdplay.o drw.o util.o $(LDFLAGS)

mpdback: mpdback.o drw.o util.o 
	$(CC) -o $@ mpdback.o drw.o util.o $(LDFLAGS)

mpdforward: mpdforward.o drw.o util.o 
	$(CC) -o $@ mpdforward.o drw.o util.o $(LDFLAGS)

mpdnext: mpdnext.o drw.o util.o 
	$(CC) -o $@ mpdnext.o drw.o util.o $(LDFLAGS)

mpdprev: mpdprev.o drw.o util.o 
	$(CC) -o $@ mpdprev.o drw.o util.o $(LDFLAGS)

mpdinfo: mpdinfo.o drw.o util.o  
	$(CC) -o $@ mpdinfo.o drw.o util.o $(LDFLAGS)
