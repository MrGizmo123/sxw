# sxw - simple X widgetsloaaa.la.la
# See LICENSE file for copyright and license details.

include config.mk

SRCDIR = src/
OBJDIR = obj/
BINDIR = bin/


PROGS = time weather battery mpdplay mpdinfo brightness packages iconbutton infowidget dwmtag volume quote planets

SRC = $(addprefix $(SRCDIR), $(addsuffix .c, $(PROGS))) 
OBJ = $(addprefix $(OBJDIR), $(addsuffix .o, $(PROGS))) obj/drw.o obj/util.o
OUT = $(addprefix $(BINDIR), $(PROGS))

options:
	@echo date build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

#.c.o:
#	$(CC) -c $(CFLAGS) $< 

$(OBJ): $(SRCDIR)config.h config.mk $(SRCDIR)drw.h $(SRCDIR)util.h

$(OBJDIR)%.o : $(SRCDIR)%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BINDIR)% : $(OBJDIR)%.o $(OBJDIR)drw.o $(OBJDIR)util.o
	$(CC) -o $@ $< $(OBJDIR)drw.o $(OBJDIR)util.o $(LDFLAGS) 

clean:
	rm -f $(OUT) $(OBJ)  

all: $(OUT)
