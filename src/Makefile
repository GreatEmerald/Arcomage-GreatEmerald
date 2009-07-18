CC=gcc
BIN=arcomage
BINSERVER=arcomage-server

all: $(BIN) $(BINSERVER)

clean:
	rm -f arco.o arcosrv.o common.o cards.o graphics.o input.o network.o sound.o BFont.o $(BIN) $(BINSERVER)

$(BIN): arco.o common.o cards.o graphics.o input.o network.o sound.o BFont.o
	$(CC) arco.o common.o cards.o graphics.o input.o network.o sound.o BFont.o -lSDLmain -lSDL -lSDL_image -lSDL_net -lSDL_mixer -o $(BIN)

$(BINSERVER): arcosrv.o network.o common.o cards.o
	$(CC) arcosrv.o network.o common.o cards.o -lSDLmain -lSDL -lSDL_net -o $(BINSERVER)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
