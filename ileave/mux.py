import sys
import os.path
__author__  = 'da_fox@junkmail.com'
__version__ = '0.0.1'

def printStatus(part, full):
  progbar=""
  j=0
  while j < 40:
    progbar=progbar+'\b'
    j=j+1
  print progbar,
  i=(((part*100)/full)*73)/100
  j=0
  while j < i:
    progbar=progbar+'#'
    j=j+1
  while j < 73:
    progbar=progbar+'.'
    j=j+1
  i=((part*100)/full)
  if i<10:
    j='00'+str(i)
  else:
    if i<100:
      j='0'+str(i)
    else:
      j=str(i)
  print progbar+' '+j+'%',
  sys.stdout.flush()
   
if __name__ == "__main__":
  if len(sys.argv) < 5 :
    print "Sorry, I need at least 3 filenames and an integer argument:"
    print "  python mux.python raw_video.aa wavdata.wav output.aa AUDIO_SAMPLE_SIZE"
  else:
    try:
      video_in = open(sys.argv[1], "rb")
    except IOError:
      print "Couldn't open "+sys.argv[1]
      sys.exit(-1)
    try:
      audio_in = open(sys.argv[2], "rb")
    except IOError:
      print "Couldn't open "+sys.argv[2]
      sys.exit(-1)
    try:
      video_out= open(sys.argv[3], "wb")
    except IOError:
      print "Couldn't open "+sys.argv[3]
      sys.exit(-1)
    AudSampSize=int(sys.argv[4])
    VideoFileSize=os.path.getsize(sys.argv[1])
    print "Now muxing file..."
    print " ......................................................................... 000%",
    done=False
    bytesread=0
    vid_in = video_in.read(2048+16) #Fontdata + header
    bytesread=bytesread+len(vid_in)
    video_out.write(vid_in)
    while not done==True:
      vid_in = video_in.read(48+8000)
      bytesread=bytesread+len(vid_in)
      wav_in = audio_in.read(AudSampSize)
      if (len(vid_in)==0):
        done=True
      if (len(wav_in)<AudSampSize):
        #fill with NULL 
        for i in range(AudSampSize - len(wav_in)):
          wav_in=wav_in+' '
      video_out.write(vid_in)
      video_out.write(wav_in)
      vid_in = video_in.read(AudSampSize) # Read null bytes
      bytesread=bytesread+len(vid_in)
      printStatus(bytesread, VideoFileSize)
