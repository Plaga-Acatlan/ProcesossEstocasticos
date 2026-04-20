CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -std=c99
LDFLAGS =

# Archivos fuente
SRCS = src/datatype.c src/tool.c
OBJS = $(SRCS:.c=.o)

# Biblioteca estática
LIB = libprocesos.a

# Ejecutable de ejemplo
MAIN = main
MAIN_SRC = src/main.c

all: $(LIB) $(MAIN)

$(LIB): $(OBJS)
	ar rcs $@ $^

$(MAIN): $(MAIN_SRC) $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB) $(MAIN)

.PHONY: all clean