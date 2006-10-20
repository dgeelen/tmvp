uses crtclone;

const
  CODE_BUFSIZE=18;
  COMP_BUFSIZE=1;
  CODE_MAX=$EFFF;
  CODE_RST=$FFFE;

type
  tcarr = array[0..CODE_MAX-1] of byte;
  tcodebuff = array[0..(CODE_BUFSIZE*1024*2)-1] of byte;
  tcompbuff = array[0..(COMP_BUFSIZE*1024)-1] of word;
var
   codebuf: tcodebuff;
   compbuf: tcompbuff;

   infile: file;

   c_char: ^tcarr;
   c_codehi: ^tcarr;
   c_codelo: ^tcarr;


   framebuf: array[0..4000-1] of word;
   z: word;

procedure set80x50; assembler;
asm
  mov ax,z;
  mov ax,00003h
  int 10h
  mov ax,01112h
  int 10h
end;

PROCEDURE Flip; ASSEMBLER;
ASM
{  cmp Screen.DBuffer, 0}
{  je @Done}
  push ES
  push SI
  mov CX, 8000 / 4
  mov AX, $B800
  db 66h; mov ES, AX
  xor DI, DI
  mov si, offset framebuf
  cld
  db 66h; rep movsw
  pop SI
  pop ES
{@Done:}
END;

var
  read: word;
  readpos: word;
  readfil: word;
  ncodes: word;
  i: word;
  tcode, tc2, lcode: word;
  s: string;
  off: boolean;

begin
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

  set80x50;
  if (paramcount = 1) then
    assign(infile,paramstr(1))
  else
    assign(infile,'c:\stuff\b800.lzw');
  reset(infile,2);

  asm
    push $FFFF        { ax : value }
    push seg framebuf { es : dest seg }
    push di           { di : dest ofs }
  end;

  lcode := $FFFF;
  off := false;

  repeat  {    blockread(infile, framebuf, 8000, read);}
    if (readpos = readfil) then begin
      blockread(infile, compbuf, sizeof(compbuf) div 2, readfil);
      readpos := 0;
    end;
    if (off) then begin
      codebuf[0] := codebuf[read];
      read := 1;
      off := false;
    end else
      read := 0;

    i := readpos;
    while (i < readfil) do begin

      tcode := compbuf[i];
    if (tcode = CODE_RST) then begin
      ncodes:= 256;
      lcode := $FFFF;
    end else begin
      tc2 := tcode;
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

      move(s[1], codebuf[read], length(s));
      inc(read, length(s));

      if (lcode <> $FFFF) and (ncodes < CODE_MAX) then begin
        c_char^[ncodes]   := byte(s[1]);
        c_codehi^[ncodes] := lcode shr 8;
        c_codelo^[ncodes] := lcode and $FF;
        inc(ncodes);
      end;
      lcode := tcode;
    end;
      inc(i);
    end;
    readpos := readfil;
    if (read and 1 = 1) then begin
       off := true;
       dec(read);
    end;
    if (read and 1 = 1) then begin
       off := true;
       dec(read);
    end;
    asm
      cld
      pop di
      pop es
      pop ax

      cmp read, 0               { check for eof }
      je @exit

      mov bx, read
      mov si, offset codebuf
      add bx, si

    @loop:
      cmp si, bx
      jge @done
      cmp ax, $FFFF
      jne @else
      call flip
      mov di, offset framebuf
    @nxt:
      lodsw
      jmp @loop
    @else:
      cmp ax, $0000
      je @nxt

      mov dx, bx        { dx := #words left in buffer }
      sub dx, si
      shr dx, 1

      xor cx, cx        { cx := #words to copy }
      mov cl, al

      cmp dx, cx        { if dx >= cx goto @go }
      jge @go

      mov cx, dx
      sub al, cl
      rep movsw
      jmp @done

    @go:
      rep movsw
      shr ax, 8
      add di, ax
      add di, ax
      xor ax, ax
      jmp @loop
    @done:
      push ax
      push es
      push di
    @exit:
    end;

  until (read=0) or (keypressed);

  { restore stack }
  asm
  end;

{  close(infile);}


end.
