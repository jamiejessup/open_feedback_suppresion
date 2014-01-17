CC=g++
CFLAGS=-O3 -Wall -fPIC

INCLUDES=
LDFLAGS= -shared

SOURCES=feedback_supressor.c biquad_filter.c
OBJ = $(SOURCES:.c=.o)
FS = feedbackSupressor.lv2/feedback_supressor.so

all: folder plugin

folder:
	mkdir -p feedbackSupressor.lv2/

install:
	cp -r feedbackSupressor.lv2/ /usr/lib/lv2/

uninstall:
	rm -r /usr/lib/lv2/feedbackSupressor.lv2/

plugin: $(SOURCES) $(FS)

$(FS): $(OBJ)
	$(CC) $(OBJ) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $< -g $(CFLAGS) -c -o $@

clean:
	rm -f feedbackSupressor.lv2/feedback_supressor.so
	rm *.o
