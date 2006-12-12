all: subtitler
subtitler: subtitler.cpp ../common/UColor.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math subtitler.cpp -L../common -o subtitler 
clean:
	rm -rf *.o subtitler.exe subtitler

