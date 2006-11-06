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

DM_RAW      EQU 1	; implemented and working
DM_LZ77S    EQU 2
DM_DEFLATE  EQU 3

DECODE_MODE EQU DM_RAW
;DECODE_MODE EQU DM_LZ77S

INPBUF_SIZE EQU 64*1024

codeseg
proc decoder
  ; Pre:  global Filehandle:dword == File opened for reading
  ; Post: File has been decoded/played
  uses eax,ebx,ecx,edx,esi,edi,es



;--- LOAD AND 'PLAY' A _RAW_ FILE ---
  ; Simon sais:
  ; blah
  mov ax, ds
  mov es, ax
  cld
  
  mov  ebx, [dword ptr Filehandle]
  lea  edx, [DecodeBuffer]
  lea  eax, [edx + FRAME_SIZE + PALETTE_SIZE]
  cli                          ; Prevent FLIP from occurring to early
  mov  [dword ptr DecodeBufHead], edx
  mov  [dword ptr DecodeBufTail], edx ; head == tail ==> Buffer Empty, head == tail - 1 ==> Buffer Full
  mov  [dword ptr AudioDataPtr] , eax
  sti
  
if DECODE_MODE eq DM_LZ77S  
  mov  [dword ptr InputBufHead], offset InputBuffer	
  mov  [dword ptr InputBufTail], offset InputBuffer	
endif
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
  BufferSpaceLeft DecodeBufHead, DecodeBufTail, DECBUF_SIZE ; eax == empty bytes left in buffer
  ; do crappy refill for now....
  cmp eax, 0FFh
  jbe @@waitnextiter
  BufferSpaceLeft InputBufHead, InputBufTail, INPBUF_SIZE;
  sub eax, INPBUF_SIZE
  neg eax
  cmp eax, 0FFh + 4
  ja @@nofread
  ; yeah... crappy here
  mov ecx, eax
  mov esi, [dword ptr InputBufHead]
  mov edi, offset InputBuffer
  rep movsb
  
  lea ecx, [INPBUF_SIZE]
  sub ecx, eax
  
  lea edx, [offset InputBuffer + eax]
  
  push eax
  mov  ebx, [dword ptr Filehandle]
  _fread
  pop ecx
  
  add eax, ecx
  add eax, offset InputBuffer
  mov [dword ptr InputBufTail], eax
  mov [dword ptr InputBufHead], offset InputBuffer
  
@@nofread:

  mov esi, [dword ptr InputBufHead]
  
  lodsb
  
  cmp al, 080h
  jae @@literalcopy
  
  ; LZ77 Copy
  
  push ebx

  mov edi, [dword ptr DecodeBufTail]
    
  shl ax, 8
  lodsb
  movzx edx, ax
  lodsb
  
  mov [dword ptr InputBufHead], esi
  mov esi, edi
  sub esi, edx
  movzx ecx, al
  
  cmp esi, offset DecodeBuffer
  jae @@nosign
  add esi, DECBUF_SIZE
  @@nosign:  
  
        mov ebx,esi
        cmp ebx,edi
        jae @@higher
        mov ebx,edi
       @@higher:

        ;{ edx := #words left in buffer }
        mov edx, offset DecodeBuffer + DECBUF_SIZE
        sub edx, ebx

        ;{ if ecx < edx then goto @go }
        cmp ecx, edx
        jb @@golz77copy

        ;{ ecx, edx := edx, ecx-edx }        // !whoa, multiple assignment!!
        sub edx, ecx
        add ecx, edx
        neg edx

        rep movsb

        mov ecx, edx
        cmp esi, offset DecodeBuffer + DECBUF_SIZE
        jne @@noesi
        mov esi, offset DecodeBuffer
       @@noesi:
        cmp edi, offset DecodeBuffer + DECBUF_SIZE
        jne @@noedi
        mov edi, offset DecodeBuffer
        jmp @@nosign ;TODO: figure out when this happens and what it does
       @@noedi:
        jmp @@nosign
       @@golz77copy:

        rep movsb

  pop ebx
  
  mov [dword ptr DecodeBufTail], edi

  jmp @@NextFrame  
  
@@literalcopy:
  mov cl, al
  and ecx, 07Fh
  mov edi, [dword ptr DecodeBufTail]
  
  ;{ edx := #bytes left till buffer end }
  mov edx, offset DecodeBuffer + DECBUF_SIZE
  sub edx, edi

  cmp ecx, edx
  jb @@golitcopy
  
  ;{ ecx, edx := edx, ecx-edx }        // !whoa, multiple assignment!!
  sub edx, ecx
  add ecx, edx
  neg edx
  
  rep movsb
  
  mov ecx, edx
  mov edi, offset DecodeBuffer
  
@@golitcopy:
  rep movsb
  
  mov [dword ptr DecodeBufTail], edi
  mov [dword ptr InputBufHead], esi

  jmp @@NextFrame
  
  
  
endif                           ;******* End LZ77S Player *******
;----------------------------------------------------------------
@@waitnextiter:
  call keypressed              ; TODO: LESS CHECKING ?
  jz   @@ProcessKeypress
  hlt
  jmp @@NextFrame
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
InputBuffer db INPBUF_SIZE dup (?); data is read into this buffer from input file
InputBufHead dd ?             ; HEAD points to the first unread element in the buffer
InputBufTail dd ?             ; TAIL points one past the element last added to the buffer
AudioDataPtr dd ?              ; AudioDataPtr points to the next-to-be-played sample of audio data
DebugFilehandle  dd ?
end
