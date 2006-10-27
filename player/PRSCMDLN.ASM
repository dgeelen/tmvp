title Parse Commandline
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
proc ParseCmdLineArgs 
  ; Pre:  ESI == The number of command line arguments
  ;       EDI == Near pointer to an array of near pointers (argv)
  ;              Terminated by NULL pointer
  ; Post: All commandline arguments have been parsed, appropriate actions
  ;       haven been taken, and internal variables have been set to specified
  ;       parameters
  ;       If a filename was given and it was possible to open that file
  ;       a filehandle has been placed in [FileHandle]
  uses eax, ebx, ecx, edx
  mov  ecx, esi                ; ecx = #arguments
  dec  ecx                     ; -> Not interested in own name
  add  edi, 4                  ; /
  mov  [dword ptr Filehandle], 0 ; set filehandle to NULL
@@next_arg:
  mov  ebx, [dword ptr edi]    ; ebx = ptr to argument
  test ebx, ebx                ; if NULL
  jz   @@done                  ; Then we're done
  push ebx                     ; push ptr to asciiz string
  mov  eax, esi                ; eax = #args
  sub  eax, ecx                ; eax = current arg
  push eax                     
  lea  eax, [cmdLinArg]        
  push eax                     ; Push asciiz ptr
  call _printf                 ; print it
  add  esp, 8 ; 12                 ; clear up stack
;--- Now the argument has been printed, let's take a look and see if it's a
;    possible filename
  pop  edx                     ; This is the addr of the ascizz cmdlin arg
  cmp  [byte ptr edx], '-'
  je   @@parseCmd
  ;Possible filename
  lea  eax, [dword ptr Filehandle]; might need it when we fopen() a file
  cmp  [dword ptr eax], 0
  jne  @@FileOpen
  ;Let's fopen() it            ; we try to fopen it as soon as here
  lea  ebx, [modechar_RO]      ; so that if one file fails an other may 
  push ebx edx
  call _fopen                  ; Can't call with extended syntax
  pop  edx ebx                 ; actually order unimportant
  jc   @@fopenerror
  mov  [dword ptr Filehandle], eax
  jmp  short @@FileOpen
@@fopenerror:
  push eax                     ; error code
  push edx                     ; filename 
  lea  ebx, [strFileOpenError]
  push ebx                     ; msg
  call _printf
  add  esp, 3*4
  jmp  short @@FileOpen        ; if everything went ok, the file is now open
@@parseCmd:
  
@@FileOpen:
  add  edi, 4                  ; next index into arg ptr list
  loopnz @@next_arg
@@done:
  lea  eax, [dword ptr Filehandle]; might need it when we fopen() a file
  cmp  [dword ptr eax], 0
  jne  @@FileNameGiven
  ; Try to open default filename 'output.aa'
  lea  eax, [strnoFileNameWarn]
  push eax
  call _printf
  add  esp,4
  lea eax, [ strDefaultFilename ]
  lea ebx, [ modechar_RO ]
  push ebx
  push eax
  call _fopen                  ; eax == error code or filehandle
  pop  ebx                     ; ebx=filename
  pop  ecx                     ; mode char
  jc   @@DefaultFilenamefopenerror
  mov  [dword ptr Filehandle], eax
  jmp  short @@FileNameGiven
@@DefaultFilenamefopenerror:
  push eax                     ; error #
  push ebx                     ; filename
  lea  edx, [strFileOpenError]
  push edx                     ; msg
  call _printf
  add  esp, 3*4
@@FileNameGiven:  
  ret
endp

dataseg
  cmdLinArg db 'Argument %i = ''%s''',newline,'$',0
  strnoFileNameWarn db 'No (valid) filename supplied, opening `output.aa''',newline,36,0
  strFileOpenError db 'Error opening `%s'': error #%i',newline,36,0
  record cmdLinParseFlags FileLoaded:1=0
  strDefaultFilename db 'output.aa',0
  modechar_RO db 'r ',0
end

  testflag [byte ptr cmdLinParseFlags], mask FileLoaded
  jnz  @@l1
  setflag [byte ptr cmdLinParseFlags], mask FileLoaded
  jmp  short @@l1
@@parseCmd:  
  jmp short @@l1
@@l1:
