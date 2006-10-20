uses crtclone, dos;

const
  CODE_BUFSIZE=17;
  COMP_BUFSIZE=9;
  CODE_MAX=$EFFF;
  CODE_RST=$FFFE;

type
  tcarr = array[0..CODE_MAX-1] of byte;
  tcodebuff = array[0..(CODE_BUFSIZE*1024*2)-1] of byte;
  tcompbuff = array[0..(COMP_BUFSIZE*1024)-1] of word;
  tPalette = array[0..47] of byte; { 48 x 3 bytes }
  tFont = array [0..2047] of byte; { 8 bytes / char }
var
   codebuf: tcodebuff;
   compbuf: tcompbuff;
   palette: tPalette;
   font : tFont;

   infile: file;
   debugfile: file of word;
   c_char: ^tcarr;
   c_codehi: ^tcarr;
   c_codelo: ^tcarr;
   debugword: word;
   debugbyte: word;
   xhulpvar: word;
   ffseen:word;

   framebuf: array[0..4000-1] of word;
   z: word;
   flipflag: boolean;

 OLDEXITPROC:POINTER;
 OLDTIMERPROC:PROCEDURE;

   itimer : boolean;


procedure set80x50; assembler;
asm
{  mov ax,z;
  mov ax,00003h
  int 10h
  mov ax,01112h
  int 10h }
  {* Ripped from AAVP.COM *}
  mov ax, 1202h
  mov bx, 0003h
  int 10h
  mov ax, 0003h
  int 10h
  mov ax, 1112h
  xor bx,bx
  int 10h
end;

procedure set80x25; assembler;
asm
  mov ax,00003h
  int 10h
end;

procedure setPalette; assembler;
asm
  {Assume: DS == DataSeg}
{ Note on the palette structure:
  indexes 0-6 are colors 0 - 5 and 7
  indexes 7-14 are colors 8-15
  index 15 is color 6 }
  mov ax, seg palette
  mov ds, ax
  mov si, offset palette[0]
  mov cl, 0 { first 8 palette entries }
  xor cx, cx
  mov dx, 03C9h
@l1:
  mov al,cl  { al = color# }
  dec dx     { dx = 03C8h = select color }
  out dx, al { select color }
  inc dx     { dx = 03C9h = modify color }
  {Note: l00p unrolled for t3h sp33d }
  lodsb      { get a palette byte into al }
  out dx, al { output R }
  lodsb      { get a palette byte into al }
  out dx, al { output G }
  lodsb      { get a palette byte into al }
  out dx, al { output B }
  inc cl
  cmp cl, 6
  jne @l1

  mov al,20  { al = color# }
  dec dx     { dx = 03C8h = select color }
  out dx, al { select color }
  inc dx     { dx = 03C9h = modify color }
  {Note: l00p unrolled for t3h sp33d }
  lodsb      { get a palette byte into al }
  out dx, al { output R }
  lodsb      { get a palette byte into al }
  out dx, al { output G }
  lodsb      { get a palette byte into al }
  out dx, al { output B }

  inc cl
  mov al,cl  { al = color# }
  dec dx     { dx = 03C8h = select color }
  out dx, al { select color }
  inc dx     { dx = 03C9h = modify color }
  {Note: l00p unrolled for t3h sp33d }
  lodsb      { get a palette byte into al }
  out dx, al { output R }
  lodsb      { get a palette byte into al }
  out dx, al { output G }
  lodsb      { get a palette byte into al }
  out dx, al { output B }
  inc cl
  add cl, 48 { next 8 palette entries }
@l2:
  mov al,cl  { al = color# }
  dec dx     { dx = 03C8h = select color }
  out dx, al { select color }
  inc dx     { dx = 03C9h = modify color }
  {Note: l00p unrolled for t3h sp33d }
  lodsb      { get a palette byte into al }
  out dx, al { output R }
  lodsb      { get a palette byte into al }
  out dx, al { output G }
  lodsb      { get a palette byte into al }
  out dx, al { output B }
  inc cl
  cmp cl, 64
  jne @l2

  @exit:
  end;
procedure noblink; assembler;
asm
  mov ax,01003h
  xor bx,bx;
  int 10h
end;
procedure doblink; assembler;
asm
  mov ax,01003h
  xor bx,bx;
  inc bx
  int 10h
end;


PROCEDURE Flip; ASSEMBLER;
ASM
  push DS
  push ES
  push SI

         MOV DX,03DAh

         @lopv0:    IN AL,DX
                 AND AL,8
                         JNZ @lopv0

           @lopv1:    IN AL,DX
                   AND AL,8
                           JZ @lopv1

  {*NEW* : Palette dinges}
  call setpalette {}

  mov CX, 8000 / 4
  mov AX, $B800
  db 66h; mov ES, AX
  xor DI, DI
  mov ax, seg framebuf
  mov ds, ax
  mov si, offset framebuf
  cld
  db 66h; rep movsw
  pop SI
  pop ES
  pop DS
{@Done:}
END;

{$F+}
PROCEDURE NEWTIMERPROC;INTERRUPT;
BEGIN
 IF (not flipflag) THEN BEGIN
   flip;
   flipflag := true;
 END else begin
{   writeln(' langzaam');}
 end;
 OLDTIMERPROC;
END;
{$F-}

PROCEDURE INEWTIMER;
BEGIN
 IF NOT ITIMER THEN BEGIN
  GETINTVEC($1C,@OLDTIMERPROC);
  SETINTVEC($1C,@NEWTIMERPROC);
  ITIMER:=TRUE;
 END;
END;

PROCEDURE IOLDTIMER;
BEGIN
 IF ITIMER THEN BEGIN
  SETINTVEC($1C,@OLDTIMERPROC);
  ITIMER:=FALSE;
 END;
END;

PROCEDURE OUREXITPROC;FAR;
BEGIN
 IOLDTIMER;
 EXITPROC:=OLDEXITPROC;
END;

procedure setfont; assembler;
asm
  mov  ax, $1100 {load user font}
  mov  bh, 8     {bytes per char}
  mov  bl, 0     {block 0}
  mov  cx, $0100 {all chars}
  mov  dx, $0000 {magic?}
  lea  bp, font
  int  $10
end;

procedure showfont;
var
  i:integer;
  x:integer;
  y:integer;
  c:char;
begin
  asm
    pusha
    push es
    push di
    mov ax, $b800
    mov es, ax
    xor di, di
    mov ax, $0700
    mov bx, $0010
@rowl:
    mov cx, $0010
@charl:
    stosw {ax -> es:di ; di++}
    inc  al
    loop @charl
    add  di,$80
    dec  bx
    jnz  @rowl
    pop  di
    pop  es
    popa
    end;
  end;


var
  read: word;
  readpos: word;
  readfil: word;
  ncodes: word;
  i: word;
  tcode, tc2, lcode: word;
  s: string;
  sbuf: array[0..255] of byte;
  lbuf: byte;
  off: boolean;
  decstart: word;
  decstop: word;
  decspace: word;
begin
  asm
    mov bx, [123]
    mov ax, [123 + bx]
    db 66h; mov ax, [123 + bx]
    db 67h; mov ax, [123 + bx]
    db 66h; db 67h; mov ax, [123 + bx]
  {  mov ax, [2*bx]}
  end;

  if (paramcount = 1) then
    assign(infile,paramstr(1))
  else
    assign(infile,'c:\stuff\mkr.aa');
  reset(infile,2);
  blockread(infile,font[0],sizeof(font) div 2);

  writeln('MAGiC'); { DO NOT REMOVE THIS LINE, THINGS *WILL* BREAK }
  asm
    call set80x50
    call setfont  { }
    end;
  noblink;

  OLDEXITPROC:=EXITPROC;
  EXITPROC:=@OUREXITPROC;

  inewtimer;

  port[$43] := $36;
  port[$40] := $21;
  port[$40] := $C2;

  new(c_char);
  new(c_codehi);
  new(c_codelo);

  for i := 0 to 255 do begin
    c_char^[i] := i;
    c_codehi^[i] := $FF;
    c_codelo^[i] := $FF;
  end;

  ncodes := 256;

  readpos := readfil;

{  assign(debugfile,'asmde.lzw');
  rewrite(debugfile); }

  asm
    mov ax, offset codebuf
    mov decstart, ax
    mov decstop, ax
  end;

  lcode := $FFFF;
  off := false;
  flipflag:=true;
  debugword:=0;
  randomize;

{  blockread(infile, palette[0], 24);
asm  db $cc end;
  asm
    call setPalette
    end;
{  for i:=0 to 15 do begin
    palette[i*3]:=random(64);
    palette[i*3+1]:=random(64);
    palette[i*3+2]:=random(64);
    end;{}
{    palette[16*3-1]:=63;{}
{  setPalette;{}

  repeat  {    blockread(infile, framebuf, 8000, read);}
    repeat
      if (readpos = readfil) then begin
        blockread(infile, compbuf, sizeof(compbuf) div 2, readfil);
        readpos := 0;
      end;

      asm
        mov ax, decstart
        cmp decstop, ax
        jb @nosub
        add ax, CODE_BUFSIZE*1024*2
      @nosub:
        sub ax, decstop
        mov decspace, ax
      end;


      while (decspace > 1*1024) and (readpos < readfil) do begin
        tcode := compbuf[readpos];

        if (tcode = CODE_RST) then begin
          ncodes:= 256;
          lcode := $FFFF;
        end else begin
(*          tc2 := tcode;
d
          s:='';
          if (tc2 < ncodes) then begin
            repeat
              s := char(c_char^[tc2]) + s;
              tc2 := (c_codehi^[tc2] shl 8) or (c_codelo^[tc2]);
            until tc2=$FFFF;
          end else begin
            tc2 := lcode;
            repeat
              s := char(c_char^[tc2]) + s;
              tc2 := (c_codehi^[tc2] shl 8) or (c_codelo^[tc2]);
            until tc2=$FFFF;
            s := s + s[1];
          end;
*)
          asm
            xor cx, cx
            mov di, offset sbuf + $FF

            mov bx, tcode
            cmp bx, ncodes
            jae @spec
          @l1:
            les si, c_char
            mov al, es:[si+bx]
            mov ds:[di], al
            dec di
            inc cx

            les si, c_codehi
            mov ah, es:[si+bx]
            les si, c_codelo
            mov al, es:[si+bx]

            mov bx,ax
            cmp bx,$FFFF
            jne @l1
            inc di
            jmp @done
          @spec:
            dec di
            inc cx
            mov bx, lcode
          @l2:
            les si, c_char
            mov al, es:[si+bx]
            mov ds:[di], al
            dec di
            inc cx

            les si, c_codehi
            mov ah, es:[si+bx]
            les si, c_codelo
            mov al, es:[si+bx]

            mov bx,ax
            cmp bx,$FFFF
            jne @l2
            inc di
            mov al, ds:[di]
            mov ds:[offset sbuf + $FF], al
          @done:
            mov al, ds:[di]
            push ax
          end;

          asm
            mov ax, ds
            mov es, ax

            sub decspace, cx

            mov si, di
            mov di, decstop
            mov dx, offset codebuf + CODE_BUFSIZE*1024*2
            sub dx, di

            cmp cx, dx
            jb @go

            sub dx, cx
            add cx, dx
            neg dx

            rep movsb

            sub di, CODE_BUFSIZE*1024*2

            mov cx, dx
          @go:
            rep movsb

            mov decstop, di
          end;

          asm
            pop ax
            mov dx, lcode
            cmp dx, $FFFF
            je @done
            mov bx, ncodes
            cmp bx, CODE_MAX
            jae @done

            les si, c_char
            mov es:[si+bx], al

            mov ax, dx

            les si, c_codehi
            mov es:[si+bx], ah
            les si, c_codelo
            mov es:[si+bx], al
            inc bx
            mov ncodes, bx
          @done:
          end;
          lcode := tcode;
        end;
        inc(readpos);
      end;

      asm
        mov ax, decstop
        cmp decstart, ax
        jbe @le
        add ax, CODE_BUFSIZE*1024*2
      @le:
        sub ax, decstart
        mov decspace, ax
      end;
    until (((decspace > 10 * 1024) or (readfil = 0)) and (flipflag));
{Assume: decspace > 10*1024 => plek zat om ook Palette te lezen
{        Gebeurt _NA_ decoden frame

{ Attempt RLE Dump
  debugword := decstart;
  debugbyte := 0;
  while (decspace > 1) and (debugbyte<>$FFFF) do begin
    asm
      cld
      mov ax, ds
      mov es, ax
      mov si, debugword

      cmp si, offset codebuf + CODE_BUFSIZE*1024*2
      jb @nosub
      sub si, CODE_BUFSIZE*1024*2
    @nosub:
      cmp si, decstop
      je @exit
      lodsw

    @exit:
      mov debugbyte, ax
      mov debugword, si
    @done:
    end;
    write(debugfile, debugbyte);
    dec(decspace);
    dec(decspace);
  end; { }

{ dit hier doet de RLE (CodeBuf) to FrameBuf }
    asm
      cld
      mov ax, ds
      mov es, ax
      mov di, offset framebuf
      mov si, decstart
    @loop:
      cmp si, offset codebuf + CODE_BUFSIZE*1024*2
      jb @nosub
      sub si, CODE_BUFSIZE*1024*2
    @nosub:
      cmp si, decstop
      je @exit
      lodsw
      cmp ax, $FFFF
      je @done

      mov dx, offset codebuf + CODE_BUFSIZE*1024*2
      sub dx, si        { dx := #words left in buffer }
      shr dx, 1

      xor cx, cx        { cx := #words to copy }
      mov cl, al

      cmp dx, cx        { if dx >= cx goto @go }
      jge @go

      mov cx, dx
      sub al, cl
      rep movsw
      sub si, CODE_BUFSIZE*1024*2
      xor cx,cx
      mov cl,al

    @go:
      rep movsw
      shr ax, 8
      add di, ax
      add di, ax
      jmp @loop
    @exit:
    @done: {actually this should read 'DoPalette' or so}
{//    db $cc}
      push es { Dunno if es:di is used at this point }
      push di { let's save it }
      mov  ax, seg Palette
      mov  es, ax
      mov  di, offset Palette[0]

      mov  cx, 24

      mov dx, offset codebuf + CODE_BUFSIZE*1024*2
      sub dx, si        { dx := #words left in buffer }
      shr dx, 1

      cmp dx, cx        { if dx >= cx goto @go }
      jge @fillpalette

      { cx, dx := dx, cx - dx } { whoa! multiple assignment!}
      sub dx, cx
      add cx, dx
      neg dx

      rep movsw

      sub si, CODE_BUFSIZE*1024*2

      mov cx, dx

@fillpalette:
{      lodsw {AX <- DS:SI ; SI++ }
{      stosw {AX -> ES:DI ; DI++ }
      rep movsw
      pop  di
      pop  es

{      add si,48        {we're going to read some Palette bytes}
      mov decstart, si

    end;                {  }

    flipflag := false;
{    flip;             }
  until (keypressed) or ((decstart = decstop) and (readfil = 0));
  readkey;
  port[$43] := $36;
  port[$40] := $FF;
  port[$40] := $FF;
  set80x25;
  noblink;
  close(infile);
  writeln('Program finished');

end.
