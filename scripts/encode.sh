#!/bin/sh

MPLAYER="`which mplayer 2> /dev/null`"
MENCODER="`which mencoder 2> /dev/null`"
ILEAVE="../ileave/ileave"
PBCAT="../pbcat/pbcat"
IMAGIZER="../imagizer1/imagize"
COMPRESS="../compress1/lz77s"
NORMALIZER="../normalizer/normalize"
FMAGIC="fmagic.dat"
FONT="asc_ord_hi.fon"
VIDFILTERS="harddup,expand=:::::4/3,scale=160:100,format=rgb24"
SUBTITLER="`which cat`"
COMPRESS_HAX="333 1" # set a seekpoint every 333 frames (~15sec), and use 'keyframes'

if [ "${2}" != "" ] ; then # FIXME: This (Actually: all arguments) need to be handled properly
  if [ "${2}" == "dafox" ] || [ "${2}" == "simon" ] ; then
    UNAME="${2}"
  else
    COMPRESS_HAX="${2}"
  fi
fi

if [ "${UNAME}" == "" ] ; then
  UNAME="${3}"
fi

if [ "${UNAME}" == "dafox" ] ; then
  IMAGIZER="/home/dafox/Projects/tmvp/imagizer1/kdev/imagize/optimized/src/imagize"
  SUBTITLER="`which cat`" #"/home/dafox/Projects/tmvp/subtitler/kdev/subtitler/optimized/src/subtitler"
  FONT="nes_chars.fnt"
  VIDFILTERS="harddup,expand=:::::4/3,scale=320:-2,pp7=0:1,unsharp,2xsai,scale=-1:-2,hqdn3d,dsize=160:100,scale=160:-2,format=rgb24"
  #VIDFILTERS="harddup,pp7=0:1,expand=:::::4/3,dsize=160:100,scale=160:100,format=rgb24"
  #VIDFILTERS="-sws 7 -vf-add spp=6,scale=320:-2,2xsai,scale=320:-2,spp=6,scale=320:-2,2xsai,scale=320:-2,spp=6,scale=320:-2,2xsai,scale=320:-2,spp=6,dsize=160:100,scale=160:-2,format=rgb24"
  #VIDFILTERS="-sws 7 -vf-add spp=6,scale=320:-2,2xsai,scale=320:-2,spp=6,scale=320:-2,2xsai,scale=320:-2,spp=6,scale=320:-2,2xsai,scale=320:-2,spp=6,expand=:::::4/3,dsize=160:100,scale=160:-2,format=rgb24"

  #Nextline is for the old 386 inverted Black and White
  #VIDFILTERS="harddup,expand=:::::4/3,eq2=1:-1:0:0,scale=320:-2,pp7=0:1,unsharp,2xsai,scale=-1:-2,hqdn3d,dsize=160:100,scale=160:-2,format=rgb24"
  
  NORMALIZER_OPTS="255"
  COMPRESS_HAX="333 0"
elif [ "${UNAME}" == "simon" ] ; then
  VIDFILTERS="expand=:::::4/3,hqdn3d,scale=160:100,hqdn3d,dsize=160:100,scale=-1:-2,format=rgb24,harddup"
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
  echo "Error: pbcat not found!"
  exit
fi

if [ ! -f "${SUBTITLER}" ] ; then
  echo "Error: Subtitler not found!"
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

TMPDIR=`mktemp -td tmvenc-XXXXXX`

# create our fifo pipes
mkfifo "${TMPDIR}"/vidfifo
mkfifo "${TMPDIR}"/audfifo1
mkfifo "${TMPDIR}"/audfifo2

#echo ">Starting progress bar"
#"${PBCAT}" "${INFILE}" "${TMPDIR}"/catfifo &

echo -n Starting
echo -n .

#echo ">Starting audio decoder (mplayer)"
#Volnorm=2:1 => uses several samples for better accuracy. However results in ~1s of soft sound at the start of the file
"$MPLAYER" "${INFILE}" -vc null -vo null -ao pcm:fast:file="${TMPDIR}"/audfifo1:nowaveheader \
  -af volnorm=2:1,resample=8000,channels=1:2:0:0:1:0,format=u8 \
  ${3} ${5} \
  -quiet &> aud.log &

MPLAYER_PID="`echo $!`"

echo -n .

#echo ">Starting video decoder (mencoder)"
"$MENCODER" "$INFILE" -o "${TMPDIR}"/vidfifo -of rawvideo -ovc raw -oac pcm -channels 1 -srate 4000 \
  -ofps 20.0 -noautoexpand \
  -vf-add $VIDFILTERS \
  ${3} ${4} \
  2>&1 \
  | "${PBCAT}" \
  > vid.log &

MENCODER_PID="`echo $!`"

echo -n .



#cat ${TMPDIR}/catfifo &

#echo ">Starting audio normalizer"
"$NORMALIZER" "${TMPDIR}"/audfifo1 "${TMPDIR}"/audfifo2 ${NORMALIZER_OPTS}&

NORMALIZER_PID="`echo $!`"

#echo ">Making file header"
cat ${FMAGIC} > "$OUTFILE"
cat ${FONT} >> "$OUTFILE"

echo .

#SUBTITLER="cat"
#echo "SUBTITLER=${SUBTITLER}"
#echo ">Starting imagizer + subtitler + interleaver + compressor"
if [ "$3" == "" ] ; then p="1024" ; else p="$3" ; fi #Set the pallete change threshold
if [ "$4" == "" ] ; then c="148" ; else c="$4" ; fi #Set the character change threshold
  "${IMAGIZER}" "${TMPDIR}"/vidfifo -p "$p" -c "$c" -f ${FONT} \
| "${SUBTITLER}" \
| "${ILEAVE}" - "${TMPDIR}"/audfifo2 - \
| "${COMPRESS}" ${COMPRESS_HAX} \
>> "${OUTFILE}"
# "$PROGRESSBAR" size
#echo ">DONE"

# delete our fifo pipes
rm "${TMPDIR}"/vidfifo
rm "${TMPDIR}"/audfifo1
rm "${TMPDIR}"/audfifo2

rmdir "${TMPDIR}"
echo "${MPLAYER_PID}" "${MENCODER_PID}" "${NORMALIZER_PID}"
