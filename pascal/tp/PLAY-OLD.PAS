uses crtclone, dos;

const
  CODE_BUFSIZE=18;
  COMP_BUFSIZE=9;
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
   flipflag: boolean;

 OLDEXITPROC:POINTER;
 OLDTIMERPROC:PROCEDURE;

   itimer : boolean;


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
  push DS
  push ES
  push SI
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

  set80x50;

  asm
    mov ax, 1003h
    mov bl, 0
    int 10h
  end;

  if (paramcount = 1) then
    assign(infile,paramstr(1))
  else
    assign(infile,'c:\debug.lzw');
  reset(infile,2);

  asm
    mov ax, offset codebuf
    mov decstart, ax
    mov decstop, ax
  end;

  lcode := $FFFF;
  off := false;
  flipflag:=true;

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
    @done:
      mov decstart, si
    end;

    flipflag := false;
  until (keypressed) or ((decstart = decstop) and (readfil = 0));
{  readkey;}

  close(infile);


end.
