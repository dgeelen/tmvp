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

  ;debug
  ;mov  al, 75                  ; 15909.066666667 hz
  ;out 040h,al
  ;mov  al, 0
  ;out 040h,al

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
  mov  [dword ptr ChainTimer], (119318000/((((QUARTZ_TICKS*10)/AUDIO_INT_FREQ+5)/10) * 182) + 5)/10
  mov  [dword ptr FlipTimer], AUDIO_INT_FREQ/FPS
  mov  [dword ptr DecodeBufHead], eax
  mov  [dword ptr DecodeBufTail], eax
  mov  [dword ptr AudioDataPtr], eax
  mov  [dword ptr SampleTimer], AUDSAMP_SIZE
  mov  ebx, large 0b800h       ; SEG(video_ram)
  inc  al                      ; al = 1
  inc  al                      ; mov ax, 2
  int  031h                    ; Segment to selector, ax = selector
  jnc  @@GotVidSegSel
  lea  eax, [strVidSegSelError]
  push eax
  call _printf
  add  esp, 4
  jmp  short @@pError
@@GotVidSegSel:
  mov  [word ptr cs:vidmem_selector], ax
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
;  mov al, 60                   ; should be 59.659=20000hz. 60=19886.333333333hz
;  out 040h,al                  ; this is a 0.005683333% difference
;  mov al, 0
;  out 040h, al                 ;need a high byte as well

  mov  al, low(((QUARTZ_TICKS*10)/AUDIO_INT_FREQ+5)/10) ; round to the nearest int
  out  040h,al
  mov  al, high(((QUARTZ_TICKS*10)/AUDIO_INT_FREQ+5)/10)
  out  040h, al                 ;need a high byte as well

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
  push ds                      ; save ds
  push eax                     ; and eax
  push esi                     ; and esi
  mov  ds, [word ptr cs:protmode_ds]
  inc  [dword ptr NrServicedInts]

  ; first check that the buffer is non-empty
  mov  esi, [dword ptr DecodeBufHead]
  cmp  esi, [dword ptr DecodeBufTail]
  je   @@NoFlip               ; if empty do nothing
  ; Play a sample of audio    ; ** MAYOR TODO: Figure out if A/V Sync is neccesary and how to implement it **
  mov  esi, [dword ptr AudioDataPtr] ; load pointer
  lodsb                        ; get data byte
  out  042h, al                ; 042h = timer 2 counter
  dec  [dword ptr SampleTimer]
  jnz  @@StillInAudioDataBlock
  add  esi, PALETTE_SIZE + FRAME_SIZE ;Select next audiodata block
  mov  [dword ptr SampleTimer], AUDSAMP_SIZE ; reset timer
@@StillInAudioDataBlock:
  cmp  esi, (offset DecodeBuffer) + DECBUF_SIZE
  jb   @@StillInBuffer
  mov  esi, (offset DecodeBuffer) + PALETTE_SIZE + FRAME_SIZE
@@StillInBuffer:
  mov  [dword ptr AudioDataPtr], esi

  dec  [dword ptr FlipTimer]   ; *FIXME* : Inc pre or post check?
  jnz  @@NoFlip
  ;Flip, DANGEROUS: RENTRY?
  mov  al, 020h                ;
  out  020h, al                ; signal End of Interrupt
  sti                          ; renable interrupts
  mov  [dword ptr FlipTimer], AUDIO_INT_FREQ/FPS; reset FlipTimer
  ; Flip videobuffer to the screen
  mov  esi, [dword ptr DecodeBufHead]
  push es edi ecx edx          ; save registers needed to flip
  mov  es, [word ptr vidmem_selector]; selector for the video_ram
  xor  edi, edi                ; start filling ram from address 0 (b800h:0000h)
  mov  ecx, large FRAME_SIZE shr 2 ; Bytes to dword
  vsync                        ; modifies dx, al
  rep  movsd                   ; Flip
  setpalette                   ; ds:esi -> Palette data, uses al, cl, dx
  add  esi, AUDSAMP_SIZE       ; We don't want to display audiodata
  cmp  esi, (offset DecodeBuffer) + DECBUF_SIZE
  jb   @@NoResetFramePtr       ; If we've exceeded the buffer size, reset
  mov  esi, offset DecodeBuffer
@@NoResetFramePtr:
  mov  [dword ptr DecodeBufHead], esi; update head pointer
  pop  edx ecx edi es
@@ExitFlip:
  pop  esi
  pop  eax
  pop  ds
  iret

@@NoFlip:
  dec  [dword ptr ChainTimer]  ; *FIXME* : Inc pre or post check?
  jnz  @@exit
  mov  [dword ptr ChainTimer], (119318000/((((QUARTZ_TICKS*10)/AUDIO_INT_FREQ+5)/10) * 182) + 5)/10; reset ChainTimer
  pop  esi
  pop  eax
  pop  ds                      ; restore ds and eax
  db  0eah                     ; else we chain into original handler with a far jmp
  old_protmode_int08 db 6 dup (0)
@@exit:
  mov  al, 020h
  out  020h, al
  pop  esi
  pop  eax
  pop  ds
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
strVidSegSelError db 'Error: could not get selector for video segment',newline, 36, 0

esi_msg db 'ESI="%x"="%i"',newline, 36, 0

udataseg
AudioBuffer db AUDSAMP_SIZE * RENDER_AHEAD dup (?); interrupt consumes 1000 bytes / frame (1/20s)
                                                  ; NOTE: If possible, we should read this also directly from [DecodeBuffer]
vidmem_selector dw ?
NrServicedInts dd ?
FlipTimer dd ?
ChainTimer dd ?
SampleTimer dd ?
end
