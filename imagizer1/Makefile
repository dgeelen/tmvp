all: cmdline
cmdline: imagize.cpp URender.cpp UColor.cpp UImage.cpp median_cut.cpp
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math imagize.cpp URender.cpp UColor.cpp UImage.cpp median_cut.cpp -o imagize
clean:
	rm *.exe
	rm *.obj
