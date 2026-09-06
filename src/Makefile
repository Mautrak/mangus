# Hızlı POSIX derlemesi (CMake gerektirmez):  make -C src
# Tam derleme, testler ve Windows için üst dizindeki CMakeLists.txt kullanılır.
CC      ?= cc
CFLAGS  ?= -O2 -g
WARN     = -Wall -Wextra -Wno-unused-parameter
CPPFLAGS += -DHAVE_CRYPT=1
LDLIBS   =

UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  CPPFLAGS += -DHAVE_CRYPT_H=1
  LDLIBS   += -lcrypt
endif

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

mangus: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

%.o: %.c $(wildcard *.h)
	$(CC) -std=gnu11 $(CPPFLAGS) $(CFLAGS) $(WARN) -c $<

clean:
	rm -f $(OBJS) mangus

.PHONY: clean
