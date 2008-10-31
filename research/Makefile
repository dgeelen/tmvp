obj-m += blinkbit.o

all: ctst2 ctst3
blink: blinkbit.ko
blinkbit.ko: blinkbit.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
ctst: ctst.cpp
	g++ ctst.cpp -lncurses -o ctst -I .
ctst2: ctst2.cpp
	g++ -O2 ctst2.cpp -lcurses -o ctst2 -I .
ctst3: ctst3.cpp
	g++ -O2 ctst3.cpp -lcurses -o ctst3 -I .
ctst5: ctst5.cpp 
	g++ -O2 ctst5.cpp ../common/UImage.cpp ../common/UColor.cpp ../imagizer1/URender.cpp ../imagizer1/mediancut.cpp ../imagizer1/median_cut.cpp -lpng -DWITH_PNG -o ctst5
clean:
	rm -f ctst  ctst.exe
	rm -f ctst2 ctst2.exe
	rm -f ctst3 ctst3.exe
	rm -f ctst4 ctst4.exe
	rm -f ctst5 ctst5.exe
cleanbb:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
