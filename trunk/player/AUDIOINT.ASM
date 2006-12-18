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

if LPT_DAC eq True
  if DISNEY_SOUND_SOURCE eq True
    mov  dx, LPT_BASE_PORT + 2
    mov  al, 04h                 ; turn it off
    out  dx, al
  endif
endif

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
  ;      A. I'm guessing that it's okay not to cleanup, but let's do it anyway.
  ;
  uses eax, ebx, ecx, edx, edi, es

  ; load es with alias to cs, so we can write into our code
  ; (this is not possible using cs directly in protected mode)
  mov ax, 000Ah			; 000Ah == Create Alias Descriptor
  mov bx, cs			; in:  bx = original selector
  int 31h			; out: ax = new selector, CF = error
  jc  @@pError
  mov es, ax
  mov  [dword ptr ChainTimer], (119318000/((((QUARTZ_TICKS*10)/AUDIO_INT_FREQ+5)/10) * 182) + 5)/10
  mov  [dword ptr DecodeBufHead], offset DecodeBuffer
  mov  [dword ptr DecodeBufTail], offset DecodeBuffer
  mov  [dword ptr AudioDataPtr], offset DecodeBuffer + DECBUF_SIZE
  xor  eax, eax
  mov  [dword ptr FrameCounter], eax
  inc  eax
if AUDDBL_SAMPLE eq True
  mov  [dword ptr DoubleSample], eax
endif
  mov  [dword ptr SampleTimer], eax
  dec  eax
  mov  ebx, large 0b800h       ; SEG(video_ram)
  add  al,2                    ; mov ax, 2
  int  031h                    ; Segment to selector, ax = selector
  jnc  @@GotVidSegSel
  lea  eax, [strVidSegSelError]
  push eax
  call _printf
  add  esp, 4
  jmp  short @@pError
@@GotVidSegSel:
  mov  [word ptr ds:vidmem_selector], ax
  ; SET PROTECTED MODE INTERRUPT
  mov   ax, 0204h              ; get protected mode interrupt vector
  mov   bl, 08h                ; of int08
  int   031h                   ; get it
  jc    @@pError
  ; CX:EDX == selector:offset
  mov  [dword ptr es:old_protmode_int08    ], edx
  mov  [ word ptr es:old_protmode_int08 + 4], cx
  ; store DS for use in interrupt
  mov  [word ptr es:protmode_ds], ds
  ; set new interrupt vector
  mov   ax, 0205h
  mov   bl, 08h
  mov   cx, cs
  mov  edx, large offset audio_int
  int  031h
  jc   @@pError
;  jmp  short @@exit            ; debug
@@ProgramTimers:
  if PC_SPEAKER eq True
  ; first timer 2
  mov al, 10010000B            ; 090h == Timer 2, 1 byte values, generate 1 pulse
  out 043h, al
  in  al, 061h                 ; 061h ==speakerport,
  or  al, 003h                 ; connect to speaker
  out 061h, al
  endif
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

if LPT_DAC eq True
  if DISNEY_SOUND_SOURCE eq True
    mov  dx, LPT_BASE_PORT + 2
    mov  al, 04h                 ; turn it on
    out  dx, al
  endif
endif

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

proc audio_int nolanguage ; this is an interrupt, let's do our own push-ups!
  cli                          ; disable any other interrupts
  push ds                      ; save ds
  push eax                     ; and eax
  push esi                     ; and esi

  ; set ds to our data segment selector
  db 66h, 0B8h		; mov ax, imm16
  protmode_ds dw 0	; this is set by install_audio_interrupt
  mov  ds, ax

  ; first check that the buffer is non-empty
  mov  esi, [dword ptr DecodeBufHead]
  cmp  [dword ptr DecodeBufTail], esi ; CMP MEM, REG is 1 clock faster than CMP REG,MEM?!
  je   short @@NoFlip          ; if empty do nothing
  ; Play a sample of audio     ; note: A/V Sync is maintained because every frame has 1/20s of audio, which is played completely before proceding with display ing the next frame.
  mov  esi, [dword ptr AudioDataPtr] ; load pointer
  lodsb                        ; get data byte
if BEEP_ON_FREAD eq True
  ; TODO: When running this on _ACTUAL HARDWARE_ test if it is really usefull to use the actual value
  ;       or if just suspending the speaker (mov  al, 0.5*MAX_AMPLITUDE) is enough. In the latter case
  ;       this BEEP_ON_FREAD section may be removed.
  mov  [dword ptr WaveSample], eax ; ONLY AL IS DEFINED, But for alignment reason we still read/write dwords
endif
if PC_SPEAKER eq True
  out  042h, al                ; 042h = timer 2 counter
endif
if LPT_DAC eq True
  push dx
  mov  dx, [word ptr LPTPortAddress] ;LPT_BASE_PORT
  out  dx, al
  if DISNEY_SOUND_SOURCE eq True ; This is Disney Sound Source Voodoo (required in DOSBox)
    add  dx, 2                 ; select control regists
    mov  al, 00ch              ; device off
    out  dx, al
    mov  al, 004h              ; device on
    out  dx, al
  endif
  pop  dx
endif
if AUDDBL_SAMPLE eq True
  ; TODO: This sounds like it might need some work in DOSBox, but it needs to be tested on
  ;       actual hardware to be sure. (Conclusion based on 'It Verks for PC_SPEAKER')
  xor  [dword ptr DoubleSample], large 00000001h
  jnz  @@RepeatSample
  mov  [dword ptr AudioDataPtr], esi
  dec  [dword ptr SampleTimer]
@@RepeatSample:
endif
if AUDDBL_SAMPLE eq False
  mov  [dword ptr AudioDataPtr], esi
  dec  [dword ptr SampleTimer]
endif
  jnz  short @@NoFlip          ; We're still in the current datablock, thus certainly in the buffer
  add  esi, PALETTE_SIZE + FRAME_SIZE ;Select next audiodata block
  cmp  esi, (offset DecodeBuffer) + DECBUF_SIZE
  jb   short @@NoWrap          ; if we've selected a datablock outside the buffer we must wrap
  mov  esi, (offset DecodeBuffer) + PALETTE_SIZE + FRAME_SIZE
@@NoWrap:
  mov  [dword ptr AudioDataPtr], esi
  mov  [dword ptr SampleTimer], AUDSAMP_SIZE ; reset timer
  jmp  short @@Flipper

@@NoFlip:
  dec  [dword ptr ChainTimer]  ;
  jnz  short @@exit            ; time to call the original handler (update system clock etc)
  mov  [dword ptr ChainTimer], ((QUARTZ_TICKS*100)/((((QUARTZ_TICKS*10)/AUDIO_INT_FREQ+5)/10) * 182) + 5)/10; reset ChainTimer
  pop  esi                     ; clear the stack
  pop  eax                     ; of the registers
  pop  ds                      ; that we used
  db  0eah                     ; and we chain into original handler with a far jmp
  old_protmode_int08 db 6 dup (0)
@@exit:
  mov  al, 020h
  out  020h, al
  pop  esi
  pop  eax
  pop  ds
  sti
  iret
@@Flipper:                     ; we put flip at the _very end_ of the interrupt, because setpalette is rather large
  mov  al, 020h                ;
  out  020h, al                ; signal End of Interrupt
  sti                          ; renable interrupts   DANGEROUS: RENTRY?
  inc  [dword ptr FrameCounter]
  mov  esi, [dword ptr DecodeBufHead]


;;;;;;;;;;; ; fake flip
;  add  esi, FRAME_SIZE + AUDSAMP_SIZE
;  cmp  esi, (offset DecodeBuffer) + DECBUF_SIZE
;  jb   short @@NoRe32324324 ; If we've exceeded the buffer size, reset
;  mov  esi, offset DecodeBuffer
;@@NoRe32324324:
;  mov  [dword ptr DecodeBufHead], esi; update head pointer
;  pop  esi
;  pop  eax
;  pop  ds
;  iret
;;;;;;;;;;;


  push es edi ecx edx          ; save registers needed to flip
  mov  es, [word ptr vidmem_selector]; selector for the video_ram
  xor  edi, edi                ; start filling ram from address 0 (b800h:0000h)
  mov  ecx, large FRAME_SIZE shr 2 ; Bytes to dword
  vsync                        ; modifies dx, al
;  cli
  rep  movsd                   ; Flip
;  push esi
  setpalette                   ; ds:esi -> Palette data, uses al, cl, dx
;  pop  esi
;  sti
  add  esi, AUDSAMP_SIZE       ; We don't want to display audiodata
  cmp  esi, (offset DecodeBuffer) + DECBUF_SIZE
  jb   short @@NoResetFramePtr ; If we've exceeded the buffer size, reset
  mov  esi, offset DecodeBuffer
@@NoResetFramePtr:
  mov  [dword ptr DecodeBufHead], esi; update head pointer
  pop  edx ecx edi es
@@ExitFlip:
  pop  esi
  pop  eax
  pop  ds
  iret
endp

endp
dataseg
strIntset db 'interrupt installed!',newline,36,0
strProtmodeInstallError db 'Error while installing Protected mode interrupt',newline,36,0
strProtmodeUninstallError db 'Error while uninstalling Protected mode interrupt',newline,36,0
strNoIntUninstallError db 'uninstall_audio_interrupt(): Errr... no interrupt got installed?',newline,36,0
strVidSegSelError db 'Error: could not get selector for video segment',newline, 36, 0

udataseg
FrameCounter dd ?
vidmem_selector dw ?
ChainTimer dd ?
SampleTimer dd ?
if BEEP_ON_FREAD eq True
  WaveSample dd ?
endif
if AUDDBL_SAMPLE eq True
  DoubleSample dd ?
endif
if LPT_DAC eq True
  LPTPortAddress dd ?
endif
end

