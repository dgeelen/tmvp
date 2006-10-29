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
  lea  edx, [strDebugFileName]
  _fopen WRITEONLY
  mov [dword ptr DebugFilehandle], eax
  lea  edx, [buffer]
;// test copy loop
@@loop1:
  mov  ebx, [dword ptr Filehandle]
  mov  ecx, bufsize
  _fread                       ; ebx == filehandle, ecx == nr bytes to write, edx == ptr to buffer
  test eax,eax
  jz  @@done
  mov  ecx, eax
  mov  ebx, [dword ptr DebugFilehandle]
  _fwrite                      ; ebx == filehandle, ecx == nr bytes to write, edx == ptr to buffer
@@done:
  _fclose
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
