#
# LetDown Makefile
# Complemento, by Acri Emanuele <crossbower@gmail.com>
# GNU GPL v3 - 2009
#


CC  =   g++
RM  =   rm -f

CFLAGS   =   -Wall `libnet-config --cflags --defines` `python-config --includes`
LDFLAGS  =   -lpcap `libnet-config --libs` `python-config --ldflags`
DBGFLAGS =   -ggdb

SRC      =   $(wildcard *.cpp)
OBJECTS  =   $(SRC:.cpp=.o)
PROGNAME =   letdown

all: $(PROGNAME)

letdown: $(OBJECTS)
	$(CC) -o $(PROGNAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

%.o: %.cpp %.h
	$(CC) -o $@ -c $< $(CFLAGS)

debug:
	$(CC) -o $(PROGNAME) $(SRC) $(CFLAGS) $(LDFLAGS) $(DBGFLAGS)

rebuild: distclean all

clean:
	$(RM) *.o $(PROGNAME)

distclean: clean
	$(RM) $(PROGNAME)

