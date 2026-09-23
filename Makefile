CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

LDFLAGS = -pthread -lrt

SERVER_SOURCES = src/server.c src/database.c src/queue.c src/logger.c src/config.c src/monitor.c

CLIENT_SOURCES = src/client.c

all: server client

server: $(SERVER_SOURCES)
	$(CC) $(CFLAGS) -o server $(SERVER_SOURCES) $(LDFLAGS)

client: $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) -o client $(CLIENT_SOURCES) $(LDFLAGS)

clean:
	rm -f server client
