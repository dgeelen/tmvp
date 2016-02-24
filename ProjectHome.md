This is an effort to display the best possible video in 80x50 16 color text mode.
We have actually already achieved most of this goal, and as a result development is currently somewhat stalled.


### Notes ###
We have placed an executable of the player online, since that uses pure assembly and needs to be compiled with Borland Turbo Assembler. We use version 5.0, but other versions may work. The tools you need to actually create a video should build cleanly on linux ([r215](https://code.google.com/p/tmvp/source/detail?r=215)), and possibly cygwin (cygwin has not been tested for a while). Just run "makeall.sh" located in the scripts folder.
After that encoding is fairly straight forward using "encode.sh" (also in the scripts folder)
You will probably need to play with, and/or modify the script a bit as it is 'tuned' to our development environment ;-)