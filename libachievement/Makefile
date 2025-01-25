CXX	= g++10
BIN_DIR = lib
BIN     = $(BIN_DIR)/libachievement.a
INCLUDE = .

GCC_VERSION = $(shell $(CC) --version 2>&1 | grep "(GCC)" | cut -d' ' -f3  | cut -d'.' -f1)

CFLAGS  = -m32 -Wall -O2 -pipe -g -I$(INCLUDE) -I../Extern/include
CFLAGS += -std=c++20

LIBS =

OBJFILES = Document.o Util.o

default:
	$(MAKE) $(BIN)

$(BIN): $(OBJFILES)
	if [ ! -d $(BIN_DIR) ]; then mkdir $(BIN_DIR); fi
	ar cru $(BIN) $(OBJFILES) $(LIBS)
	ranlib $(BIN)
	chmod 700 $(BIN)

clean:
	rm -f *.o
	rm -f $(BIN)

dep:
	touch Depend
	$(CXX) $(CFLAGS) -MM *.cpp > Depend

$(OBJFILES):
	$(CXX) $(CFLAGS) -c $<

sinclude Depend
