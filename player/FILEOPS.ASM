title File operations such as open, close, read and write
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

; ****************************************************************************
;                         B I G   F A T   W A R N I N G
;  All these functions will (*soon*) be rewritten as macros and as such their
;  implemtations and arguments may (will) be subject sudden, unexpected
;  change so please don't use them (too much)...
;                           C A V E A T   E M P T O R
; ****************************************************************************

proc _fwrite
  ; Pre : Stack contains a ptr to a buffer, the size of items, the number of
   ;      items to read and a file handle
  uses ebx, ecx, edx
  arg @@pbuf:dword, @@size:dword, @@nmemb:dword, @@filehandle:dword

  push [dword ptr @@filehandle]
  lea  eax, [strfwrite]
  push eax
  call _printf                 ; destroys eax, ebx
  add  esp, 2*4
  ; delete above lines to disable dbg

  mov  ecx, [dword ptr @@size]
  mov  edx, [dword ptr @@nmemb]
  imul ecx, edx                ; ecx = nr bytes to read
  mov  edx, [dword ptr @@pbuf]
  mov  ebx, [dword ptr @@filehandle]
  xor  eax, eax                ; clear eax in order to accurately show the
  mov  ah, 040h                ; number of bytes write
  int  021h
  ret
endp

proc _fread
  ; Pre : Stack contains a ptr to a buffer of size at least size * nmemb
  ;       bytes, the required size, the number of items to read and
  ;       a file handle
  ; Post: The buffer has been filled with no more than size * nmemb bytes
  ;       CF clear indicates eax contains actual nr bytes read
  ;                          eax == 0 => EOF
  ;       CF set indicates an error occured
  ;              eax == FFFFFFFFh indicates invalid input
  ;              other wise ax == DOS error nr.
uses ebx, ecx, edx
arg @@pbuf:dword, @@size:dword, @@nmemb:dword, @@filehandle:dword

  push [dword ptr @@filehandle]
  lea  eax, [strfread]
  push eax
  call _printf                 ; destroys eax, ebx
  add  esp, 2*4
  ; delete above lines to disable dbg

  mov  ecx, [dword ptr @@size]
  mov  edx, [dword ptr @@nmemb]
  imul ecx, edx                ; ecx = Low(Result) = nr bytes to read <= 65535
  mov  edx, [dword ptr @@pbuf]
  mov  ebx, [dword ptr @@filehandle]
  xor  eax, eax                ; clear eax in order to accurately show the
  mov  ah, 03Fh                ; number of bytes read
  int  021h
  ret
endp

proc _fclose
  ; pre : Valid filehandle on the stack
  ; post: CF is clear on succes, and file is closed.
  ;                              eax is preserved
  ;       CF is set on failure, and file is in undetermined state
  ;                             eax = error code
  uses ebx
  arg @@filehandle:dword

  push [dword ptr @@filehandle]
  lea  eax, [strfclose]
  push eax
  call _printf                 ; destroys eax, ebx
  add  esp, 2*4
  ; delete above lines to disable dbg

  push eax
  xor  eax, eax
  mov  ah, 03Eh
  mov  ebx, [dword ptr @@filehandle]
  int  021h
  jc    @@exit
  mov  eax, [dword ptr esp]
@@exit:
  inc esp                      ; inc does not affect CF
  inc esp                      ; (or so I have been told)
  inc esp
  inc esp
  ret
endp

proc _fopen_DEPRECATED
  ; Pre: Stack contains 2 ptr to an asciiz string, the first pointing to a filename
  ;      the second a mode char
  ; Post: CF is clear on succes, and a filehandle is returned in eax
  ;       CF is set on failure, and an error code is returned in eax (al?)
  uses ebx, edx
  arg @@filenameptr:dword, @@modecharptr:dword
  push [dword ptr @@modecharptr]
  push [dword ptr @@filenameptr]
  lea  eax, [strOpeningFilename]
  push eax
  call _printf                 ; destroys eax, ebx
  add  esp, 3*4
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
strfclose db '_fclose(): Closing filehandle %i',newline,36,0
strfread db '_fread(): reading from filehandle %i',newline,36,0
strfwrite db '_fwrite(): writing to filehandle %i',newline,36,0
end
