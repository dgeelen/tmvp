title Audio handler Interrupt Service Routine
.386
.model flat, Cpp               ; do we reallly want C calling conventions in an interrupt?
ideal
include "globals.inc"
include "macros.mac"

locals ;@@                     ; Use '@@' as local symbol prefix
                               ; '@@' is default
                               ; Must be 2 chars
jumps                          ; Auto-convert out-of-range

codeseg

proc uninstall_audio_interrupt
  ; Pre: A ISR was installed
  ; Post: The ISR is restored to it's original ptr
  xor  eax, eax
  cmp  [dword ptr cs:old_protmode_int08], eax
  jne  @@UninstallProtectedmode
  cmp  [dword ptr cs:old_protmode_int08], eax
  je   @@NoInt
@@UninstallProtectedmode:
  ; CX:EDX is selector:offset
  mov  edx, [dword ptr cs:old_protmode_int08  ]
  mov   cx, [ word ptr cs:old_protmode_int08+4]
  mov   ax, 0205h
  mov   bl, 08h
  int  031h                    ; set interrupt vector
  jnc  @@UnprogramTimers
  lea  eax, [strProtmodeUninstallError]
  push eax
  call _printf
  add  esp, 4
  jmp  short @@exit
@@UnprogramTimers:
  ; First timer 2
  mov al, 10110110B
  out 043h, al
  in  al, 061h
  and al, 0FCh                 ; disconnect from speaker
  out 061h, al
  ; Then timer 0
  mov al, 00110110B
  out 043h, al
  mov al, 0ffh                 ; 18.2 hz (?)
  out 040h,al
  out 040h,al

  ;finally print some stats
  lea  eax, [strNrServicedInts]
  push [dword ptr NrServicedInts]
  push eax
  call _printf
  add  esp, 4*2
  jmp short @@exit
@@NoInt:
  lea  eax, [strNoIntUninstallError]
  push eax
  call _printf
  add  esp, 4
@@exit:
  ret
endp

proc install_audio_interrupt
  ; Pre: -
  ; Post: ??
  ; Notes:
  ; Q&A: Q. Does WDosX automagically restore interrupts after the program terminates
  ;         and/or do I neeed to restore Protected Mode interrupts, since when the
  ;         program ends the Protected Mode environment will discarded anyway ?
  ;      A. *TODO*
  uses eax, ebx, ecx, edx, edi, es
  xor  eax, eax
  mov  [dword ptr NrServicedInts], eax
  mov  [dword ptr ChainTimer], eax
  ; SET PROTECTED MODE INTERRUPT
  mov   ax, 0204h              ; get protected mode interrupt vector
  mov   bl, 08h                ; of int08
  int   031h                   ; get it
  jc    @@pError
  ; CX:EDX == selector:offset
  mov  [dword ptr cs:old_protmode_int08    ], edx
  mov  [ word ptr cs:old_protmode_int08 + 4], cx
  ; store DS for use in interrupt
  mov  [word ptr cs:protmode_ds], ds
  ; set new interrupt vector
  mov   ax, 0205h
  mov   bl, 08h
  mov   cx, cs
  mov  edx, large offset audio_int_protmode_simpletest
  int  031h
  jc   @@pError
;  jmp  short @@exit            ; debug
@@ProgramTimers:
  ; first timer 2
  mov al, 10010000B            ; 090h == Timer 2, 1 byte values, generate 1 pulse
  out 043h, al
  in  al, 061h                 ; 061h ==speakerport,
  or  al, 003h                 ; connect to speaker
  out 061h, al

  ; then timer 0
  mov al, 00110110B            ; == 036h
  out 043h, al
  ; 1193180/X=hz, hz=16000 => X=74.57 ~= 74 (slightly too slow)
  mov al, 60                   ; should be 59.659=20000hz. 60=19886.333333333hz
  out 040h,al
  mov al, 0
  out 040h, al                 ;need a high byte as well

  lea  eax, [strIntset]
  push eax
  call _printf
  add  esp, 4
  jmp  short @@exit
@@pError:
  lea  eax, [strProtmodeInstallError]
  push eax
  call _printf
  add  esp, 4
@@exit:
  ret
endp

proc audio_int_protmode_simpletest nolanguage ; this is an interrupt, let's do our own push-ups!
  cli                          ; disable any other interrupts
  push  eax                    ; save registers we use
  ; do stuff
  ; ...
  ; Should set DS to prevent writing to random memory
  push ds
  mov  ds, [word ptr cs:protmode_ds]
  inc  [dword ptr NrServicedInts]
;  jmp  short @@exit            ; debug
  inc  [dword ptr ChainTimer]
  cmp  [dword ptr ChainTimer], 1093 ; almost correct for 19886.333333333hz, should be 1092.6556776561092.655677656
  jne  @@exit
  mov  [dword ptr ChainTimer], 0
  ;jmp:
  pop  ds
  pop  eax
  db 0eah                      ; else we chain into original handler
  ;call:
  ;db 0ffh                     ; choose either one
  ;db 0e9h                     ; but not both
  old_protmode_int08 db 6 dup (0)
  ; profit?
@@exit:
  pop  ds
  mov  al, 020h
  out  020h, al
  pop  eax
  sti
  iret
  protmode_ds dw 0
endp

endp
dataseg
strIntset db 'interrupt installed!',newline,36,0
strNrServicedInts db 'Nr of interrupts serviced: %i',newline,36,0
strProtmodeInstallError db 'Error while installing Protected mode interrupt',newline,36,0
strProtmodeUninstallError db 'Error while uninstalling Protected mode interrupt',newline,36,0
strNoIntUninstallError db 'uninstall_audio_interrupt(): Errr... no interrupt got installed?',newline,36,0
udataseg
NrServicedInts dd ?
ChainTimer dd ?
end
