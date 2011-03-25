.PHONY:all clean

all: moduleGSM

clean:
	rm moduleGSM

CFLAGS=`pkg-config --cflags ivy-glib gammu`
LDFLAGS=`pkg-config --libs ivy-glib gammu`

moduleGSM:moduleGSM.c gammutools.c
	gcc -g -O2 -Wall $(CFLAGS) -o $@ $^ $(LDFLAGS)

