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
  ; Post: eax == 0 on succes, eax > 0 indicates error code
  ;                           eax == -2 : Failed to open input
  ; Initialise all sub parts (de-lzw/l77, vidbuf, sndPlay, palette etc): *TODO*
  mov [dword ptr FileHandle], 0; make sure there is no ghostly file handle
  call ParseCmdLineArgs
  mov  eax, [dword ptr FileHandle]
  test eax, eax
  mov  eax, -2                 ; Failed to open input
  jz   @@exit_fail
  call filemagic               ; test if this is a real TMVP file and what decoder to use
  test eax, eax
  jnz  @@exit_fail             ; nonzero indicates error code
  call InitVidBuf              ; Fill the videobuffer with '0' (or a test pat.)
  call Set80x50
  call SetFont
  test eax, eax
  jz   @@FontSetOK
  push eax
  lea  eax, [strFontLoadFail]
  push eax
  call _printf
  add  esp, 4*2
@@l1:
;  hlt  ; disabled as it is a privileged instruction in pm, and gives us a #GP(0)
  call keypressed
  jnz  @@l1
  call getkey
  xor  eax, eax
  dec  eax
  jmp  short @@exit_fail
@@FontSetOK:
  call NoBlink
  call install_audio_interrupt ; assume no failure
@@exit_succes:
  xor  eax, eax
@@exit_fail:
  ret
endp Initialise

proc UnInitialise ; Destroys ax
  ; This does any uninitialisation that needs doing
  ; Pre: eax == 0xffffffff indicates we should not CLS
  ; Post: Screen has been reset and cleared, interrupts vectors reset etc
  ; always returns eax==0
  uses ebx
  inc  eax
  jz   @@NoCLS
  call DoBlink                 ;
;  call Set80x25                ;
  call RestoreOldVideoMode
@@NoCLS:
  call uninstall_audio_interrupt
  ; we need to close the filehandle
  mov  ebx, [dword ptr FileHandle]  ; will be zero when fopen has failed
  _fclose                           ; So we close stdin...
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
strFontLoadFail db 'Error while loading font! (error #%i)', newline, 'press any key to quit.',newline, 36, 0
end
