TITLE Init
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"
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
  call InitVidBuf              ; Fill the videobuffer with '0' (or a test pat.)
  jmp  short @@exit_succes     ; *DEBUG*
  call Set80x50                ; Set highres Textmode
  call NoBlink                 ; Disable the blink bit

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
  ; always returns eax==0
  uses ebx
  call DoBlink
;  call Set80x25
  ; we need to close the filehandle 
  push [dword ptr FileHandle]  ; will be zero when fopen has failed
  call  _fclose                ; but DOS doesn't seem to care?
  pop  ebx
  jnc   @@NofcloseError
  push eax
  lea  eax, [strFileCloseError]
  push eax
  call _printf
  add  esp, 2*4
;  xor  eax, eax               ; --- Promised to always return '0' 
;  dec  eax                    ;  /
;  jmp  short @@exit           ; /
@@NofcloseError:  
  xor  eax, eax
@@exit:
  ret
endp UnInitialise
dataseg
strFileCloseError db 'Error while closing videofile! (error #%i)',newline,36,0
end
