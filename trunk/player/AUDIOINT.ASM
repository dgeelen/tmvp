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
  lea  eax, [strNrServicedInts]
  push [dword ptr NrServicedInts]
  push eax
  call _printf
  add  esp, 4*2
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
  jnc  @@ProgramTimers
  lea  eax, [strProtmodeUninstallError]
  push eax
  call _printf
  add  esp, 4
  jmp  short @@exit
@@ProgramTimers:

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
  jmp  short @@protmode         ; this seems to Verk (TM)
  ; SET PROTECTED MODE INTERRUPT
  mov   ax, 0204h              ; get protected mode interrupt vector
  mov   bl, 08h                ; of int08
  int   031h                   ; get it
  jc    @@pError
  ; CX:EDX == selector:offset
  mov  [dword ptr cs:old_protmode_int08    ], edx
  mov  [ word ptr cs:old_protmode_int08 + 4], cx
  ; set new interrupt vector
  mov   ax, 0205h
  mov   bl, 08h
  mov   cx, cs
  mov  edx, large offset audio_int_protmode_simpletest
  int  031h
  jc   @@pError
  ; store DS for use in interrupt
  mov  [word ptr cs:protmode_ds], ds
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

proc audio_int_protmode_simpletest
  cli
  ; do stuff
  ; ...
  ; Should set DS to prevent writing to random memory
  push ds
  mov  ds, [word ptr cs:protmode_ds]
  inc  [dword ptr NrServicedInts]
;  jmp  short @@exit
  ;Call old handler
  pop  ds
  ;push [dword ptr cs:old_protmode_int08  ]
  ;push [ word ptr cs:old_protmode_int08+4]
  ;ret                         ; jmp to old handler
  ;call  [cs:old_protmode_int08]          ; is this the correct form of jmp ?
  db 0eah
  old_protmode_int08 db 6 dup (0)
  ;jmp 00:0000
  ; profit?
@@exit:
  pop  ds
  mov  al, 020h
  out  020h, al
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
end
