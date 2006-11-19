obj-m += blinkbit.o

all: ctst ctst2 ctst3
blink: blinkbit.ko
blinkbit.ko: blinkbit.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
ctst: ctst.cpp
	g++ ctst.cpp -lncurses -o ctst -I .
ctst2: ctst2.cpp
	g++ -O2 ctst2.cpp -lcurses -o ctst2 -I .
ctst3: ctst3.cpp
	g++ -O2 ctst3.cpp -lcurses -o ctst3 -I .
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm ctst
	rm ctst2
	rm *.exe &> /dev/null
	rm *.obj &> /dev/null
