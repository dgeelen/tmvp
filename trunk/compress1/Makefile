all: lz77s unlz77s
lz77s: lz77s.cpp mystream.cpp
	g++ -march=pentium-m -O3 -pipe -fomit-frame-pointer -falign-functions=32 -fweb -ffast-math lz77s.cpp mystream.cpp -o lz77s
unlz77s: unlz77s.cpp mystream.cpp
	g++ unlz77s.cpp mystream.cpp -o unlz77s
clean:
	rm *.exe
	rm *.obj
