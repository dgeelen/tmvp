; NOTE:
; This code was taken mostly unchanged from an article in the
; 'Assembly Programming Journal', Issue 2. The article was
; 'THE.C.STANDARD.LIBRARY.IN.ASSEMBLY' and was written by Xbios2.
; Many thanks for the code :)
; sprintf.asm ============================================================
title vsprintf
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"

macro getarg register
	lea	eax, [a_argList]
	mov	edx, [eax]
	add	[dword ptr eax], 4
	mov	register, [edx]
endm

codeseg
proc _vsprintf
  arg a_output:dword, a_format:dword, a_argList:dword
	local v_width:dword, v_prec:dword, v_zeroLen:dword, v_sign:dword, v_strbuf:byte:12, v_strLen:dword
  uses ebx, ebx, ecx, edx, esi, edi, es

		mov	esi, [a_format]
		mov	edi, [a_output]
    mov eax, ds
    mov es, eax
    cld

mainLoop:	lodsb				; get character
		cmp	al, '%'			; test if it is '%'
		je	short controlChar
		stosb				; if not, just copy it
		test	al, al
		jnz	short mainLoop		; if char is not NULL, loop
		jmp	EndOfString		; jump if char is null
; ---------------------------------------------------------------------------

controlChar:	xor	ecx, ecx		; set stage to 0
		or	eax, -1
		xor	ebx, ebx		; no flags set
		mov	[v_width], eax		; no width given
		mov	[v_zeroLen], ecx	; 0
		mov	[v_prec], eax		; no .prec given
		mov	[v_sign], ecx		; 0, no sign prefix

formatLoop:	xor	eax, eax
		lodsb
		cmp	al, ' '
		jl	unknown_char			; char below ' '
		movzx	edx, [byte ptr xxlat + eax - ' ']
		jmp	[dword ptr jumptable +edx*4]	; we jump with the char in AL
; ---------------------------------------------------------------------------
n_CharsWritten:	getarg	eax
		mov	edx, edi
		sub	edx, [a_output]		; calculate length

		test	ebx, 16
		jnz	short nchars_short

		mov	[eax], edx
		jmp	short fw_mainloop

nchars_short:	mov	[eax], dx
fw_mainloop:	jmp	mainLoop
; ---------------------------------------------------------------------------
Percent:	cmp	[byte ptr esi-2], al	; al='%'
		jne	unknown_char
		stosb
		jmp	mainLoop
; ---------------------------------------------------------------------------
; flag characters
HashSign:	or	ebx, 1
		jmp	short chkflags
MinusSign:	or	ebx, 2
		jmp	short chkflags
BlankOrPlus:	or	[byte ptr v_sign], al	; ' ' will become '+'
chkflags:	or	ecx, ecx
		jnz	unknown_char
		jmp	formatLoop
; ---------------------------------------------------------------------------
Asterisk:	getarg	eax

		cmp	ecx, 2
		jge	short asterisk_prec
		test	eax, eax
		jge	short width_positive
		neg	eax
		or	ebx, 2

width_positive:	mov	[v_width], eax
		mov	ecx, 3
		jmp	short fwwB
; - - - - - - - - - - - - - - - - - - - - - - -
asterisk_prec:	cmp	ecx, 4
		jnz	unknown_char
		inc	ecx			; set stage to 5
		mov	[v_prec], eax

fwwB:		jmp	formatLoop
; ---------------------------------------------------------------------------
Dot:		cmp	ecx, 4
		jge	unknown_char
		mov	ecx, 4
		inc	[v_prec]		; set .prec to 0
		jmp	formatLoop
; ---------------------------------------------------------------------------
Digit:		sub	al, '0'			; convert ASCII to value
		jnz	short digit2
		or	ecx, ecx
		jnz	short digit2
		test	ebx, 2			; we come here if width=0n
		jnz	short fwwC
		or	ebx, 8
		inc	ecx			; set stage to 1
		jmp	fwwC
; - - - - - - - - - - - - - - - - - - - - - - -
digit2:		cmp	ecx, 2
		jg	short digit_prec
		mov	ecx, 2
		cmp	[v_width], 0
		jge	short digit_width
		mov	[v_width], eax
		jmp	short fwwC
; - - - - - - - - - - - - - - - - - - - - - - -
digit_width:	imul	edx, [v_width], 10
		add	eax, edx
		mov	[v_width], eax
		jmp	short fwwC
; - - - - - - - - - - - - - - - - - - - - - - -
digit_prec:	cmp	ecx, 4
		jnz	unknown_char
		imul	edx, [v_prec], 10
		add	eax, edx
		mov	[v_prec], eax

fwwC:		jmp	formatLoop
; ---------------------------------------------------------------------------
h_shortint:	or	ebx, 16
		mov	ecx, 5
		jmp	formatLoop
; ---------------------------------------------------------------------------
o_octal:	mov	ecx, 8			; radix
		test	ebx, 1
		jz	short unsigned
		mov	[byte ptr v_sign], '0'
		jmp	short integer

u_unsigned:	mov	ecx, 10			; radix
unsigned:	mov	[byte ptr v_sign], 0	; no sign
		jmp	short integer

x_Hexadecimal:	mov	ecx, 16			; radix
		mov	ah, al
		xor	al, 'X'			; AL is the char ('x' or 'X')
		mov	bh, al
		test	ebx, 1
		jz	short integer
		mov	al, '0'
		mov	[word ptr v_sign], ax
		jmp	short integer

d_decimal:
    mov	ecx, 10			; radix
		or	ebx, 32

integer:	getarg	eax
		test	ebx, 16
		jz	short integer_cnvt	; if not short, don't change

short_integer:	test	ebx, 32			; is integer signed?
		jnz	short short_signed
		and	eax, 0FFFFh		; zero extend 16 to 32
		jmp	short nosign
short_signed:	cwde				; sign extend 16 to 32

integer_cnvt:	test	ebx, 32
		jz	nosign
		or	eax, eax
		jns	nosign
		neg	eax
		mov	[byte ptr v_sign], '-'

nosign:		lea	edx, [offset v_strbuf + 11]
		or	eax, eax
		jnz	short ltoa
		cmp	[v_prec], eax		; eax is 0 if we are here
		jnz	short zero
		mov	[byte ptr edx], al	; value 0 with .0 prec
		mov	[v_strLen], eax		; means no string
		jmp	printit			; so output no digits

zero:		cmp	[byte ptr v_sign], '0'
		jnz	short ltoa
		mov	[byte ptr v_sign], 0	; we don't want 0x0, nor '00'

	; convert EAX into ASCII
ltoa:		push	edi
		push	esi
		xor	esi, esi
		mov	edi, edx
		mov	[byte ptr edi], 0

ltoaLoop:	xor	edx, edx
		div	ecx			; ecx is the radix
		xchg	eax, edx
		add	al,90h
		daa
		adc	al,40h
		daa
		or	al, bh			; switch case if needed
		dec	edi
		inc	esi
		mov	[edi], al
		xchg	eax, edx
		or	eax, eax
		jnz	short ltoaLoop

		mov	eax, esi
		mov	edx, edi
		pop	esi
		pop	edi

		mov	[v_strLen], eax
		mov	ecx, [v_prec]
		or	ecx, ecx
		js	noprec

	; A precision was given
		sub	ecx, eax
		jle	short skipzerolen
		mov	[v_zeroLen], ecx	; if prec>digits then
						; add (prec-digits) '0'
		jmp	short skipzerolen

noprec:		test	ebx, 8
		jz	short skipzerolen
		cmp	[v_width], 0
		jle	short skipzerolen

;------------------
; we come here if width=0n
		mov	ecx, [v_width]
		sub	ecx, eax		; EAX=[v_strLen]
		jle	short skipzerolen
		mov	eax, [dword ptr v_sign]
		or	al, al
		jz	short setzerolen
		dec	ecx
		shr	eax, 8
		jz	short setzerolen
		dec	ecx
		js	short skipzerolen
setzerolen:	mov	[v_zeroLen], ecx

skipzerolen:	mov	eax, [dword ptr v_sign]
		or	al, al
		jz	short finishint
		dec	[v_width]
		shr	eax, 8
		jz	short finishint
		dec	[v_width]

finishint:
    mov	eax, [v_zeroLen]
		add	[v_strLen], eax
		jmp	printit

; ---------------------------------------------------------------------------
; Pointer: same as %.8X

p_pointer:	getarg	ecx

		lea	edx, [v_strbuf]
		push	ebx
		mov	ebx, 7
loopPointer:	mov	al, cl
		shr	ecx, 4
		and	al, 0Fh
		add     al,90h
		daa
		adc     al,40h
		daa
		mov	[edx+ebx], al
		dec	ebx
		jns	loopPointer
		pop	ebx

		mov	[byte ptr edx+8], 0
		mov	[v_strLen], 8
		jmp	printit
; ---------------------------------------------------------------------------
c_char:		getarg	eax
		lea	edx, [v_strbuf]
		mov	[edx], eax		; stores char (rest of EAX is
						; not important)
		mov	[v_strLen], 1		; set length to one char
		jmp	printit
; ---------------------------------------------------------------------------
s_string:	getarg	edx
		or	eax, -1
		test	edx, edx
		jnz	short strlen_I
		mov	edx, offset Null	; Pointer 0 prints 'Null'
strlen_I:	inc	eax
    cmp	[byte ptr edx+eax], 0
		jnz	short strlen_I

		cmp	eax, [v_prec]
		jle	short setLen
		cmp	[v_prec], 0
		jl	short setLen
		mov	eax, [v_prec]

setLen:		mov	[v_strLen], eax

; ---------------------------------------------------------------------------
; we must arrive here with EDX pointing to the string to print
; and it's length in [v_strLen]

	; left pad with spaces IF necessary
printit:
    test	ebx, 2			; Is it left justified?
		mov	ebx, [v_width]
		jnz	short printPrefix	; if yes, don't pad left
		mov	ecx, ebx
		sub	ecx, [v_strLen]
		jle	printPrefix
		mov	al, ' '
		rep stosb			; >>> left pad
		mov	ebx, [v_strLen]

	; print one- or two-chars PREFIX
printPrefix:	mov	eax, [v_sign]
		or	al, al
		jz	short padZero
		stosb				; print the sign prefix
		shr	eax, 8			; AL=AH, AH=0
		jz	short padZero
		stosb				; print the sign prefix

	; pad with zeroes IF necessary
padZero:	mov	ecx, [v_zeroLen]	; we are sure that ecx>=0
		sub	[v_strLen], ecx
		sub	ebx, ecx
		mov	al, '0'			; ECX=[v_zeroLen]
		rep stosb			; >>> pad with 0s
		mov	ecx, [v_strLen]
		sub	ebx, ecx
		xchg	esi, edx
		rep movsb			; >>> copy string
		xchg	esi, edx
		js	short skipRightpad	; refers to SUB EBX, ECX
		mov	ecx, ebx
		mov	al, ' '
		rep stosb			; >>> right pad with ' '
skipRightpad:	jmp	mainLoop
; ---------------------------------------------------------------------------
;
; If an	unknown	specification character is found, _vsprintf enters the
; following loop. This loop copies verbatim all the rest of the string
; (from the '%' on)

unknown_char:	mov	al, '%'
scanback:	dec	esi
		cmp	[esi], al
		jne	short scanback
copyrest:	lodsb
		stosb
		test	al, al
		jnz	short copyrest
;
; ---------------------------
; return the number of chars written

EndOfString:	mov	eax, edi
		sub	eax, [a_output]
		dec	eax
		ret
endp

; --- DATA ---------------------------------------------------------
dataseg
Null		db '(null)',0
		align 4
jumptable dd offset BlankOrPlus	; 0
          dd offset HashSign	; 1
          dd offset Asterisk	; 2
          dd offset MinusSign	; 3
          dd offset Dot		; 4
          dd offset Digit		; 5
          dd offset h_shortint	; 6
          dd offset d_decimal	; 7
          dd offset o_octal	; 8
          dd offset u_unsigned	; 9
          dd offset x_Hexadecimal	; 10
          dd offset p_pointer	; 11
          dd offset unknown_char	; 12 = f_floating
          dd offset c_char	; 13
          dd offset s_string	; 14
          dd offset n_CharsWritten ; 15
          dd offset formatLoop	; 16 = Ignore character
          dd offset unknown_char	; 17 = Unknown char
          dd offset Percent	; 18

	;       !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
xxlat	db  0, 17, 17,  1, 17, 18, 17, 17, 17, 17,  2,  0, 17,  3,  4, 17
	;   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
	db  5,  5,  5,  5,  5,  5,  5,  5,  5, 17, 17, 17, 17, 17, 17, 17
	;   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
	db 17, 17, 17, 17, 17, 12, 16, 12,  8, 17, 17, 17, 16, 17, 16, 17
	;   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
	db 17, 17, 17, 17, 17, 17, 17, 17, 10, 17, 17, 17, 17, 17, 17, 17
	;   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
	db 17, 17, 17, 13,  7, 12, 12, 12,  6,  7, 17, 17, 16, 17, 15,  8
	;   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL
	db 11, 17, 17, 14, 17,  9, 17, 17, 10, 17, 17, 17, 17, 17, 17, 17

end
; EOF ====================================================================
