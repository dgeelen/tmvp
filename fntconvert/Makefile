all: fnt2rgb rgb2fnt
fnt2rgb: fnt2rgb.cpp ../common/UTypes.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math fnt2rgb.cpp -o fnt2rgb
rgb2fnt: rgb2fnt.cpp ../common/UTypes.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math rgb2fnt.cpp -o rgb2fnt
clean:
	rm -f fnt2rgb fnt2rgb.exe
	rm -f rgb2fnt rgb2fnt.exe
