CC = gcc
CFLAGG = -Wall -Wextra -Werror -g
LIBS = -lssl -lcrypto

SRCS = fetchmail.c login.c retrieve.c parse.c mime.c list.c tls.c
OBJS = $(SRCS:.c=.o)
EXEC = fetchmail

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGG) $(OBJS) -o $(EXEC) $(LIBS)

%.o: %.c 
	$(CC) $(CFLAGG) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)