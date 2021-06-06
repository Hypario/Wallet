SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

CFLAGS := `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -ljansson

%.o: %.c
	gcc -o $@ -c $< $(CFLAGS)

all: $(OBJS)

clean:
	rm -rf build
