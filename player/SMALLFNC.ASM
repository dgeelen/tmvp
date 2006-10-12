TITLE smallfunctions
.386
.model flat
ideal
include "globals.inc"

codeseg

proc SetPalette
  ; Sets the palette from palPtr
  ; Pre: Pointer to [palette] is present on the stack
  ; Post: The palette has been set
  arg  palPtr:dword
  push bp
  mov  bp, sp


  pop  bp
  ret
endp SetPalette

proc UpdateDisplay
  ; Outputs the current frame to the frame buffer and sets the palette
  ; Pre: Pointer to [palette] and [framedata] are present on the stack
  ; Post: The screen and palette have been updated
  arg  framePtr:dword, palPtr:dword
  push bp ax cx
  mov  bp, sp
  call setPalette              ; palPtr already on the stack
  mov  cx, 2000                ; 8000 bytes / 2000 dwords

  ;Do stuff with ProtectedMode

  pop  cx ax bp
  ret 
endp UpdateDisplay

proc KeyPressed ; Destroys ax
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
 ;Note: I'm kind of hoping int10 only changes ax
  push ax
  mov  ax, 0003h
  int  010h
  pop  ax
  ret
endp Set80x25

proc Set80x50 ; Destroys none
 ; Sets DOS textmode 80x50
 ; Pre: -
 ; Post: Guess
 ;Note: I'm kind of hoping int10 only changes ax,bx
  push ax bx  
  mov  ax, 01202h
  mov  bx, 0003h
  int  010h
  mov  ax, 0003h
  int  010h
  mov  ax, 01112h
  xor  bx, bx
  int  010h
  pop  bx ax
  ret
endp Set80x50

proc NoBlink ; Destroys none
  ; Disables the blink bit
  ; Pre: -
  ; Post: Blinking is disabled
  push ax bx
  mov  ax, 01003h
  xor  bx, bx
  int  010h
  pop  bx ax
  ret
endp NoBlink

proc DoBlink ; Destroys none
 ; Enables the blink bit
 ; Pre: -
 ; Post: Blinking is reenabled
 push ax bx
 mov  ax, 01003h
 xor  bx, bx
 inc  bx
 int  010h
 pop  ax bx
 ret
endp DoBlink
dataseg

end