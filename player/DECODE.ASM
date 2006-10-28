TITLE Decode
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"

locals ;@@                     ; Use '@@' as local symbol prefix
                               ; '@@' is default
                               ; Must be 2 chars
;jumps

bufsize equ 1024*128

codeseg
proc decoder
  ; Pre:  global Filehandle:dword == File opened for reading
  ; Post: File has been decoded/played
  uses eax,ebx,ecx,edx
  
;// open debug file
  lea  eax, [strDebugFileName]
  _fopen eax, WRITEONLY
  mov [dword ptr DebugFilehandle], eax
  
;// test copy loop
@@loop1:
  lea edx, [buffer]
  mov ecx, [dword ptr Filehandle]
  call _fread, edx, 1, bufsize, ecx

  lea edx, [buffer]
  mov ecx, [dword ptr DebugFilehandle]
  call _fwrite, edx, 1, eax, ecx
  
  test eax,eax
  jnz @@loop1
  
  call _fclose, ecx
  
  ret
endp

dataseg
strDebugFileName db 'output.tst',0
strReadMsg db 'postread, eax=%i',newline,'$',0
strWriteMsg db 'postwrite, eax=%i',newline,'$',0
strOpenMsg db 'postopen, eax=%i',newline,'$',0
modechar_WO db ' w',0
  
udataseg
buffer db bufsize dup (?)
DebugFilehandle  dd ?
end
