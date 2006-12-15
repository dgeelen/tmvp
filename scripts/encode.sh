#!/bin/sh

MPLAYER="`which mplayer 2> /dev/null`"
MENCODER="`which mencoder 2> /dev/null`"
ILEAVE=../ileave
IMAGIZER=../imagize
COMPRESS=../lz77s
NORMALIZER=../normalize
FMAGIC="fmagic.txt"
FONT="asc_ord_hi.fon"
VIDFILTERS="filmdint,scale=160:100,format=rgb24"
SUBTITLER="`which cat`"

if [ "${2}" == "dafox" ] ; then
  MPLAYER="`which mplayer`"
  MENCODER="`which mencoder`"
  ILEAVE="/home/dafox/Projects/tmvp/ileave/ileave"
#  IMAGIZER=/home/dafox/Projects/tmvp/imagizer1/imagize
  IMAGIZER="/home/dafox/Projects/tmvp/imagizer1/kdev/imagize/optimized/src/imagize"
  COMPRESS="/home/dafox/Projects/tmvp/compress1/lz77s"
  PBCAT=~/Projects/tmvp/pbcat/pbcat
  NORMALIZER="/home/dafox/Projects/tmvp/normalizer/normalize"
  SUBTITLER="/home/dafox/Projects/tmvp/subtitler/kdev/subtitler/optimized/src/subtitler"
  FMAGIC="fmagic.txt"
#  FONT="blocks.fon"
  VIDFILTERS="filmdint,scale=320:-2,pp7=0:1,unsharp,2xsai,scale=-1:-2,hqdn3d,dsize=160:100,scale=160:100,format=rgb24"
elif [ "${2}" == "simon" ] ; then
  MPLAYER="/cygdrive/c/stuff/mplayer/mplayer/mplayer.exe"
  MENCODER="/cygdrive/c/stuff/mplayer/mplayer/mencoder.exe"
  ILEAVE="../ileave/ileave.exe"
  PBCAT="../pbcat/pbcat.exe"
  IMAGIZER="../imagizer1/imagize.exe"
  COMPRESS="../compress1/lz77s.exe"
  NORMALIZER="../normalizer/normalize.exe"
  VIDFILTERS="filmdint,hqdn3d,scale=160:100,hqdn3d,dsize=160:100,scale=-1:-2,format=rgb24"
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
  exit
fi

if [ ! -f "${FONT}" ] ; then
  echo "Error: Cannot find \`${FONT}'! (Required for FontData)"
  exit
fi

if [ ! -f "${PBCAT}" ] ; then
  echo "Error: Cannot find \`${FONT}'! (Required for FontData)"
  exit
fi

if [ ! -f "${SUBTITLER}" ] ; then
  echo "Error: Cannot find \`${FONT}'! (Required for FontData)"
  exit
fi

if [ "${1}" != "" ] ; then
  if [ "${1}" == "-" ] ; then
    INFILE="/dev/stdin"
    OUTFILE=test.tmv
  else
    INFILE="$1"
    OUTFILE="`echo ${1} | sed 's:\.[^\.]*$:\.tmv:'`"
    if ! echo "${OUTFILE}" | grep tmv > /dev/null ; then
      OUTFILE="${OUTFILE}.tmv"
    fi
#    echo Transcoding \`${1}\' to \`${OUTFILE}\'
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
rm -rf /tmp/vidfifo  &> /dev/null
rm -rf /tmp/audfifo1 &> /dev/null
rm -rf /tmp/audfifo2 &> /dev/null
rm -rf /tmp/catfifo  &> /dev/null

# create our fifo pipes
mkfifo /tmp/catfifo
mkfifo /tmp/vidfifo
mkfifo /tmp/audfifo1
mkfifo /tmp/audfifo2

#echo ">Starting progress bar"
"${PBCAT}" "${INFILE}" /tmp/catfifo &

#echo ">Starting audio decoder (mplayer)"
#Volnorm=2:1 => uses several samples for better accuracy. However results in ~1s of soft sound at the start of the file
"$MPLAYER" /tmp/catfifo -vc null -vo null -ao pcm:fast:file=/tmp/audfifo1:nowaveheader	\
  -af volnorm=2:1,resample=8000,channels=1:2:0:0:1:0,format=u8			\
  ${3} \
  -quiet &> aud.log &

#echo ">Starting video decoder (mencoder)"
#"$MENCODER" "$INFILE" -o /tmp/vidfifo -of rawvideo -ovc raw -oac copy 		\
#  -vf filmdint=io=23976:20000,hqdn3d,scale=160:100,hqdn3d,scale,format=rgb24	\
#  -quiet &> vid.log &
"$MENCODER" "$INFILE" -o /tmp/vidfifo -of rawvideo -ovc raw -oac copy     \
  -ofps 20.0 \
  -vf-add $VIDFILTERS \
  ${3} \
  -quiet &> vid.log &

#echo ">Starting audio normalizer"
"$NORMALIZER" /tmp/audfifo1 /tmp/audfifo2 &

#echo ">Making file header"
cat ${FMAGIC} > "$OUTFILE"
cat ${FONT} >> "$OUTFILE"

#SUBTITLER="cat"
#echo "SUBTITLER=${SUBTITLER}"
#echo ">Starting imagizer + subtitler + interleaver + compressor"
  "${IMAGIZER}" /tmp/vidfifo - 750 148 ${FONT} \
| "${SUBTITLER}" \
| "${ILEAVE}" - /tmp/audfifo2 - \
| "${COMPRESS}" 			\
>> "${OUTFILE}"
# "$PROGRESSBAR" size
#echo ">DONE"

# delete our fifo pipes
rm /tmp/catfifo
rm /tmp/vidfifo
rm /tmp/audfifo1
rm /tmp/audfifo2
