TITLE printf
.386
.model flat, Cpp ; stdcall allows TASM to generate pro- and epi-log code
                 ; in procedures (set up stackframe, save USEd variables etc
ideal
include "globals.inc"

codeseg

proc _printf near
  ; Pre: stack contains FormatStringPtr and arguments
  ; Post: The string has been printed to stdout
  arg  FormatStringPtr:dword
; Can't use this here :(
;  push bp
;  mov  bp, sp
  pop  eax                     ; eax = ret addr
  mov  [retaddr], eax          ; save the retaddr
  pop  eax                     ; eax = FormatStringPtr
  push esp                     ; argList
  push eax                     ; FormatStringPtr
;  push dword offset strOutputBuf
  lea  edx, [strOutputBuf]     ; output buffer
  push edx
  mov  [byte ptr edx+255], '$'
;  call _vsprintf 
  ;_vsprintf returns the #printed chars in eax
  call int_print
  ;now we need to 'unpop' the argList and outputbuf ptrs
  pop  eax                     ; output buffer
  pop  ebx                     ; FormatStringPtr
  pop  eax                     ; argList
  push ebx                     ; FormatStringPtr
  mov  eax, [retaddr]
  push eax
  ; stack is now back in order
  lea  edx, [strOutputBuf]
  ; before we output the string add some terminators
  mov  [byte ptr edx+255], '$'
  mov  ah, 9
  int  21h
; Can't use this here :(
;  pop  bp
  ret
endp _printf
dataseg
strOutputBuf db 256 dup ('#') ;(?)
retaddr dd 1 dup  (?)
end
