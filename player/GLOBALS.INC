;---Procedures
global Initialise:proc
global UpdateDisplay:proc
global KeyPressed:proc
global GetKey:proc
global Set80x25:proc
global Set80x50:proc
global NoBlink:proc
global DoBlink:proc
global SetFont:proc
global UnInitialise:proc
global _vsprintf:proc
global _printf:proc
global int_print:proc
global InitVidBuf:proc
global ParseCmdLineArgs:proc
global RestoreOldVideoMode:proc
global ClearScreen:proc
global install_audio_interrupt:proc
global uninstall_audio_interrupt:proc
global Decoder:proc
global fileop_function_test:proc
global filemagic:proc

;---Variables
global DecodeBufHead:dword
global DecodeBufTail:dword
global AudioDataPtr:dword
global DecodeBuffer:byte:DECBUF_SIZE
global AudioBuffer:byte:AUDSAMP_SIZE * RENDER_AHEAD
global Filehandle:dword
global DecoderMode:dword
global FileSize:dword
global DecodeableBytes:dword
global SeekOffsetCount:dword
global FrameCounter:dword
if BEEP_ON_FREAD eq True
  global WaveSample:dword
endif