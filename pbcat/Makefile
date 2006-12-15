all: pbcat
pbcat: pbcat.cpp ../common/UTypes.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math pbcat.cpp -o pbcat
clean:
	rm -f pbcat pbcat.exe
