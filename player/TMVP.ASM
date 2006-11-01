title tmvp
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"

locals ;@@                     ; Use '@@' as local symbol prefix
                               ; '@@' is default
                               ; Must be 2 chars
;jumps                         ; Auto-convert out-of-range

codeseg
proc tmvp
  EntryPoint:
;  int  3 no PM debugging ?
;  call ParseCmdLineArgs
;  jmp @@Terminate
  lea  eax, [strIntroMsg]
  push eax                     ; can't use extended call- >) NoLanguage proc.
  call _printf                 ; destroys eax, ebx
  add  esp, 4*1
  call Initialise
  test  eax, eax
  jnz   @@InitialisationError
  lea  eax, [strInitOk]
  push eax
  call _printf
  add  esp, 4*1
  ; Wait for a keypress
@l1:
  mov  cx, 0ffffh
  @l2:
  loopnz @l2
  call keypressed
  jnz  @l1

  call fileop_function_test
  ;// test decoder
  call decoder

;  Everything else is set up, turn down the lights and switch on the screen ;)
;  call Set80x50                ; Set highres Textmode
;  call NoBlink                 ; Disable the blink bit

   ; * DO STUFF *
   ; Like call Decoder or smurfin

; Terminate
@@Terminate:
  call UnInitialise
  lea  eax, [strExitMsg]
  push eax
  call _printf
  add  esp, 4*1
  mov  eax, 4C00h
  int  21h                     ; Does not return

@@InitialisationError:         ; eax == error nr, print msg and jmp @@exit
@@FailedToOpenInput:
  cmp  eax, -2
  jne  @@UnknownError
  lea  edx, [strFailedToOpenInput]
  mov  ah, 09h
  int  021h
  xor  eax, eax
  dec  eax
  jmp  short @@Terminate
@@UnknownError:
  lea  edx, [strUnknownError]
  mov  ah, 09h
  int  021h
  jmp  short @@Terminate
endp tmvp

;---Data Section---------------------------------------------------------------

dataseg
;strings are zero terminated for _vsprintf, dollar terminated for DOS
  strIntroMsg db '-[ TMVP v0.1, coding by Hackykid & Da ',159,'ox.   Assembled on ',??date,' @ ',??time, ' ]-','$',0
;  strExitMsg db 'Thank you for using this program, we hope you were impressed (We sure were ;)',newline,36,0
  strExitMsg db 'This is a WIP-build, use at your own risk!',newline,36,0
  strInitOk db 'Initialisation complete, press any key to begin playback.',newline,'$',0
  strPressKey db 'Press any key to continue.',newline,36,0
  strFailedToOpenInput db 'Sorry, unable to open any input file.',newline,'Try something like `tmvp sample.aa''',newline,'$',0
  strUnknownError db 'Sorry, some error we did not anticipate occured.',newline,'$',0


;---GLOBAL VARIABLES---
udataseg
Filehandle  dd ?
end tmvp ; needs entry point

