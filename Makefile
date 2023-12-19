CC=gcc
CFLAGS=-I. -Wall

TARGET=main

SRCS=main.c server.c

OBJDIR=obj

OBJS=$(patsubst %,$(OBJDIR)/%,$(SRCS:.c=.o))

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(TARGET)

.PHONY: clean

