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
  call ParseCmdLineArgs
  jmp @@Terminate
  call Initialise
  lea  eax, [strIntroMsg]
  push eax                     ; can't use extended call- >) NoLanguage proc.
  call _printf                 ; destroys eax, ebx 
  
  lea  eax, [strPressKey]
  push eax
  call _printf
 
  ; Wait for a keypress
@l1:
  call keypressed
;  jnz  @l1  
  
  ; Terminate
@@Terminate:  
  call UnInitialise
  lea  eax, [strExitMsg]
  push eax
  call _printf
  mov  eax, 4C00h
  int  21h
endp tmvp

;---Data Section---------------------------------------------------------------

dataseg
  strIntroMsg db '-[ TMVP v0.1, coding by Hackykid & Da ',159,'ox.    Assembled on ',??date,' @',??time, ' ]-','$',0
;  strExitMsg db 'Thank you for using this program, we hope you were impressed (We sure were ;)',newline,36,0
  strExitMsg db 'This is a WIP-build, use at your own risk!',newline,36,0
  strPressKey db 'Press any key to exit...',newline,'$',0
  ; zero terminated for _vsprintf, dollar terminated for DOS
  strTestsprintf db 'this is a number %i and this too %i,',CrLf,'and a ''%s''',CrLf,'$',0
;  strTestsprintf db 'this is a number %i and this too %i',0ah,0dh,'$',0  
  someChars db 'asdasdasdadasd',0
  
;---GLOBAL VARIABLES---
udataseg
VideoBuffer db 8000 dup (?)
Filehandle dd ?
end tmvp ; needs entry point

