TITLE smallfunctions
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"
locals
jumps

codeseg

proc InitVidBuf
  ; Initialises the videobuffer with a pattern
  uses  eax, ecx, edi, es
  mov  ax, ds
  mov  es, ax                 ; stosd uses es:edi
  lea  edi, [DecodeBuffer]
  mov  ecx, large DECBUF_SIZE shr 2
  xor  eax, eax
  mov  eax, '1111'
@@l1:
  mov  [dword ptr es:edi], eax
  add  edi, 4
  inc  al
  dec  ecx
  jnz  @@l1
  ;repnz stosd
  ret
endp

proc KeyPressed nolanguage ; Destroys ax, nolanguage == faster
  ; Pre: -
  ; Post: Returns ax = 0 = NZF when no key is present, ax = 1 = ZF otherwise
  mov  ah, 01h
  int  016h                        ; ZF set if no keystroke available
  jz   @false
@true:
  xor  al, al                      ; set ZF
  xchg ah, al                      ; does not affect ZF
  ret
@false:
  xor  ax, ax
  cmp  al, 1
  ret
endp

proc GetKey
  ; Read a keystroke from the keybuffer
  xor  eax, eax
;  mov  ah, 10h
;  int  016h
  ;mov  ah, 06h                 ; Direct Console Input
  ;mov  ah ,07h                 ; direct char input
  mov  ah, 08h                 ; Read key from stdin w/o echo
  int  021h
  and  ax, 00ffh
  ret
endp

proc SetFont
  ; Reads the first 2k of the INPUTFILE and sets the DOS Font with that data
  ; Pre: File is open, readable and Size(FILE) >= 2k
  ; Post: ax == 0 => The DOS font has been set
  ;       else ax == error
  uses ebx, ecx, edx, es, ds
  ; allocate some DOS memory to hold the font data
  mov  ax, 0100h               ; INT 31 P - DPMI 0.9+ - ALLOCATE DOS MEMORY BLOCK
  mov  bx, 2048/16             ; BX = number of paragraphs (16 byte blocks) to allocate
  int  031h                    ; Return: CF clear if successful AX = real mode segment of allocated block,  DX = first selector for allocated block
  jc   @@exit
  mov [ word ptr RMCallStruct + 022h], ax  ; 22h  WORD  ES

  ; read the fontdata into memory
  mov  ebx, [dword ptr FileHandle] ; FileHandle is global
  mov  ecx, 8*256              ; read 256 chars of 8 bytes each
  mov  ax, ds                  ; save ds
  mov  es, ax                  ;
  mov  ds, dx                  ; set ds == selector of DOS memory block
  xor  edx, edx
  _fread                       ; ebx == filehandle, ecx == nr bytes to read, ds:edx == ptr to buffer
  mov  dx, ds                  ; dx == selector of DOS memory block
  mov  ecx, 0ffffffffh
  jc   @@FreeMemory            ; if error
  test eax, eax
  jz   @@FreeMemory            ; or no bytes read (EOF)

  xor eax, eax                 ; or if you just don't like the blocks
  jmp  @@exit


  ; fill out the RealMode Call Struct
  mov  ax, es                  ; ax == dataseg
  mov  ds, ax                  ; reset ds
  mov [dword ptr RMCallStruct + 008h], 00h ; 08h  DWORD EBP
  mov [dword ptr RMCallStruct + 00ch], 00h ; 0Ch  DWORD reserved (00h)
  mov [dword ptr RMCallStruct + 010h], 000000800h ; 10h  DWORD EBX / bytes per char, block 0
  mov [dword ptr RMCallStruct + 014h], 000000000h ; 14h  DWORD EDX / magic ?
  mov [dword ptr RMCallStruct + 018h], 000000100h ; 18h  DWORD ECX / all chars
  mov [dword ptr RMCallStruct + 01ch], 000001100h ; 1Ch  DWORD EAX / load user font

  ; Simulate the RealMode BIOS SetFont interrupt call
  lea  edi, [RMCallStruct]     ; es:edi -> realmode call struct
  mov  ax, 0300h               ; SIMULATE REAL MODE INTERRUPT
  mov  bx, 0010h               ; int 10
  mov  cx, 0                   ; don't need anything on the stack
  int  031h                    ; this actually copies the fontdata into video ram
  mov  ecx, 0ffffffffh
  jc   @@FreeMemory
  inc  ecx                     ; everything is ok
@@FreeMemory:                  ; Clear the DOS memory
  mov  ax, 0101h               ; FREE DOS MEMORY BLOCK
  int  031h
  jc   @@exit
  xor  eax, eax                ; eax == 0 => everything ok
  inc  ecx                     ; why are we freeing memory?
  jnz   @@exit
  dec  eax                     ; set error
@@exit:
  ret
endp

proc Set80x25 ; Destroys none
  ; Sets DOS textmode 80x50
  ; Pre: -
  ; Post: Old video mode has been stored and current mode switched to 80x50x4
  ; Note: I'm kind of hoping int10 only changes ax
  uses eax
  mov  ax, 0083h               ; setting high bit of al prevents CLS
  int  010h
  mov  ax, 01114h              ; load 8x16 font
  xor  bx, bx                  ; block 0
  int  010h
  ret
endp Set80x25

proc RestoreOldVideoMode
  xor  ax, ax
  mov  al, [byte ptr OldVideoMode]
  ;or   al, 080h                ; do not cls
  and  al, 7Fh                 ; DO cls
  int  010h                    ; restore old video mode
;  mov  ax, 01142h              ; Reload 8x16 font
;  xor  bx, bx                  ; block 0?
;  int  010h
  ret
endp

proc ClearScreen
  ; Pre: The screen is all messy and sticky and needs to be cleansed of filth
  ; Post: The screen has been wiped clean
  ; *TODO* : Do this with a call to ClearVidBuffer and Flip
  ;          because now it does not actually clear the screen and should
  ;          be called 'SeekLastScreenLine'
  uses eax, ebx                ; ebx destroyed by _printf
  lea  eax, [strCLS]
  push eax
  call _printf
  add  esp, 4
  ret
  strCLS db 50 dup (newline)
         db 36,10
endp

proc Set80x50 ; Destroys none
  ; Sets DOS textmode 80x50
  ; Pre: -
  ; Post: Guess
  ; Note: I'm kind of hoping int10 only changes ax,bx
  uses ax, bx
  mov  ah, 0fh                 ; get current video mode
  int  010h                    ; ah=#cols, al=mode, bh=active page
  mov  [byte ptr OldVideoMode], al ; save it
  mov  ax, 0003h               ; setting high bit of al prevents CLS (don't do that here)
  int  010h
  mov  ax, 01112h              ; load 8x8 font
  xor  bx, bx                  ; block 0
  int  010h

; This commented block of code is how the mode change is performed
; by the old code, but I don't know exactly what is does.
; The first interrupt call bothers me (ax=0x1202/bx=0x0003/int 0x10)
; since I can't even find it's meaning in Ralph's list.
; but it seemed to work ...
;---Begin--
;  mov  ax, 01202h
;  mov  bx, 0003h
;  int  010h
;  mov  ax, 0003h
;  int  010h
;  mov  ax, 01112h
;  xor  bx, bx
;  int  010h
;--End--
  ret
endp Set80x50

proc NoBlink ; Destroys none
  ; Disables the blink bit
  ; Pre: -
  ; Post: Blinking is disabled
  uses ax, bx
  mov  ax, 01003h
  xor  bx, bx
  int  010h
  ret
endp NoBlink

proc DoBlink ; Destroys none
  ; Enables the blink bit
  ; Pre: -
  ; Post: Blinking is reenabled
  uses ax, bx
  mov  ax, 01003h
  xor  bx, bx
  inc  bx
  int  010h
  ret
endp DoBlink

proc int_print nolanguage; Destroys none, but is t3h 3vil
;int_print: ; ax==number
pusha
push  eax
lea   ebx, [int_print_warn]
push  ebx
call  _printf
pop   ebx
pop   eax
xor   cx,cx
push  cx
mov   ecx, 10
whilenotdone:
xor   edx,edx
;idiv : div edx:eax, result = eax=quoti, edx=remain
idiv  ecx
; edx = eax mod 10 (?)
add   edx, '0'
push  dx
cmp   eax,0
jnz whilenotdone
; hopefully we now have the sting on the stack, in reverse
mov   bx,ds
push  cs
pop   ds
lea   edx, [scratchpad]
print_int_print:
pop   ax
cmp   ax, 0
je    end_int_print
mov   [cs:scratchpad],al
mov   ah, 09h
int   21h
jmp   short print_int_print
end_int_print:
lea   dx,[strcrlf]
mov   ah,09h
int   21h
mov   ds,bx
popa
ret
strcrlf db 10,13,36
scratchpad db 0,36
int_print_warn db 'Warning: Someone is using `int_print()''!', 0ah, 0dh,'$', 0
endp int_print
dataseg
udataseg
OldVideoMode db ?
FontData db 8*256 dup (?)
RMCallStruct db 32 dup (?)  ; Real mode call struct
end
