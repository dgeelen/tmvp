;---Procedures
global Initialise:proc 
global SetPalette:proc
global UpdateDisplay:proc
global KeyPressed:proc
global GetKey:proc
global Set80x25:proc
global Set80x50:proc
global NoBlink:proc
global DoBlink:proc
global UnInitialise:proc
global _vsprintf:proc
global _printf:proc
global int_print:proc
global InitVidBuf:proc
global ParseCmdLineArgs:proc
global _fopen:proc
;---Variables
global VideoBuffer:byte:8000
global Filehandle:dword