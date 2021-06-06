SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

CFLAGSA := `pkg-config --libs gtk+-3.0` -ljansson
CFLAGSB := `pkg-config --cflags gtk+-3.0`

%.o: %.c
	gcc $(CFLAGSB) -o $@ $< $(CFLAGSA)

all: $(OBJS)

clean:
	rm -rf build
