TITLE Decode
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"

locals ;@@                     ; Use '@@' as local symbol prefix
                               ; '@@' is default
                               ; Must be 2 chars
jumps

bufsize equ 1024*128

DM_RAW      EQU 1
DM_LZ77S    EQU 2
DM_DEFLATE  EQU 3

DECODE_MODE EQU DM_RAW

codeseg
proc decoder
  ; Pre:  global Filehandle:dword == File opened for reading
  ; Post: File has been decoded/played
  uses eax,ebx,ecx,edx



;--- LOAD AND 'PLAY' A _RAW_ FILE ---
  ; Simon sais:
  ; blah
  mov  ebx, [dword ptr Filehandle]
  lea  edx, [DecodeBuffer]
  lea  eax, [edx + FRAME_SIZE + PALETTE_SIZE]
  cli                          ; Prevent FLIP from occurring to early
  mov  [dword ptr DecodeBufHead], edx
  mov  [dword ptr DecodeBufTail], edx ; head == tail ==> Buffer Empty, head == tail - 1 ==> Buffer Full
  mov  [dword ptr AudioDataPtr] , eax
  sti
@@NextFrame:
;----------------------------------------------------------------
if DECODE_MODE eq DM_RAW        ;******* Start Raw Player *******
  BufferSpaceLeft DecodeBufHead, DecodeBufTail, DECBUF_SIZE ; eax == empty bytes left in buffer
  ; dont refill if the buffer is still >50% full
  cmp  eax, DECBUF_SIZE shr 1
  jbe  @@waitnextiter
  
  ; dont refill if there is less than DECWIN_SIZE free space 
  ; ( and cap the amount of bytes to refill at freespace-DECWIN_SIZE )
  sub  eax, DECWIN_SIZE
  js   @@waitnextiter
  
  ; cap amount of bytes to refill to not go past the end of our circular buffer
  mov  ecx, large (offset DecodeBuffer) + DECBUF_SIZE   ; ecx == end of buffer
  sub  ecx, edx                                         ; ecx == amount of bytes from tail to end of buffer
  cmp  eax, ecx
  ja   @@NoMinMax
  mov  ecx, eax
@@NoMinMax:

  ; do the read
  _fread
  ; increase the tail ptr, and wrap it if needed
  add  edx, eax
  cmp  edx, large (offset DecodeBuffer) + DECBUF_SIZE   ; because we cap to not go past the end of the buffer
  jne  @@NoWrap                                         ; we only need to wrap when we are exactly at the end
  mov  edx, large offset DecodeBuffer                   ; and also always wrap to the start of the buffer
@@NoWrap:

  ; move tail ptr back into memory
  mov  [dword ptr DecodeBufTail], edx
  test eax, eax
  jz  @@Done
endif                           ;******* End Raw Player *******
;----------------------------------------------------------------
if DECODE_MODE eq DM_LZ77S      ;******* Start LZ77S Player *******
endif                           ;******* End LZ77S Player *******
;----------------------------------------------------------------
@@waitnextiter:
  call keypressed              ; TODO: LESS CHECKING ?
  jz   @@ProcessKeypress
  hlt
  jmp  short @@NextFrame
@@ProcessKeypress:
  call getkey
  cmp  al, 27                  ; ESC
  je   @@done
  cmp  al, 32                  ; Spatie
  jne  @@NextFrame
  cli
  mov  eax, [dword ptr DecodeBufHead] ; save head
  mov [dword ptr DecodeBufHead], edx  ; 'empty' buffer
  sti
  push eax                            ; eax used in following
@@paused:
  hlt
  call keypressed
  jnz  @@paused
  call getkey                  ; read and discard key
  pop  eax
  mov  [dword ptr DecodeBufHead], eax
  jmp  @@NextFrame
  ret
@@done:
  hlt
  mov  eax, [dword ptr DecodeBufHead]
  test eax, [dword ptr DecodeBufTail]
  je   @@done

  ret
endp

dataseg
strDebugFileName db 'output.tst',0
strReadMsg db 'postread, eax=%i',newline,'$',0
strWriteMsg db 'postwrite, eax=%i',newline,'$',0
strOpenMsg db 'postopen, eax=%i',newline,'$',0
modechar_WO db ' w',0

udataseg
DecodeBuffer db DECBUF_SIZE dup (?) ; (1 frame + 1/20s audio) * 32 == 268800bytes
DecodeBufHead dd ?             ; HEAD points to the first unread element in the buffer
DecodeBufTail dd ?             ; TAIL points one past the element last added to the buffer
AudioDataPtr dd ?              ; AudioDataPtr points to the next-to-be-played sample of audio data
buffer db bufsize dup (?)
DebugFilehandle  dd ?
end
