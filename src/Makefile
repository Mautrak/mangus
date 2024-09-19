# Compiler selection
CXX = g++

# Determine the operating system to set executable extension and remove command
ifeq ($(shell uname -s), Linux)
    EXE_EXT =
    RM = rm -f
    OPENSSL_INC =
    OPENSSL_LIB =
    LIBS = -lssl -lcrypto -lcrypt
else ifeq ($(OS), Windows_NT)
    EXE_EXT = .exe
    RM = del /Q
    OPENSSL_INC = -IC:/mingw64/include
    OPENSSL_LIB = -LC:/mingw64/lib
    LIBS = -lssl -lcrypto
endif

# Flags for optimization and debugging
PROF = -O2 -g
NOCRYPT =

# Compiler and linker flags
CXXFLAGS = -Wall $(PROF) $(NOCRYPT) -Wno-misleading-indentation -Wno-format-overflow $(OPENSSL_INC)
LDFLAGS = $(PROF) $(OPENSSL_LIB)

# Source files
SRC_FILES = \
    act_comm.c act_hera.c act_info.c act_move.c act_obj.c act_wiz.c ban.c \
    cJSON.c comm.c const.c db.c db2.c dilek.c kirliisler.c effects.c ek.c ek2.c \
    exlist.c fight.c flags.c handler.c handler_room.c healer.c interp.c lookup.c mafya.c \
    magic.c magic2.c martial_art.c mob_creator.c mob_prog.c obj_creator.c obj_prog.c \
    quest.c recycle.c save.c skills.c special.c tables.c update.c

# Object files generated from source files
OBJ_FILES = $(SRC_FILES:.c=.o) data.o

# Executable name
TARGET = mangus$(EXE_EXT)

# Default target
.PHONY: all
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJ_FILES)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile .c files to .o files with dependency on merc.h
%.o: %.c merc.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Handle data.c separately if needed
data.o: data.c merc.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up generated files
.PHONY: clean
clean:
	$(RM) $(OBJ_FILES) $(TARGET)

# Phony target to force rebuild
.PHONY: rebuild
rebuild: clean all
