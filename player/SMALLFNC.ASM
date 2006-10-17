TITLE smallfunctions
.386
.model flat, Cpp
ideal
include "globals.inc"

codeseg

proc SetPalette
  ; Sets the palette from palPtr
  ; Pre: Pointer to [palette] is present on the stack
  ; Post: The palette has been set
  arg  palPtr:dword
  uses eax
endp SetPalette

proc UpdateDisplay
  ; Outputs the current frame to the frame buffer and sets the palette
  ; Pre: Pointer to [palette] and [framedata] are present on the stack
  ; Post: The screen and palette have been updated
  arg  framePtr:dword, palPtr:dword
  uses ax, cx
  mov  bp, sp
  call setPalette              ; palPtr already on the stack
  mov  cx, 2000                ; 8000 bytes / 2000 dwords

  ;Do stuff with ProtectedMode

  ret 
endp UpdateDisplay

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
endp KeyPressed

proc GetKey
  ; Read a keystroke from the keybuffer
  
  ret
  
endp GetKey

proc Set80x25 ; Destroys none
  ; Sets DOS textmode 80x50
  ; Pre: -
  ; Post: Guess
  ; Note: I'm kind of hoping int10 only changes ax
  uses ax
  mov  ax, 0003h
  int  010h
  ret
endp Set80x25

proc Set80x50 ; Destroys none
  ; Sets DOS textmode 80x50
  ; Pre: -
  ; Post: Guess
  ; Note: I'm kind of hoping int10 only changes ax,bx
  uses ax, bx  
  mov  ax, 01202h
  mov  bx, 0003h
  int  010h
  mov  ax, 0003h
  int  010h
  mov  ax, 01112h
  xor  bx, bx
  int  010h
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
lea   ebx, [int_print_warn]
push  ebx
call  _printf
pop   ebx
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
lea   dx,[crlf]
mov   ah,09h
int   21h
mov   ds,bx
popa 
ret
crlf db 10,13,36
scratchpad db 0,36
int_print_warn db 'Warning: Someone is using `int_print()''!', 0ah, 0dh,'$', 0
endp int_print
dataseg

end
