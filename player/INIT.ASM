TITLE Init
.386
.model flat, Cpp
ideal
include "globals.inc"
;global Initialise:proc  ; declare public proc

codeseg
proc Initialise
  ; This calls all the individual initialisation routines
  ; while printing some status outputs
  ; Pre : -
  ; Post: ax == 0 on succes, ax > 0 indicates error code
  
  ; Initialise all sub parts (de-lzw/l77, vidbuf, sndPlay, palette etc): *TODO*
  
  ; Everything else is set up, turn down the lights and switch on the screen ;)

  call ParseCmdLineArgs 
  jmp  short @@exit_succes     ; *DEBUG*
  call Set80x50                ; Set highres Textmode
  call NoBlink                 ; Disable the blink bit
  call InitVidBuf              ; Fill the videobuffer with '0' (or a test pat.)
;  call Flip
  jmp  short @@exit_succes
@@exit_fail:
  xor  ax, ax
  inc  ax
  ret
@@exit_succes:
  xor  ax, ax
  ret
endp Initialise

proc UnInitialise ; Destroys ax
  ; This does any uninitialisation that needs doing
  ; Pre: -
  ; Post: Screen has been reset and cleared, interrupts vectors reset etc
  ; always returns ax==0
  call DoBlink
;  call Set80x25
  xor  ax, ax
  ret
endp UnInitialise
;proc malloc   ; NOTE: Use 'uninitialised data'
;  arg size:dword=ParamSize
;  push bp
;  mov  bp,sp
;  
;  pop  bp
;  ret ParamSize
;endp malloc
dataseg
end
