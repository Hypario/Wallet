SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

%.o: %.c
		gcc -o $@ $< `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`

all: $(OBJS)