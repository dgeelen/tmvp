TITLE Decode
.386
.model flat, Cpp
ideal
include "globals.inc"
include "macros.mac"

locals ;@@                     ; Use '@@' as local symbol prefix
                               ; '@@' is default
                               ; Must be 2 chars
;jumps            

codeseg
proc Decoder

endp

dataseg
strFileCloseError db 'Error while closing videofile! (error #%i)',newline,36,0
udataseg
buffer db 1024*128 dup (?) 
end