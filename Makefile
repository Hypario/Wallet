SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

%.o: %.c
		gcc -o $@ $< `pkg-config --cflags --libs gtk4`

all: $(OBJS)
