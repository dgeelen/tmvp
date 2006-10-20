title fopen()
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"

locals ;@@                     ; Use '@@' as local symbol prefix
                               ; '@@' is default
                               ; Must be 2 chars
jumps                          ; Auto-convert out-of-range                       

codeseg
proc _fopen
  ; Pre: Stack contains 2 ptr to an asciiz string, the first pointing to a filename 
  ;      the second a mode char
  ; Post: CF is clear on succes, and a filehandle is returned in eax
  ;       CF is set on failure, and an error code is returned in eax
  uses ebx, edx
  arg @@filenameptr:dword, @@modecharptr:dword
  push [dword ptr @@modecharptr]
  push [dword ptr @@filenameptr]
  lea  eax, [strOpeningFilename]
  push eax
  call _printf                 ; destroys eax, ebx
  add  esp, 2*4
  ; delete above lines to disable dbg
  mov  eax, [dword ptr @@modecharptr]
  mov  bx, [word ptr eax]
  xor  eax, eax
  cmp  bl, 'r'
  sete al
  cmp  bh, 'w'
  sete ah
  shl  ah, 1
  or   al, ah
  dec  al
  mov  edx,[dword ptr @@filenameptr]; ptr to filename
  mov  ah, 03Dh                ; open existing file
  int  021h                    ; try to open it
  ; if succes ax=filehandle, else ax=error#
@@exit:
  ret
endp
dataseg
strOpeningFilename db '_fopen(): Opening `%s'' with mode `%s''',newline,36,0
end
