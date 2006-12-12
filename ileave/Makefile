all: ileave
ileave: main.cpp ../common/UTypes.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math main.cpp -o ileave
clean:
	rm -f ileave ileave.exe
