CC = g++
src = 11127137.cpp
target = 11127137
Libs = -lpthread

all: $(target)
	$(CC) -o $(target) $(src) $(Libs)

clean:
	rm -f $(target)