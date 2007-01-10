all: imagize
imagize: imagize.cpp URender.cpp mediancut.cpp mediancut.h ../common/UColor.cpp ../common/UImage.cpp median_cut.cpp URender.h ../common/UColor.h ../common/UImage.h median_cut.h ../common/UTypes.h ../common/UCmdLineParser.cpp ../common/UCmdLineParser.h
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math imagize.cpp mediancut.cpp URender.cpp ../common/UColor.cpp ../common/UImage.cpp median_cut.cpp ../common/UCmdLineParser.cpp -o imagize
clean:
	rm -f imagize imagize.exe
