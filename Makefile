# xep136_plugin makefile

CFLAGS=`pkg-config --cflags glib-2.0`
CFLAGS+=`pkg-config --cflags gtk+-2.0`
CFLAGS+=`pkg-config --cflags pidgin`

LDFLAGS=`pkg-config --libs glib-2.0`
LDFLAGS+=`pkg-config --libs pidgin`

INSTDIR = ~/.purple/plugins/


xep136_plugin.so: xep136_plugin.o
	gcc $(LDFLAGS) -shared -g -o $@ $? 

xep136_plugin.o: xep136_plugin.c
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC

clean:
	rm -f xep136_plugin.so
	rm -f xep136_plugin.o

install: xep136_plugin.so
	@if [ -d $(INSTDIR) ]; \
	then \
	    cp xep136_plugin.so $(INSTDIR); \
	    echo "xep136_plugin.so installed"; \
	else \
	    mkdir $(INSTDIR); \
	    cp xep136_plugin.so $(INSTDIR); \
	    echo "created $(INSTDIR)"; \
	    echo "xep136_plugin.so installed"; \
	fi
