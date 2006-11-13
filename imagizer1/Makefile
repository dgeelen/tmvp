all: imagize
imagize: imagize.cpp URender.cpp UColor.cpp UImage.cpp median_cut.cpp URender.h UColor.h UImage.h median_cut.h UTypes.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math imagize.cpp URender.cpp UColor.cpp UImage.cpp median_cut.cpp -o imagize
clean:
	rm *.exe
	rm *.obj
