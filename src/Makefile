CC=gcc
BIN=arcomage
BINSERVER=arcomage-server
CFLAGS=-c

all: $(BIN) $(BINSERVER)

clean:
	rm -f minIni.o arco.o arcosrv.o common.o cards.o graphics.o input.o network.o sound.o BFont.o $(BIN) $(BINSERVER)

$(BIN): minIni.o arco.o common.o cards.o graphics.o input.o network.o sound.o BFont.o
	#$(CC) minIni.o arco.o common.o cards.o graphics.o input.o network.o sound.o BFont.o `pkg-config --libs sdl` -lSDL_image -lSDL_net -lSDL_mixer `pkg-config --libs lua5.1` -o $(BIN)

$(BINSERVER): minIni.o arcosrv.o network.o common.o cards.o
	$(CC) minIni.o arcosrv.o common.o cards.o network.o `pkg-config --libs sdl` -lSDL_net -o $(BINSERVER)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ `pkg-config --cflags sdl` `pkg-config --cflags lua5.1`
