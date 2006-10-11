TITLE smallfunctions
.386
.model flat
ideal
include "globals.inc"

codeseg

proc _printf
  ; Pre: stack contains FormatStringPtr and arguments
  ; Post: The string has been printed to stdout
  arg  FormatStringPtr:dword
  push bp
  mov  bp, sp
  pop  eax                     ; eax = FormatStringPtr
  push esp                     ; argList
  push eax                     ; format
  push dword offset strOutputBuf
  call _vsprintf
  lea  edx, [strOutputBuf]
  mov  ah, 9
  int  21h  
  pop  bp
  ret
endp _printf
dataseg
strOutputBuf db 256 dup (?)
end
