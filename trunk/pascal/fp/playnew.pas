uses dos, crt
{$IFDEF win32}
, video
{$ELSE}
, go32
{$ENDIF}
;

const
  MUSIC_HZ  = 16000.0;
  VIDEO_FPS = 24.0;

  MAXTIMER  = 1192736;
  COUNTDOWN = trunc(MAXTIMER / MUSIC_HZ);
  REAL_HZ   = MAXTIMER / COUNTDOWN;
  FRAMETIME = trunc(REAL_HZ / VIDEO_FPS);
  CHAIN     = trunc(REAL_HZ / 18.2);

  // value to program timer with
  TIMER_INT : word = COUNTDOWN;
  // amount of interrupts per frame
  FRAME_INT : word = FRAMETIME;
  // amount of interrupts per 1/18.2th second
  CHAIN_INT : word = CHAIN;

const
  // sizes in kilobytes
  CODE_BUFSIZE=64;
  COMP_BUFSIZE=32;

var
  // data is read from file into this buffer to be decompressed
  compbuf: array[0..(COMP_BUFSIZE*512  )-1] of word;
  // data is decompressed from compbuf into codebuf
  codebuf: array[0..(CODE_BUFSIZE*512*2)-1] of byte;
  // data is decoded from codebuf into framebuf
  framebuf: array[0..4000-1] of word;
  // data will be copied from framebuf into video memory

  // holds the segment selecter used to acces the video mem
  VidSel: word;
  flipflag: boolean;


{$IFDEF win32}
procedure set80x50;
var
  m: TVideoMode;
begin
  InitVideo;
  m.col := 80;
  m.row := 50;
  m.color := true;
  SetVideoMode(m);
end;
procedure set80x25; begin
  DoneVideo;
end;
procedure noblink; begin end;
procedure doblink; begin end;
PROCEDURE Flip; begin
  move(framebuf, videobuf^, 8000);
  UpdateScreen(false);
end;

procedure GetDataPtr(var Sel:word; var Offset: longint);
var
  GSSel: word;
  Ofs: longint;
begin
  asm
    mov GSSel, ds
    mov ofs, offset framebuf
  end;
  Sel := GSSel;
  Offset := ofs-64;
  asm
    mov gs, GSSel;
  end;
end;

{$ELSE}

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

procedure GetDataPtr(var Sel:word; var Offset: longint);
var
  regs: trealregs;
  GSSel: word;
begin
  realintr($FF, regs);
  writeln(regs.ax,':',regs.bx);
  GSSel := segment_to_descriptor(regs.ax);
  Sel := GSSel;
  Offset := regs.bx;
  asm
    mov gs, GSSel;
  end;
end;

{$ENDIF}

const
  CODE_MAX=$EFFF;
  CODE_RST=$FFFE;

var
  // table holding our strings, using 2 words per entry
  // with low 8 bits first word being the last char in the string
  // and with the second word being an index into this table for the rest(start) of the string
  // ncode is the next free entry in this table
  table: array[0..(CODE_MAX)-1] of cardinal;
  ncode: word;

  infile: file;

  decstart, decstop: longint;
  decspace: longint;

  readpos, readfil: longint;

  tcode, lcode: word;

  sbuf: array[0..$FF] of byte;

var
  rawfile: file;
  nread:   integer;
  prebuffer: boolean;

var
  DatSel: word;
  DatOfs: longint;
  ichar :char;


procedure wrapper;
begin
      blockread(infile, compbuf, sizeof(compbuf) div 2, readfil);
      readpos := 0;
end;
begin
  GetDataPtr(DatSel, DatOfs); // this also sets the 'gs' register to DatSel
                              // free pascal shoudn't touch gs so we can
                              // use it to access the ints data from now on

  set80x50;
  noblink;

  for ncode := 0 to 255 do begin
    table[ncode]   := $FFFF0000 or ncode;
  end;

  ncode := 256;
  tcode := $FFFF;

  if (paramcount = 1) then
    assign(infile,paramstr(1))
  else
    assign(infile,'c:\stuff\mkr.aa');

  reset(infile,2);

  asm
    mov eax, offset codebuf
    mov decstart, eax
    mov decstop, eax
  end;

  readpos := readfil;
  flipflag := true;
  prebuffer:=false;

  repeat  {    blockread(infile, framebuf, 8000, read);}
    if (readpos = readfil) then begin
      blockread(infile, compbuf, sizeof(compbuf) div 2, readfil);
      readpos := 0;
    end;
    asm
       @start:
      { if readfil = 0 then goto endofinput }
        cmp readfil, 0
        je @endofinput

      { decspace := free space in buffer }
      { used space= CODE_BUFSIZE*512*2 - decspace }
        mov eax, decstart
        sub eax, decstop
        dec eax
        jns @noadd
        add eax, CODE_BUFSIZE*512*2
       @noadd:
        mov decspace, eax

       @loopnext:
      { if (readpos >= readfil) then goto cloop2 }
        mov ebx, readpos
        cmp ebx, readfil
        jnae @cloop3

        call wrapper
        jmp @start
{        xor ebx, ebx}

       @cloop3:

      { lcode := tcode }
        mov ax, tcode
        mov lcode, ax

      { tcode := compbuf[readpos++] }
        xor eax,eax
        mov ax, [offset compbuf + 2*ebx]
        mov tcode, ax
        inc readpos

      { if (tcode = CODE_RST) then reset_and_loop_next }
        cmp ax, CODE_RST
        jne @noreset
        mov ncode, 256
        mov tcode, $FFFF
        jmp @loopnext
       @noreset:

        std
        { edi := @sbuf[$ff] }
        mov edi, offset sbuf + $FF          // edi := @sbuf[$ff]

        cmp ax, ncode
        jae @specfstring
       @lfstring1:
        { sbuf[edi--] := table[eax].char }
        { eax := table[eax].code }
        mov eax, [offset table + 4*eax]     // eax := $AAAAXXCC, CC=char, AAAA=index for rest of string
        stosb                               // sbuf[i--] = $CC(al)
        shr eax, 16                         // eax := $AAAA

        { if ax <> $FFFF then goto @l1 }
        cmp ax,$FFFF
        jne @lfstring1

        inc edi                             // edi := @s[1] ,@(last writen byte in sbuf)
        jmp @donefstring                    // goto @done
       @specfstring:
        dec edi                             // edi := @sbuf[$FF-1] (skip last char for now)
        mov ax, lcode                       // ax := lcode
       @lfstring2:
        mov eax, [offset table + 4*eax]     // same as above ;-p
        stosb
        shr eax, 16

        cmp ax,$FFFF
        jne @lfstring2

        inc edi                             // edi := @s[1] ,@(last writen byte in sbuf)
        { s[last] := s[1] }
        mov al, [edi]                       // al := last byte written in sbuf
        mov [offset sbuf + $FF], al         // sbuf[$FF] := al (write last char)
       @donefstring:
        { al := s[1] }
        mov al, [edi]
        cld



        mov ecx, offset sbuf + 1 + $FF
        sub ecx, edi
        sub decspace, ecx                   // decspace -= bytes to write

        mov esi, edi                        // esi := last byte written in sbuf, first byte to copy
        mov edi, decstop                    // edi := end of data in buffer

        { edx := space left till end of buffer }
        mov edx, offset codebuf + CODE_BUFSIZE*512*2
        sub edx, edi                        //

        { if ecx < edx then goto @go }
        cmp ecx, edx
        jb @go

        { ecx, edx := edx, ecx-edx }        // !whoa, multiple assignment!!
        sub edx, ecx
        add ecx, edx
        neg edx

        rep movsb

        sub edi, CODE_BUFSIZE*512*2        // circular buffer wrap
        mov ecx, edx                        // ecx := bytes still left to write
       @go:
        rep movsb

        mov decstop, edi                    // written stuff into buffer, update decstop

      { add new code+string to table if needed }
        { if ncode >= CODE_MAX then goto @done}
        mov cx, ncode                       // using (e)cx as it already should be 0
        cmp cx, CODE_MAX
        jae @doneadd
        { if lcode == $FFFF then goto @done}
        mov dx, lcode
        cmp dx, $FFFF
        je @doneadd

        { table[ncode].char := s[1] }
        mov [offset table + 4*ecx   ], ax
        { table[ncode].code := lcode }
        mov [offset table + 4*ecx +2], dx

        { inc ncode }
        inc ncode
       @doneadd:

       @endofinput:
      { update flipflag }
{$IFNDEF win32}
        mov ebx, DatOfs
        mov al, gs:[ebx+6]
        mov FlipFlag, al

      { if !flipflag then goto nextloop }
        cmp al, 0
        je @loopnext
{$ENDIF}

      { if descpace < CODE_BUFSIZE*512*2 - 10*1024 }
        cmp decspace, CODE_BUFSIZE*512*2 - 10*1024
        jbe @endloop

        cmp readfil, 0
        jne @loopnext
       @endloop:
    end;
    prebuffer := true;
    asm
      cld
      push es
      { es := gs }
      mov ax, DatSel
      mov gs, ax
      mov es, ax
      { eax := 0 }
      xor eax, eax
      { edi := @framebuf }
      mov edi, DatOfs
      add edi, 64
      { esi := @(start of data in buffer) }
      mov esi, decstart
    @loop:
      { if esi >= buffer_size then esi -= buffer_size }
      cmp esi, offset codebuf + CODE_BUFSIZE*512*2
      jb @nosub
      sub esi, CODE_BUFSIZE*512*2
    @nosub:
      { if esi = @(end of data in buffer) then goto @exit }
      cmp esi, decstop
      je @exit
      { ax := next control word }
      lodsw
      { if ax = $FFFF then goto @done }
      cmp ax, $FFFF
      je @done

      { edx := #words left in buffer }
      mov edx, offset codebuf + CODE_BUFSIZE*512*2
      sub edx, esi
      shr edx, 1

      { cx := #words to copy (=al) }
      xor ecx, ecx
      mov cl, al

      { if ecx <= edx goto @go }
      cmp edx, ecx
      jge @go

      { ecx := edx (=#words left in buffer) }
      mov ecx, edx
      { al -= ecx(cl) }
      sub al, cl

      rep movsw

      sub esi, CODE_BUFSIZE*512*2

      xor ecx,ecx
      mov cl,al
    @go:
      rep movsw
      {skip ah words}
      shr eax, 8
      shl eax, 1
      add edi, eax
      jmp @loop
    @exit:
    @done:
      mov decstart, esi
      pop es
    end;                {  }


    asm
      mov ebx, DatOfs
      add ebx, 20
      mov eax, decspace
      mov ecx, 10
     @nxt:
      xor edx, edx
      div ecx
      add dl, '0'
      mov dh, $0F
      mov word ptr gs:[ebx+64], dx
      dec ebx
      dec ebx
      cmp eax, 0
      jne @nxt

      mov dx, $0F20
      mov word ptr gs:[ebx+64], dx

      mov ebx, DatOfs
{$IFNDEF win32}
      mov byte ptr gs:[ebx+6], 0
    end;
{$ELSE}
    end;
    flip;
{$ENDIF}
  until (keypressed) or ((decstart = decstop) and (readfil = 0));


  readln;
  set80x25;
  doblink;
end.

