SOURCES=core.c curses.c net.c nyaa.c main.c
CFLAGS=`xml2-config --cflags` `curl-config --cflags`
LDFLAGS=-lxml2 -lcurl -lncursesw

OBJECTS=$(SOURCES:.c=.o)
OUT=ncnyaa

$(OUT): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o $(OUT)
