

MPLAYER="/cygdrive/c/stuff/mplayer/mplayer/mplayer.exe"
MENCODER="/cygdrive/c/stuff/mplayer/mplayer/mencoder.exe"
ILEAVE="../ileave/ileave.exe"
IMAGIZER="../imagizer1/imagize.exe"
COMPRESS="../compress1/lz77s.exe"
NORMALIZER="../normalizer/normalize.exe"

INFILE="$1"
OUTFILE="out.tmv"

rm vidfifo  &> /dev/null
rm audfifo1 &> /dev/null
rm audfifo2 &> /dev/null

mkfifo vidfifo
mkfifo audfifo1
mkfifo audfifo2

#echo "---extracting audio---"

echo ">Starting audio decoder (mplayer)"
"$MPLAYER" "$INFILE" -vc null -vo null -af volnorm,resample=8000,channels=1:2:0:0:1:0,format=u8 -ao pcm:fast:file=audfifo1:nowaveheader \
-quiet &> aud.log &

echo ">Starting video decoder (mencoder)"
"$MENCODER" "$INFILE" -o vidfifo -of rawvideo -ovc raw -vf filmdint=io=23976:20000,scale=160:100,format=rgb24 -oac copy \
-quiet &> vid.log &
 
echo ">Starting audio normalizer"
# "$NORMALIZER" audfifo1 audfifo2 & <-- doest work
# cat audfifo1 | "$NORMALIZER" > audfifo2 & <-- doest work
# line below does seems to work.... , cygwin weirdness? deadlock? :P
"$NORMALIZER" audfifo1 > audfifo2 &

echo ">Making file header"
cat fmagic.txt > "$OUTFILE"
cat blocks.fon >> "$OUTFILE"

echo ">Starting imagizer+interleaver+compressor"
  "$IMAGIZER" vidfifo 		\
| "$ILEAVE" - audfifo2 - 	\
| "$COMPRESS" 			\
>> "$OUTFILE";

echo ">DONE"

rm vidfifo
rm audfifo1
rm audfifo2