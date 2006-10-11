title aavp
.386
.model flat
ideal
include "globals.inc"

codeseg
proc aavp
  EntryPoint:
;  int  3 no PM debugging ?
;  call Initialise
  
  ; Print the hello world thing
  lea  edx, [strIntroMsg]
  mov  ah, 9
  int  21h
  
  ; Wait for a keypress
;@l1:
 ; call keypressed
  ;jnz  @l1  
  
;  call UnInitialise
  
  push 1234
  push 5678
  push dword offset strTestsprintf
  call _printf
  
  ; Terminate
  mov  eax, 4C00h
  int  21h
endp aavp

;---Data Section---------------------------------------------------------------

dataseg
  strIntroMsg db 'aavp v0.1 is loading...',0dh,0ah,'$'
  ; zero terminated for _vsprintf, dollar terminated for DOS
  strTestsprintf db 'this is a number %i and this too %i',0dh,0ah,'$',0 
  
end aavp ; needs entry point

