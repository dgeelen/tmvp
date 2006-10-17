title tmvp
.386
.model flat, Cpp
ideal
include "globals.inc"

codeseg
proc tmvp
  EntryPoint:
;  int  3 no PM debugging ?
  lea  edx, [strIntroMsg]
  mov  ah, 9
  int  21h
;  call Initialise
  
  lea  edx, [strPressKey]
  mov  ah, 9
  int  21h
 
  ; Wait for a keypress
;@l1:
;  call keypressed
;  jnz  @l1  
  
;  call UnInitialise
  
  mov  eax, 12345
  push eax
  mov  eax, 56789
  push eax
  lea  edx, [strTestsprintf]
  push edx
  call _printf
  add  esp, 12 ;fixup the stack
  
  ; Terminate
  mov  eax, 4C00h
  int  21h
endp tmvp

;---Data Section---------------------------------------------------------------

dataseg
  strIntroMsg db 'tmvp v0.1 is loading...       [assembled: ',??date,'@',??time,']',0dh,0ah,'$'
  strPressKey db 'Press any key to exit...',0dh,0ah,'$'
  ; zero terminated for _vsprintf, dollar terminated for DOS
  strTestsprintf db 'this is a number %i and this too %i',0dh,0ah,'$',0 
  
end tmvp ; needs entry point

