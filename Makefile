# xep136_plugin Makefile

CC=gcc

GTK_CFLAGS=`pkg-config --cflags glib-2.0`
GTK_CFLAGS+=`pkg-config --cflags gtk+-2.0`
PIDGIN_CFLAGS+=`pkg-config --cflags pidgin`

GTK_LIBS=`pkg-config --libs glib-2.0`
GTK_LIBS+=`pkg-config --libs gtk+-2.0`
PIDGIN_LIBS+=`pkg-config --libs pidgin`

INSTDIR = ~/.purple/plugins/

xep136_plugin.so: xep136_coll.o xep136_gtk.o xep136_imhtml.o xep136_misc.o xep136_plugin.o xep136_receive.o xep136_send.o 
	$(CC) $(LDFLAGS) -shared $(CFLAGS) $? -o $@ $(PIDGIN_LIBS) $(GTK_LIBS) 

xep136_coll.o: xep136_coll.c xep136_plugin.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS)

xep136_gtk.o: xep136_gtk.c xep136_plugin.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) 

xep136_imhtml.o: xep136_imhtml.c xep136_plugin.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) 

xep136_misc.o: xep136_misc.c xep136_plugin.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) 

xep136_plugin.o: xep136_plugin.c xep136_plugin.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) 

xep136_receive.o: xep136_receive.c xep136_plugin.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) 

xep136_send.o: xep136_send.c xep136_plugin.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) 


clean:
	rm -f xep136_plugin.so
	rm -f *.o


install: xep136_plugin.so
	@if [ -d $(INSTDIR) ]; \
	then \
	    cp xep136_plugin.so $(INSTDIR); \
	    echo "xep136_plugin.so installed"; \
	else \
	    mkdir -p $(INSTDIR); \
	    cp xep136_plugin.so $(INSTDIR); \
	    echo "created $(INSTDIR)"; \
	    echo "xep136_plugin.so installed"; \
	fi
