# Hızlı POSIX derlemesi (CMake gerektirmez):  make -C src
# Tam derleme, testler ve Windows için üst dizindeki CMakeLists.txt kullanılır.
# Uyarı listesi CMakeLists.txt'deki MANGUS_WARNINGS ile aynı tutulur.
CC      ?= cc
CFLAGS  ?= -O2 -g
WARN     = -Wall -Wextra -Wno-unused-parameter
CPPFLAGS += -DHAVE_CRYPT=1
LDLIBS   =

# gcc'nin bilinen gürültülü sezgisel denetimleri (clang bu bayrakları tanımaz)
IS_CLANG := $(shell echo | $(CC) -dM -E - 2>/dev/null | grep -c __clang__)
ifeq ($(IS_CLANG),0)
  WARN += -Wno-format-truncation -Wno-stringop-truncation -Wno-maybe-uninitialized
endif

UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  CPPFLAGS += -DHAVE_CRYPT_H=1
  LDLIBS   += -lcrypt -lm
endif
ifeq ($(OS),Windows_NT)           # MSYS2 / MinGW-w64: çıktı mangus.exe olur
  CPPFLAGS := $(filter-out -DHAVE_CRYPT=1,$(CPPFLAGS)) -D_WIN32_WINNT=0x0601
  LDLIBS   += -lws2_32
endif

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

mangus: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

%.o: %.c $(wildcard *.h)
	$(CC) -std=gnu11 $(CPPFLAGS) $(CFLAGS) $(WARN) -c $<

clean:
	rm -f $(OBJS) mangus mangus.exe

.PHONY: clean
