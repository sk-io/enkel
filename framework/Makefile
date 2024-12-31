CC = g++
LD = g++
CFLAGS = -g -O2 -std=c++17 -I..
LDFLAGS = -g -O2 -L../enkel -lenkel -lSDL2main -lSDL2 -lSDL2_image

OBJS = main.o framework.o graphics.o image.o

all: framework

framework: $(OBJS)
	$(LD) -o framework $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f framework *.o

