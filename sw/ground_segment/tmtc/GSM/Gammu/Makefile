#CFLAGS=$(shell pkg-config --cflags glib-2.0 --libs)  `pcre-config --libs` -lglibivy
#LDFLAGS=$(shell pkg-config --cflags glib-2.0 gammu ivy-glib --libs)

#ALL=moduleGSM 

#.PHONY: all clean

#all: $(ALL)

#clean: rm -f $(ALL)

#%:%.c $(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

moduleGSM: moduleGSM.c
	gcc -g -O2 -Wall `pkg-config glib-2.0 gammu --cflags` -o $@ $^ `pkg-config glib-2.0 gammu --libs`  `pcre-config --libs` -lglibivy
