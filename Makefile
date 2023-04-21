CC?=cc
PREFIX?=/usr/local
CFLAGS?=-pthread -Wall -Wextra -Wpedantic -Werror -O3
LDFLAGS?=

OBJECTS=src/main.o src/io.o
TARGET=spipe

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

install: $(TARGET)
	-install $(TARGET) $(PREFIX)/bin/$(TARGET)

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

uninstall:
	-rm -f $(PREFIX)/bin/$(TARGET)

default: $(TARGET)
