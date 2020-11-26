SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%)

client: source/client.c
	gcc source/client.c -o bin/client

server: source/server.c
	gcc source/server.c -o bin/server

resource: resource/cgi-source/$(SRC)
	gcc resource/cgi-source/$(SRC) -o resource/cgi-bin/$(OBJ)
