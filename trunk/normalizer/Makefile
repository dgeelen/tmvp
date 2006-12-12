all: normalize
normalize: normalize.cpp ../common/UTypes.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math normalize.cpp -o normalize
clean:
	rm -f normalize normalize.exe

