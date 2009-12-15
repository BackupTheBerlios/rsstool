.PHONY: all clean distclean install uninstall

CC=gcc
ifdef CFLAGS
CFLAGS+=-I. -DHAVE_CONFIG_H
else
CFLAGS=-I. -Wall -W -O3 -DHAVE_CONFIG_H
endif
#ifndef LDFLAGS
#LDFLAGS=-s
#endif
TARGET=rsstool


include config.mak


#DESTDIR=
#BINDIR=/usr/local/bin


OBJECTS=$(TARGET).o rsstool_misc.o rsstool_write.o \
	misc/base64.o misc/getopt.o misc/getopt2.o \
	misc/net.o misc/string.o misc/file.o misc/hash.o misc/crc32.o misc/md5.o \
	misc/misc.o misc/rss.o misc/sql.o misc/xml.o
ifdef USE_ZLIB
OBJECTS+=misc/unzip.o #misc/zip.o
endif
ifdef USE_ODBC
OBJECTS+=misc/sql_odbc.o
endif
ifdef USE_MYSQL
OBJECTS+=misc/sql_mysql.o
endif


all: $(TARGET)


clean:
	rm -f $(TARGET) $(OBJECTS) *.core *.stackdump *~ */*~ */*/*~


distclean: clean
	rm -f config.mak config.log config.status config.cache config.h


.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -vf $(TARGET) $(DESTDIR)$(BINDIR)

install-strip: install
	strip $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)


# Dependencies

