TITLE Init
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"
;global Initialise:proc  ; declare public proc

codeseg
proc Initialise
  ; This calls all the individual initialisation routines
  ; while printing some status outputs
  ; Pre : -
  ; Post: eax == 0 on succes, eax > 0 indicates error code
  ;                           eax == -2 : Failed to open input
  ; Initialise all sub parts
  mov [dword ptr FileHandle], 0; make sure there is no ghostly file handle
  call ParseCmdLineArgs
  mov  eax, [dword ptr FileHandle]
  test eax, eax
  mov  eax, -2                 ; Failed to open input
  jz   @@exit_fail
if LPT_DAC eq True
  ; since commandline parsing succeeded, LPTPortAddress contains the port# we want
  ; to use and we can look up the correct address for the lpt-port from the BDA
  mov  eax, [dword ptr LPTPortAddress]
  push eax                     ; for use in status message later on
  dec  eax                     ; we count lpt from 1, bios from 0
  shl  eax, 1                  ; port address are 2 bytes
  add  eax, 400h + 08h         ; BDA (BIOS data area) : 400h + offset for lpt
  mov  ax, [word ptr eax]
  mov  [dword ptr LPTPortAddress], eax
  push eax                     ; address of lpt port
  push offset strUsingLPTportAddress
  call _printf
  add  esp, 3*4
endif

  call filemagic               ; test if this is a real TMVP file and what decoder to use
  test eax, eax
  jnz  @@exit_fail             ; nonzero indicates error code

  ; Calculate #decodable bytes
  mov  ebx, [dword ptr FileHandle]
  mov  al, 02h                 ; seek from end-of-file
  xor  ecx, ecx
  _fseek                       ; returns eax = new file position from start of file
  mov  [dword ptr FileSize    ], eax
  mov  ecx, eax
  sub  ecx, 4                  ; seek 4 bytes from EOF, so we can read #frames-skips
  mov  al, 00h                 ; seek from start-of-file
  _fseek                       ; returns eax = new file position from start of file
  mov  ecx, 4
  lea  edx, [SeekOffsetCount]  ; each 5 bytes
  _fread                       ; ebx == filehandle, ecx == nr bytes to read, ds:edx == ptr to buffer
  mov  eax, [dword ptr SeekOffsetCount] ; a*5
  shl  eax, 2                           ; = (a*4)
  add  eax, [dword ptr SeekOffsetCount] ;         + a
  mov  edx, [dword ptr FileSize]
  sub  edx, 2048 + 16 + 4      ; cut header and last 4 bytes
  sub  edx, eax                ; cut frame offsets
  mov  [dword ptr DecodeableBytes], edx ; God I hope this is correct ...
  mov  ecx, 16                 ; set ecx to just beyond fileheader or somewhere there about..
  mov  al, 0                   ;
  _fseek
;  mov  eax, -2
;  jmp  short @@exit_fail

  call Set80x50
  call SetFont
;;;;
;if not SetFont then still seek + 2048
;  mov  ebx, [dword ptr FileHandle]
;  xor  cx, cx
;  mov  dx, 2048
;  mov  al, CURRENT_FILE_POS
;  _fseek                       ;ebx=filehandle, cx:dx=new file pos, eax(al)=origin of move, returns dx:ax=new pos
;  xor  eax,eax
;;;;

  test eax, eax
  jz   @@FontSetOK
  push eax
  lea  eax, [strFontLoadFail]
  push eax
  call _printf
  add  esp, 4*2
@@l1:
  IF PRIVMODE eq True
    hlt
  ENDIF  ; disabled as it is a privileged instruction in pm, and gives us a #GP(0)
  call keypressed
  jnz  @@l1
  call getkey
  xor  eax, eax
  dec  eax
  jmp  short @@exit_fail
@@FontSetOK:
  call NoBlink
  call install_audio_interrupt ; assume no failure, or continue anyway :)
@@exit_succes:
  xor  eax, eax
@@exit_fail:
  ret
endp Initialise

proc UnInitialise ; Destroys ax
  ; This does any uninitialisation that needs doing
  ; Pre: eax == 0xffffffff indicates we should not CLS
  ; Post: Screen has been reset and cleared, interrupts vectors reset etc
  ; always returns eax==0
  uses ebx
  inc  eax
  jz   @@NoCLS
  call DoBlink                 ;
;  call Set80x25                ;
  call RestoreOldVideoMode
@@NoCLS:
  call uninstall_audio_interrupt
  ; we need to close the filehandle
  mov  ebx, [dword ptr FileHandle]  ; will be zero when fopen has failed
  _fclose                           ; So we close stdin...
  jnc   @@NofcloseError
  push eax
  lea  eax, [strFileCloseError]
  push eax
  call _printf
  add  esp, 2*4
;  xor  eax, eax               ; --- Promised to always return '0'
;  dec  eax                    ;  /
;  jmp  short @@exit           ; /
@@NofcloseError:
  xor  eax, eax
@@exit:
  ret
endp UnInitialise
dataseg
strFileCloseError db 'Error while closing videofile! (error #%i)',newline,36,0
strFontLoadFail db 'Error while loading font! (error #%i)', newline, 'press any key to quit.',newline, 36, 0
strUsingLPTportAddress db 'Read address 0x%08x for lpt%i',newline, 36, 0
end
