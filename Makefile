SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

%.o: %.c
		gcc `pkg-config --cflags gtk+-3.0` -o $@ $< `pkg-config --libs gtk+-3.0` -Wincompatible-pointer-types

all: $(OBJS)
