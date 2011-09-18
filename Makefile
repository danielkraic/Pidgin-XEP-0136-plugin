CFLAGS=`pkg-config --cflags glib-2.0`
CFLAGS+=`pkg-config --cflags gtk+-2.0`
CFLAGS+=`pkg-config --cflags pidgin`
LDFLAGS=`pkg-config --libs glib-2.0`
LDFLAGS+=`pkg-config --libs pidgin`

xep136_plugin.so: xep136_plugin.o
	gcc $(LDFLAGS) -shared -g -o $@ $?

xep136_plugin.o: xep136_plugin.c
	gcc -c $(CFLAGS) -g -o $@ $?

clean:
	rm -f xep136_plugin.so
	rm -f xep136_plugin.o

install:
	cp xep136_plugin.so ~/.purple/plugins/
