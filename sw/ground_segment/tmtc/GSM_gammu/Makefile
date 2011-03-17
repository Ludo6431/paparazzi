.PHONY:all

all: SMS_GS

clean:
	rm SMS_GS


CFLAGS=`pkg-config --cflags ivy-glib gammu`
LDFLAGS=`pkg-config --libs ivy-glib gammu`

%:%.c
	gcc -g -O2 -Wall $(CFLAGS) -o $@ $^ $(LDFLAGS)

