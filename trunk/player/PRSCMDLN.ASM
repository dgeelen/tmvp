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
  add  edi, 4                  ; ++argv[]
  mov  [dword ptr Filehandle], 0 ; set filehandle to NULL
if LPT_DAC eq True
  mov  [dword ptr LPTPortAddress], 1; default to using lpt1
endif
@@next_arg:
  mov  edx, [dword ptr edi]    ; ebx = ptr to argument
  test edx, edx                ; if NULL
  jz   @@done                  ; Then we're done
if DEBUG eq False ; True
  push edx
  push [dword ptr ArgDbgCnt]
  push offset ArgDbgMsg
  call _printf
  add  esp, 3*4
  inc  [dword ptr ArgDbgCnt]
endif
  cmp  [byte ptr edx], '-'
  jne  @@IsFileName
  ; parse options
if LPT_DAC eq True
  cmp  [dword ptr edx], 'tpl-'
  jne  @@notSetLPT
  add  edx, 4
  movzx eax, [byte ptr edx]
  sub  eax, '0'
  jbe  @@InvalidLPT
  cmp  eax, 4                  ; TODO: Read #ports from BDA
  ja   @@InvalidLPT
  mov  [dword ptr LPTPortAddress], eax
  push eax
  push offset strUsingLPTx
  call _printf
  add  esp, 2*4
  jmp  short @@ToNextArg
@@InvalidLPT:
  push eax
  push offset strInvalidLPT
  call _printf
  add  esp, 2*4
@@notSetLPT:
  jmp  short @@ToNextArg
endif

@@IsFileName:
  cmp  [dword ptr Filehandle], 0
  jne  short @@ToNextArg
  _fopen READONLY
  jc   short @@fopenerror
  mov  [dword ptr Filehandle], eax
  jmp  short @@ToNextArg
@@fopenerror:
  push eax                     ; error code
  push edx                     ; filename
  lea  ebx, [strFileOpenError]
  push ebx                     ; msg
  call _printf
  add  esp, 3*4
@@ToNextArg:
  add  edi, 4                  ; edi points to next arg
  jmp  @@next_arg              ; if [edi]==null then done
  @@done:

  xor  eax, eax
  cmp  [dword ptr Filehandle], eax
  jne  @@Exit
  ; Try to open default filename
  push offset strDefaultFilename; filename
  push offset strnoFileNameWarn
  call _printf
  add  esp, 2*4
  lea edx, [ strDefaultFilename ]
  _fopen READONLY          ; fopen( &filename, READONLY ) // Modifies eax, ebx, edx
  jc   @@DefaultFilenamefopenerror
  mov  [dword ptr Filehandle], eax
  jmp  short @@exit
@@DefaultFilenamefopenerror:
  push eax                     ; error #
  push offset strDefaultFilename; filename
  push offset strFileOpenError ; msg
  call _printf
  add  esp, 3*4
  @@exit:
  ret
endp
;==========================================================================================================
proc ParseCmdLineArgs_OLDCRAP
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
  lea  eax, [ArgDbgMsg]
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
  ;Let's fopen() it             ; we try to fopen it as soon as here
  _fopen READONLY               ; fopen() edx = filenameptr, modifies eax, ebx
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
  ; edx points to an option (eg -XXXXXX)
  cmp  [dword ptr edx], '-lpt'
  jne  @@invalidOption


  loopnz @@next_arg
@@InvalidOption:
  push  large offset strInvalidOption
  call _printf
  xor   eax, eax
  mov   [dword ptr FileHandle], eax
  jmp   short @@exit

@@FileOpen:
  add  edi, 4                  ; next index into arg ptr list
  loopnz @@next_arg
@@done:
  lea  eax, [dword ptr Filehandle]; might need it when we fopen() a file
  cmp  [dword ptr eax], 0
  jne  @@exit
  ; Try to open default filename
  lea  eax, [strnoFileNameWarn]
  push eax
  call _printf
  add  esp,4
  lea edx, [ strDefaultFilename ]
  _fopen READONLY          ; fopen( &filename, READONLY ) // Modifies eax, ebx, edx
  jc   @@DefaultFilenamefopenerror
  mov  [dword ptr Filehandle], eax
  jmp  short @@exit
@@DefaultFilenamefopenerror:
  push eax                     ; error #
  lea ebx, [ strDefaultFilename ]
  push ebx                     ; filename
  lea  edx, [strFileOpenError]
  push edx                     ; msg
  call _printf
  add  esp, 3*4
@@exit:
  ret
endp

dataseg
if DEBUG eq False ; True
  ArgDbgMsg db 'Argument %i = `%s''',newline,'$',0
  ArgDbgCnt dd 0
endif
  strnoFileNameWarn db 'No (valid) filename supplied, opening `%s''',newline,36,0
  strFileOpenError db 'Error opening `%s'': error #%i',newline,36,0
  strInvalidOption db 'Error: cannot parse `%s''!', newline, 36, 0
  strDefaultFilename db 'sample.tmv',0
  strUsingLPTx db 'Using lpt%i',newline,'$',0
  strInvalidLPT db 'Error: LPT%i is not a valid LPT port, using LPT1',newline,'$',0
end

;leave this here incase we ever again decide to use Tasm's bit manipulations
; ----8<--------------- code
;  testflag [byte ptr cmdLinParseFlags], mask FileLoaded
;  jnz  @@l1
;  setflag [byte ptr cmdLinParseFlags], mask FileLoaded
;  jmp  short @@l1
;@@parseCmd:
;  jmp short @@l1
;@@l1:
; ---->8--------------- code
;
; ----8<--------------- dataseg
; record cmdLinParseFlags FileLoaded:1=0
; ---->8--------------- dataseg