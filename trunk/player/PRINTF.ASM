TITLE printf
.386
.model flat, Cpp ; stdcall allows TASM to generate pro- and epi-log code
                 ; in procedures (set up stackframe, save USEd variables etc
ideal
include "globals.inc"

codeseg

proc _printf nolanguage near
  ; Pre: stack contains FormatStringPtr and arguments
  ; Post: The string has been printed to stdout
  ;        eax == ebx == num chars written
  ;       
  ; Note:
  ; We need nolanguage because we are going to do some nasty stack manipulation
  
  ; First we fixup the stack to make it look _vsprintf was called directly
  pop  eax                     ; eax = ret addr
  mov  [retaddr], eax          ; save the retaddr
  pop  eax                     ; eax = FormatStringPtr
  push esp                     ; argList
  push eax                     ; FormatStringPtr
  lea  eax, [strOutputBuf]     ; output buffer
  push eax
  ; actually call _vsprintf
  call _vsprintf               ;_vsprintf returns the #printed chars in eax
  ; Now we must undo the damage to the stack
  pop  ebx                     ; output buffer
  pop  ebx                     ; FormatStringPtr
  pop  esp                     ; argList
  sub  esp, 8                  ; fix up Stack pointer
  push ebx                     ; FormatStringPtr
  mov  ebx, [retaddr]
  push ebx
  ; stack is now back in order
  mov  ebx, eax                ; tmp store retval
  push edx
  lea  edx, [strOutputBuf]
  ; before we output the string add some terminators
  mov  [byte ptr edx+255], '$'
  mov  ah, 9
  int  021h                    ; write the string to the screen
  pop  edx
  mov  eax, ebx
  ret
endp _printf
dataseg
strOutputBuf db 256 dup ('#') ;(?)
retaddr dd 1 dup  (?)
end
