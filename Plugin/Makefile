CC=gcc
CFLAGS=-g -Wall -fPIC -std=c99 $(shell pkg-config --cflags gtk+-2.0)

INCLUDES=
LDFLAGS= $(shell pkg-config --libs gtk+-2.0) -shared -lfftw3

DSPSOURCES=feedback_suppressor.c biquad_filter.c
DSPOBJECTS = $(DSPSOURCES:.c=.o)
DSP = feedback_suppressor.lv2/feedback_suppressor.so

UISOURCES=feedback_suppressor_ui.c
UIOBJECTS = $(UISOURCES:.c=.o)
UI = feedback_suppressor.lv2/feedback_suppressor_ui.so

all: folder dsp ui

folder:
	mkdir -p feedback_suppressor.lv2/

install:
	cp -r feedback_suppressor.lv2/ /usr/lib/lv2/

uninstall:
	rm -r /usr/lib/lv2/feedback_suppressor.lv2/

dsp: $(DSPSOURCES) $(DSP)

ui: $(UISOURCES) $(UI)

$(DSP): $(DSPOBJECTS)
	$(CC) $(DSPOBJECTS) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -o $@
	
$(UI): $(UIOBJECTS)
	$(CC) $(UIOBJECTS) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $< -g $(CFLAGS) -c -o $@

clean:
	rm -f feedback_suppressor.lv2/feedback_supressor.so
	rm *.o
