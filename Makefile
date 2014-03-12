CC = g++
CFLAGS = -Wall

all : twitServer twitClient

twitServer: twitServer.o
	$(CC) $(CFLAGS) twitServer.o -o $@

twitServer.o: twitServer.cpp twitServer.h
	$(CC) $(CFLAGS) -c $<

twitClient: twitClient.o
	$(CC) $(CFLAGS) twitClient.o -o $@

twitClient.o: twitClient.cpp twitClient.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o twitServer twitClient
