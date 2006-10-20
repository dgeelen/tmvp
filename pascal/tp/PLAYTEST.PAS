uses dos, crt;

type
   tFont = array [0..2047] of byte; { 8 bytes / char }

var
   framebuf: array[0..4000-1] of word;
   infile1: file;
   infile2: file;
   infile3: file;
   font : tFont;

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

PROCEDURE Flip; ASSEMBLER;
ASM
  push DS
  mov CX, 8000 / 4
  mov AX, $B800
  db 66h; mov ES, AX
  xor DI, DI
  mov ax, seg framebuf
  mov ds, ax
  mov si, offset framebuf
  cld
  db 66h; rep movsw
  pop DS
END;

procedure set80x50; assembler;
asm
  mov ax,00003h
  int 10h
  mov ax,01112h
  int 10h
end;

procedure SetBlinkEGAVGA({input } TurnOn : boolean);
begin
  asm
    mov ax, 1003h
    mov bl, TurnOn
    int 10h
  end
end;

type
    pbyte= ^byte;
var
 i: integer;
 int08:pointer;
 cnt1:^word;
 cnt2:^word;
 flipflag:^byte;
 pbuf:pointer;
 sbuf:pointer;
 sbs,sbe:^word;
 ppal:pointer;
begin
  GetIntVec($08, int08);
  int08 := pointer(longint(int08) + 249);
  cnt1 := pointer(longint(int08) + 0);
  cnt2 := pointer(longint(int08) + 2);
  flipflag := pointer(longint(int08) + 6);
  pbuf := pointer(longint(int08) + 64);
  sbs := pointer(longint(int08) + 8);
  sbe := pointer(longint(int08) + 12);
  ppal := pointer(longint(int08) + 64 - 48);

  sbuf := pointer(longint(int08) + 8064);


{  repeat}
  assign(infile1, 'c:\bass.nor');
  assign(infile2, 'c:\stuff\b.aa');
  assign(infile2, 'c:\export.aap');
  assign(infile2, 'c:\render\output.aap');
{  assign(infile2, 'c:\render\blood10.aap');}
{  assign(infile2, 'c:\test.aap');}
  assign(infile3, 'c:\blocks.fon');
  reset(infile1, 8000);
  reset(infile2, 1);
  reset(infile3, 1);

  blockread(infile3,font[0],sizeof(font));

  close(infile3);

  writeln('MAGiC'); { DO NOT REMOVE THIS LINE, THINGS *WILL* BREAK }
  set80x50;
  setfont;
  SetBlinkEGAVGA(false);

  repeat
{    writeln(sbs^,'-',sbe^);}
{    if (sbe^ = 0) and ((sbs^=0) or (sbs^ > 8000)) then begin
     writeln('reading 0->8000 ',sbs^,'-',sbe^);
      blockread(infile1, pointer(longint(sbuf)+0)^, 1);
      sbe^ := 8000;
    end;
    if (sbe^ = 8000) and (sbs^<>0) and (sbs^ <= 8000) then begin
      writeln('reading 8000->0 ',sbs^,'-',sbe^);
      blockread(infile1, pointer(longint(sbuf)+8000)^, 1);
      sbe^ :=0;
    end;}
    if (flipflag^=1) then begin
      blockread(infile2, pbuf^, 8000);
      blockread(infile2, ppal^, 48);

      flipflag^ := 0;

    end;
{    flipflag^ := 0;}
  until eof(infile2) or keypressed;
  close(infile1);
  close(infile2);
{  until keypressed;}
{  readkey;}
  exit;

 for i:=0000 to 1000-1 do
    framebuf[i] := (4 shl 12)
                or (1 shl 8)
                or (176);
  for i:=1000 to 2000-1 do
    framebuf[i] := (4 shl 12)
                or (1 shl 8)
                or (177);
  for i:=2000 to 3000-1 do
    framebuf[i] := (1 shl 12)
                or (4 shl 8)
                or (178);
  for i:=3000 to 4000-1 do
    framebuf[i] := (4 shl 12)
                or (1 shl 8)
                or (ord('x'));
  flip;
  readln;

end.
