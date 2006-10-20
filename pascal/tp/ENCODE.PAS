uses crtclone;

const
  KBUFSIZE=32;

var
   buffer: array[0..(KBUFSIZE*1024)] of byte;
   infile: file;
   outfile: file;

   framebuf : array[0..4000-1] of word;
   encodebuf: array[0..4000-1] of word;
   framebufptr: pointer;

procedure set80x50; assembler;
asm
  mov ax,00003h
  int 10h
  mov ax,01112h
  int 10h
end;

PROCEDURE Flip; ASSEMBLER;
ASM
{  cmp Screen.DBuffer, 0}
{  je @Done}
  push DS
  mov CX, 8000 / 2
  mov AX, $B800
  db 66h; mov ES, AX
  xor DI, DI
  lds SI, framebufptr
  cld
  db 66h; rep movsw
  pop DS
{@Done:}
END;

var
 scrn: byte absolute $B800:$0000;
 read: word;
 i,j : word;
 icopy,iskip:word;
 v: word;

begin


  set80x50;
  framebufptr := @framebuf[0];
  assign(infile,'c:\export.aa');
  reset(infile,1);

  assign(outfile,'c:\export.rle');
  rewrite(outfile,2);

  repeat
    blockread(infile, framebuf, 8000, read);

    i := 0;
    while (i<4000) do begin
      icopy := 0;
      while (i<4000) and (framebuf[i] <> encodebuf[i])  and (icopy<254) do begin
        inc(icopy);
        inc(i);
      end;
      iskip := 0;
      while (i<4000) and (framebuf[i] = encodebuf[i]) and (i<4000) and (iskip<254) do begin
        inc(iskip);
        inc(i);
      end;
      v := iskip shl 8 or icopy;
      blockwrite(outfile, v, 1);
      blockwrite(outfile, framebuf[i-iskip-icopy], icopy);
    end;

    v := $FFFF;
    blockwrite(outfile, v, 1);
    move(framebuf, encodebuf, 8000);
  until (read=0) or eof(infile);

  close(infile);
  close(outfile);
  readln;



end.