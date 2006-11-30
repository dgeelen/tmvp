#!/bin/sh

MPLAYER="`which mplayer`"
MENCODER="`which mencoder`"
ILEAVE=../ileave
IMAGIZER=../imagize
COMPRESS=../lz77s
NORMALIZER=../normalize
FMAGIC="fmagic.txt"
FONT="blocks.fon"
VIDFILTERS="filmdint=io=23976:20000,scale=160:100,format=rgb24"

if [ "${2}" == "dafox" ] ; then
  MPLAYER="`which mplayer`"
  MENCODER="`which mencoder`"
  ILEAVE=~/Projects/tmvp/ileave/ileave
  IMAGIZER=~/Projects/tmvp/imagizer1/imagize
  COMPRESS=~/Projects/tmvp/compress1/lz77s
  NORMALIZER=~/Projects/tmvp/normalizer/normalize
  FMAGIC="filemagic.dat"
  FONT="blocks.fon"
  VIDFILTERS="filmdint=io=23976:20000,scale=320:-2,pp7=0:1,unsharp,2xsai,scale=-1:-2,hqdn3d,dsize=160:100,scale=160:100,format=rgb24"
else
if [ "${2}" == "simon" ] ; then
  MPLAYER="/cygdrive/c/stuff/mplayer/mplayer/mplayer.exe"
  MENCODER="/cygdrive/c/stuff/mplayer/mplayer/mencoder.exe"
  ILEAVE="../ileave.exe"
  IMAGIZER="../imagize.exe"
  COMPRESS="../lz77s.exe"
  NORMALIZER="../normalize.exe"
  VIDFILTERS="filmdint=io=23976:20000,hqdn3d,scale=160:100,hqdn3d,dsize=160:100,scale=-1:-2,format=rgb24"
fi

if [ ! -f "${MPLAYER}" ] ; then
   echo "Error: MPlayer not found!"
  exit
fi

if [ ! -f "${MENCODER}" ] ; then
   echo "Error: MEncoder not found!"
  exit
fi

if [ ! -f "${ILEAVE}" ] ; then
   echo "Error: Audio Interleaver not found!"
  exit
fi

if [ ! -f "${IMAGIZER}" ] ; then
   echo "Error: Imagizer not found!"
  exit
fi

if [ ! -f "${COMPRESS}" ] ; then
   echo "Error: Compressor not found!"
  exit
fi

if [ ! -f "${NORMALIZER}" ] ; then
   echo "Error: Normalize not found!"
  exit
fi

if [ ! -f "${FMAGIC}" ] ; then
   echo "Error: Cannot find \`${FMAGIC}'! (Required for FileMagic)"
  exit #GOTO EXIT :p
fi

if [ ! -f "${FONT}" ] ; then
  echo "Error: Cannot find \`${FONT}'! (Required for FontData)"
  exit
fi

if [ "${1}" != "" ] ; then
  if [ "${1}" == "-" ] ; then
    INFILE="/dev/stdin"
    OUTFILE=test.tmv
  else
    INFILE="$1"
    OUTFILE="`echo ${1} | sed 's:\(.*\)\(\.[^\.]*\):\1.tmv:'`"
    if ! echo "${OUTFILE}" | grep tmv ; then
      OUTFILE="${OUTFILE}.tmv"
    fi
    echo Transcoding \`${1}\' to \`${OUTFILE}\'
  fi
else
  echo Warning: No input filename given, defaulting to \`/dev/stdin\'
  INFILE="/dev/stdin"
  OUTFILE=test.tmv
fi

if [ ! -f "${INFILE}" ] ; then
  echo "Error: cannot find \`${INFILE}'"
  exit
fi

# just to be sure?...
rm /tmp/vidfifo  &> /dev/null
rm /tmp/audfifo1 &> /dev/null
rm /tmp/audfifo2 &> /dev/null

# create our fifo pipes
mkfifo /tmp/vidfifo
mkfifo /tmp/audfifo1
mkfifo /tmp/audfifo2

echo ">Starting audio decoder (mplayer)"
#Volnorm=2:1 => uses several samples for better accuracy. However results in ~1s of soft sound at the start of the file
"$MPLAYER" "$INFILE" -vc null -vo null -ao pcm:fast:file=/tmp/audfifo1:nowaveheader	\
  -af volnorm=2:1,resample=8000,channels=1:2:0:0:1:0,format=u8			\
  ${3} \
  -quiet &> aud.log &

echo ">Starting video decoder (mencoder)"
#"$MENCODER" "$INFILE" -o /tmp/vidfifo -of rawvideo -ovc raw -oac copy 		\
#  -vf filmdint=io=23976:20000,hqdn3d,scale=160:100,hqdn3d,scale,format=rgb24	\
#  -quiet &> vid.log &
"$MENCODER" "$INFILE" -o /tmp/vidfifo -of rawvideo -ovc raw -oac copy     \
  -vf-add $VIDFILTERS \
  ${3} \
  -quiet &> vid.log &

echo ">Starting audio normalizer"
"$NORMALIZER" /tmp/audfifo1 /tmp/audfifo2 &

echo ">Making file header"
cat ${FMAGIC} > "$OUTFILE"
cat ${FONT} >> "$OUTFILE"

echo ">Starting imagizer + interleaver + compressor"
  "$IMAGIZER" /tmp/vidfifo 		\
| "$ILEAVE" - /tmp/audfifo2 - 	\
| "$COMPRESS" 			\
>> "$OUTFILE"

echo ">DONE"

# delete our fifo pipes
rm /tmp/vidfifo
rm /tmp/audfifo1
rm /tmp/audfifo2
