# xep136_plugin Makefile

CFLAGS=`pkg-config --cflags glib-2.0`
CFLAGS+=`pkg-config --cflags gtk+-2.0`
CFLAGS+=`pkg-config --cflags pidgin`

LDFLAGS=`pkg-config --libs glib-2.0`
LDFLAGS+=`pkg-config --libs pidgin`

INSTDIR = ~/.purple/plugins/


xep136_plugin.so: xep136_coll.o xep136_gtk.o xep136_imhtml.o xep136_misc.o xep136_plugin.o xep136_receive.o xep136_send.o
	gcc $(LDFLAGS) -shared -g -o $@ $? 

xep136_coll.o: xep136_coll.c 
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC

xep136_gtk.o: xep136_gtk.c
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC

xep136_imhtml.o: xep136_imhtml.c 
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC

xep136_misc.o: xep136_misc.c 
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC

xep136_plugin.o: xep136_plugin.c 
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC

xep136_receive.o: xep136_receive.c 
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC

xep136_send.o: xep136_send.c 
	gcc -c $(CFLAGS) -g -o $@ $? -fPIC


clean:
	rm -f xep136_plugin.so
	rm -f *.o


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
