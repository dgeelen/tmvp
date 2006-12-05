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

INPBUF_SIZE EQU 64*1024

codeseg
proc decoder
  ; Pre:  global Filehandle:dword == File opened for reading
  ; Post: File has been decoded/played
  uses eax,ebx,ecx,edx,esi,edi,es

  ; First some generic initialisation shared by all decoders (2 atm)
  mov  ebx, [dword ptr Filehandle] ; Set filehandle
  ;lea  edx, [DecodeBuffer]
  mov  edx, [dword ptr DecodeBufHead]
  add  edx, AUDSAMP_SIZE
  cli                          ; Prevent FLIP from occurring to early
  mov  [dword ptr L7SDecodeBufHead], edx
  mov  [dword ptr L7SDecodeBufTail], edx
  sti

  ; Set up LZ77s decompressor
  mov  ax, ds
  mov  es, ax
  mov  [dword ptr InputBufHead], offset InputBuffer
  mov  [dword ptr InputBufTail], offset InputBuffer
  mov  esi, eax
  mov  edi, eax
  cld

  ; then we decide which decoder to use
  mov  eax, 1
  test [dword ptr DecoderMode], eax ; is it RAW or not?
  jz   @@RawDecoder                 ; it's a raw
  shl  eax, 1
  test [dword ptr DecoderMode], eax ; test what kind of compression (LZ77s or ...??)
  jz   @@lz77sDecoder
  lea  eax, [strDecoderNotFound]
  push eax
  call _printf
  add  esp, 4
  jmp  @@exit

;******************************************************************************************************************************

@@RawDecoder:                  ; ----------------------------------------------------------------------------------------------RAW---
  IF PRIVMODE eq True
    hlt
  ENDIF  ; disabled as it is a privileged instruction in pm, and gives us a #GP(0)
  call keypressed              ; TODO: LESS CHECKING ?
  jnz  @@TestRawBufferEmpty
  call near @@ProcessKeypress
@@TestRawBufferEmpty:
  BufferSpaceLeft L7SDecodeBufHead, L7SDecodeBufTail, DECBUF_SIZE ; eax == empty bytes left in buffer
  ; dont refill if the buffer is still >50% full
  cmp  eax, DECBUF_SIZE shr 1
  jbe  @@RawDecoder

  ; dont refill if there is less than DECWIN_SIZE free space
  ; ( and cap the amount of bytes to refill at freespace-DECWIN_SIZE )
  sub  eax, DECWIN_SIZE
  js   @@RawDecoder

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
  mov  [dword ptr L7SDecodeBufTail], edx
  test eax, eax
  jz  @@Done
  jmp  @@RawDecoder          ; ---------------------------------------------------------------------------------------------ENDRAW---

;******************************************************************************************************************************

@@lz77sDecoder:              ; ---------------------------------------------------------------------------------------------LZ77s---
  IF PRIVMODE eq True
    hlt
  ENDIF  ; disabled as it is a privileged instruction in pm, and gives us a #GP(0)
  call keypressed              ; TODO: LESS CHECKING ?
  jnz  @@TestLZ77sBufferEmpty
  call near @@ProcessKeypress

@@TestLZ77sBufferEmpty:


;>--------- seek fixing-----------------
  mov eax, [dword ptr DecodeBufTail]			; compare out tail and the interrupts tail
  add eax, FRAME_SIZE + AUDSAMP_SIZE + PALETTE_SIZE
  cmp eax, offset DecodeBuffer + DECBUF_SIZE		; add 8448 to int tail to get next frame start (current frame end)
  jb @@nowrapseeker					; do wrapping
  mov eax, offset DecodeBuffer
@@nowrapseeker:
  mov ebx, eax						; save next frame start value in ebx
  sub eax, [dword ptr L7SDecodeBufTail]			; compare it with out decoding position (tail)
  neg eax
  js @@noseekerframeinc
  cmp eax, FRAME_SIZE + AUDSAMP_SIZE + PALETTE_SIZE
  jnb @@noseekerframeinc
  mov [dword ptr DecodeBufTail], ebx
@@noseekerframeinc:

  mov eax, [dword ptr DecodeBufHead]
  mov [dword ptr L7SDecodeBufHead], eax

;<--------- seek fixing-----------------

  BufferSpaceLeft L7SDecodeBufHead, L7SDecodeBufTail, DECBUF_SIZE ; eax == empty bytes left in buffer
  ; do crappy refill for now....
  cmp eax, 0FFh ;//decwinsize????
  jbe @@lz77sDecoder
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

; ecx= nr of bytes to read in
  mov  ebx, [dword ptr DecodeableBytes]
  sub  ebx, [dword ptr DecodedBytes]   ; ebx == Max nr of bytes left to read in
  cmp  ecx, ebx
  jbe  @@CanRead                       ; try not to decode more bytes then are actually compressed
  mov  ecx, ebx                        ; else ecx=max nr bytes left
@@CanRead:
  lea edx, [offset InputBuffer + eax]
  push eax
  mov  ebx, [dword ptr Filehandle]

  _fread
  pop ecx
  add  [dword ptr DecodedBytes], eax ; update nr of bytes we have read in

  add eax, ecx

  test eax, eax               ; some comments on what exactly we were testing eax for would have been handy Right About Now
  jz   @@done                 ;  Assume: eax == number of bytes in buffer?

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

  mov edi, [dword ptr L7SDecodeBufTail]

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
        ;jmp @@nosign ;TODO: figure out when this happens and what it does
       @@noedi:
        jmp @@nosign
       @@golz77copy:
        rep movsb
  pop ebx
  mov [dword ptr L7SDecodeBufTail], edi
  jmp @@TestLZ77sBufferEmpty

@@literalcopy:
  mov cl, al
  and ecx, 07Fh
  mov edi, [dword ptr L7SDecodeBufTail]

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

  mov [dword ptr L7SDecodeBufTail], edi
  mov [dword ptr InputBufHead], esi

  jmp @@TestLZ77sBufferEmpty           ; ----------------------------------------------------------------------------------------------END LZ77s---

;******************************************************************************************************************************

                               ;-----------------------------------------------------------------------------------------------KEY HANDLER---
@@ProcessKeypress:             ; *NOTICE* THIS HANDLES KEYPRESSES FOR *ALL* DECODERS
                               ; HENCE THEY _MUST_ _ALL_ OBEY THE INVARIANTS ASSUMED HERE!!!
  call getkey                  ; there was a key press let's find out which key it was (But we don't tell you what those are ;)
  cmp  al, 27                  ; ESC
  jne  @@dontexit              ; exit immidiately, without showing frames remaining in buffer
@@KeyhandlerExit:              ; THIS DOES NOT EXIT THE KEY HANDLER
  pop  eax                     ; discard return address
  jmp  @@exit
@@dontexit:
  cmp  al, 32                  ; Space
  je   @@pauseplayer
  cmp  al, 'r'                 ; RestartMovie
  je   @@RestartMovie
  cmp  al, 0                   ; Extended key?
  jne   @@ExitKeyhandler
  call getkey                  ; get second byte
  xor  bl, bl                  ; @seek, bl==0 => FWD
  cmp  al, 'M'                 ; Arrow Right
  je   @@Seek
  inc  bl                      ; @seek, bl==1 => BWD
  cmp  al, 'K'                 ; Arrow Left
  je   @@Seek
  inc  bl                      ; @seek, bl==2 => FFWD
  cmp  al, 'H'                 ; Arrow up
  je   @@Seek
  inc  bl                      ; @seek, bl==3 => FBWD
  cmp  al, 'P'                 ; Arrow down
  je   @@Seek
@@ExitKeyhandler:
  retn                         ; resume play
@@pauseplayer:
  cli
  mov  eax, [dword ptr DecodeBufHead] ; save head
  mov  edx, [dword ptr DecodeBufTail] ; and get tail
  mov [dword ptr DecodeBufHead], edx  ; 'empty' buffer
  sti
  push eax                            ; eax used in following
@@paused:
  IF PRIVMODE eq True
    hlt
  ENDIF  ; disabled as it is a privileged instruction in pm, and gives us a #GP(0)
  call keypressed
  jnz  @@paused
  call getkey                  ; read and discard key
  pop  eax
  mov  [dword ptr DecodeBufHead], eax
  retn
@@Seek:                        ; Seek to next/previous 'keyframe'
  mov  eax, [dword ptr FrameCounter]
  mov  ecx, SEEKINTERVAL       ; 200 frames
  xor  edx, edx                ; used in div
  div  ecx                     ; eax is index ptr div 5
  ;Decide where we are seeking to (bl indicates fwd, bwd, ffwd, fbwd)
  inc  bl                      ; bl == 1 .. 4
  dec  bl
  jz   @@fwd
  dec  bl
  jz   @@bwd
  dec  bl
  jz   @@ffwd
  ;fbwd
  sub  eax, 5                  ; skip -6 * 10 sec
  jmp  short @@bwd
@@ffwd:
  add  eax, 5                  ; skip 6 * 10 sec
@@fwd:
  inc  eax
  cmp  eax, [SeekOffsetCount]  ; number of seekable keyframes
  jb   @@doskip
  retn
@@bwd:
  dec  eax
  cmp  eax, -1
  jg   short @@doskip
  jmp  @@RestartMovie

@@doskip:
  mov  edx, eax
  push eax                     ; later we use this to set the #decframes
  shl  eax, 2                  ;  eax*4
  add  eax, edx                ; (eax*4)+eax = (eax*5)
  add  eax, 4+5                ; back away from the end of file
  neg  eax                     ; eax = - offset
  add  eax,[dword ptr FileSize]; eax = seek to index
  mov  ecx, eax                ; fseek(): ecx == offset
  mov  ebx, [dword ptr Filehandle]; ebx == filehandle, cx:dx = new file position, eax(al)=origin of move
  mov  al, SEEK_SET            ; seek from start
  _fseek                       ; returns dx:ax = new file position from start of file
  mov  ecx, 5                  ; each keyframe is 5 bytes
  lea  edx, [SeekPtr]          ;
  _fread                       ; ebx == filehandle, ecx == nr bytes to read, ds:edx == ptr to buffer
  mov  ecx, [dword ptr SeekPtr]
  add  ecx, FONT_SIZE + FILEHEADER_SIZE
  mov  al, SEEK_SET            ; move from start of file
  _fseek
  cli                          ; disable interrupts to prevent interference from audio interrupts
  mov  eax, [dword ptr SeekPtr];
  mov  [dword ptr DecodedBytes], eax
  mov  [dword ptr AudioDataPtr], offset DecodeBuffer + DECBUF_SIZE - AUDSAMP_SIZE - FRAME_SIZE - PALETTE_SIZE
  mov  [dword ptr SampleTimer]  , 1 ; reset currently playing audio block
  mov  eax, offset InputBuffer
  mov  [dword ptr InputBufHead ], eax
  mov  [dword ptr InputBufTail ], eax
  movzx edx, [byte ptr SeekFixup]
  mov  eax, (offset DecodeBuffer) - AUDSAMP_SIZE - AUDSAMP_SIZE - FRAME_SIZE - PALETTE_SIZE + DECBUF_SIZE
  mov  [dword ptr DecodeBufTail], eax
  mov  [dword ptr DecodeBufHead], eax
  add  eax, edx
  add  eax, AUDSAMP_SIZE
  mov  [dword ptr L7SDecodeBufTail], eax
  mov  [dword ptr L7SDecodeBufHead], eax
  pop  edx                     ; index we saved earlier
  FASTIMUL eax, edx, 200       ; dest, src, mulconst : make TASM do shifts/mul
  mov  [dword ptr framecounter], eax
  cld
  sti
  retn
@@RestartMovie:
  mov  ax, 04200h             ; lseek() from start
  mov  ebx, [dword ptr Filehandle]
  xor  cx, cx                 ; CX:DX == offset
  mov  dx, 16+2048            ; CX:DX == offset
  int  021h
  mov  [dword ptr DecodedBytes], 0   ; ebx == Reset number of bytes read
  cli
  mov  eax, (offset DecodeBuffer) - AUDSAMP_SIZE - AUDSAMP_SIZE - FRAME_SIZE - PALETTE_SIZE + DECBUF_SIZE
  mov  [dword ptr DecodeBufHead], eax
  mov  [dword ptr DecodeBufTail], eax
  add  eax, AUDSAMP_SIZE
  mov  [dword ptr L7SDecodeBufHead], eax
  mov  [dword ptr L7SDecodeBufTail], eax
  mov  [dword ptr AudioDataPtr], offset DecodeBuffer + DECBUF_SIZE - AUDSAMP_SIZE - FRAME_SIZE - PALETTE_SIZE
  mov  [dword ptr SampleTimer]  , 1
  mov  eax, offset InputBuffer
  mov  [dword ptr InputBufHead ], eax
  mov  [dword ptr InputBufTail ], eax
  xor  eax, eax
  mov  [dword ptr framecounter], eax
  mov  [dword ptr decodedbytes], eax
  cld
  sti
  retn
  ; ----------------------------------------------------------------------------------------------END KEY HANDLER---

;******************************************************************************************************************************

@@done:
  IF PRIVMODE eq True
    hlt
  ENDIF  ; disabled as it is a privileged instruction in pm, and gives us a #GP(0)
  call keypressed              ; Check for keypress incase file is b0rken
  jnz  @@doneNotKeyPress
  call near @@ProcessKeypress
@@doneNotKeyPress:
  mov  eax, [dword ptr DecodeBufHead]
  cmp  eax, [dword ptr DecodeBufTail]
  jne   @@done                 ; wait for interrupt to play remaining frames (empty the buffer)
@@exit:                        ; Then exit
  ret
endp

dataseg
strReadMsg db 'postread, eax=%i',newline,'$',0
strWriteMsg db 'postwrite, eax=%i',newline,'$',0
strOpenMsg db 'postopen, eax=%i',newline,'$',0
strDecoderNotFound db 'WTF?! Can''t find appropriate decoder, something is wrong here!', newline, 36,0
modechar_WO db ' w',0

udataseg
DecodeBuffer db DECBUF_SIZE dup (?) ; (1 frame + 1/20s audio) * 32 == 268800bytes
DecodeBufHead dd ?             ; HEAD points to the first unread element in the buffer
DecodeBufTail dd ?             ; TAIL points one past the element last added to the buffer
L7SDecodeBufHead dd ?          ; HEAD points to the first unread element in the buffer
L7SDecodeBufTail dd ?          ; TAIL points one past the element last added to the buffer
InputBuffer db INPBUF_SIZE dup (?); data is read into this buffer from input file
InputBufHead dd ?              ; HEAD points to the first unread element in the buffer
InputBufTail dd ?              ; TAIL points one past the element last added to the buffer
AudioDataPtr dd ?              ; AudioDataPtr points to the next-to-be-played sample of audio data
DecoderMode dd ?               ; This is set by the filemagic routine and determines the decoder needed to decode this file
DecodeableBytes dd ?           ; number of bytes we can safely decode
DecodedBytes dd ?              ; number of bytes we have decoded so far. DecodeableBytes==DecodedBytes => EOF=True
SeekOffsetCount dd ?           ; Number of seekable frames
SeekPtr dd ?                   ; Where to seek to  *ONE 'VARIABLE'!*
SeekFixup db ?                 ; fixup offset      ^^^^^^^^^^^^^^^^^
end
