CC = g++
CFLAGS = -g -O2 -std=c++17

OBJS = interpreter.o parser.o lexer.o ast_util.o gc.o scope.o

all: libenkel.a

libenkel.a: $(OBJS)
	ar rcs libenkel.a $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f libenkel.a *.o

