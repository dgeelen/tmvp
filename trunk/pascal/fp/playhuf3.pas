uses dos, crt




;


const
  // sizes in kilobytes
  INBUF_SIZE = 256;
  OUTBUF_SIZE = 256;

const
  // sizes in items
  REAL_INBUF_SIZE = INBUF_SIZE * 1024 div 2;
  REAL_OUTBUF_SIZE = OUTBUF_SIZE * 1024;

var
  infile: file;
  inbuf: array[0..REAL_INBUF_SIZE-1] of word;
  outbuf: array[0..REAL_OUTBUF_SIZE-1] of byte;
  incntr: longint;
  outcntr: longint;
  inread: longint;
  ineof: boolean;
  huftable: array[0..$FFFF-1] of word;

procedure refill_inbuf;
begin
  move(inbuf[incntr], inbuf[0], inread-incntr);
  incntr := inread-incntr;
  blockread(infile, inbuf[incntr], REAL_INBUF_SIZE-incntr, inread);
  inread := inread + incntr;
  incntr := 0;
end;

function read_word: word;
begin
  if (incntr < inread) then begin
    read_word := inbuf[incntr];
    inc(incntr);
  end else begin
    blockread(infile, inbuf, REAL_INBUF_SIZE, inread);
    writeln('fileread');
    incntr := 0;
    if (inread <> 0) then begin
      read_word := inbuf[incntr];
      inc(incntr);
    end else begin
      read_word := 0;
      ineof := true;
    end;
  end;
end;

procedure buildhuftable;
var
  ind: word;
  len: integer;
  inw: word;
  cnt: integer;
  i: integer;
  val: word;
  swp: word;
begin
  ind := 0;
  for len := 1 to 16 do begin
    inw := inbuf[incntr];
    inc(incntr);
    while (inw <> $FFFF) do begin
{      val := len + (inw shl 5);}
      val := len + ((inw and $100) shr (8-5)) + ((inw and $FF) shl 6);
      cnt := $8000 shr (len - 1);
      for i:=0 to cnt-1 do begin
        swp := ind + i;
        asm
          push eax; push ebx;
          mov eax, swp
          shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;
          shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;
          shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;
          shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;shr eax,1; rcl ebx,1;
          mov swp, ebx;
          pop ebx; pop eax;
        end;
        huftable[swp] := val;
      end;
      inc(ind, cnt);
      inw := inbuf[incntr];
      inc(incntr);
    end;
  end;
end;

var
  bitsleft: byte;
  hufin: longint;


function decode_huf: word;
var
  newin: longint;
  entry: word;
begin
  while (bitsleft <= 16) do begin
    newin := read_word;
    asm
      mov ebx, newin
      mov cl, bitsleft
      ror ebx, cl
      or hufin, ebx
      add bitsleft, 16
    end;
  end;

  entry := huftable[hufin and $FFFF];
  decode_huf := entry shr 4;

  asm
    mov cx, entry
    and cx, $000F
    inc cx
    mov eax, hufin
    shl ax, cl
    shr ax, cl
    rol eax, cl
    sub bitsleft, cl
    mov hufin, eax
  end;
end;


function decode_nohuf: word;
var
  newin: longint;
begin
  while (bitsleft <= 16) do begin
    newin := read_word;
    asm
      mov ebx, newin
      mov cl, bitsleft
      ror ebx, cl
      or hufin, ebx
      add bitsleft, 16
    end;
  end;

  decode_nohuf := hufin and $FFFF;

  asm
    mov eax, hufin
    xor ax, ax
    rol eax, 16
    sub bitsleft, 16
    mov hufin, eax
  end;
end;

var
  tin: word;
  tin2: word;
  lcntr: longint;
  t1,t2,t3,t4: word;

  outfile: file;
begin

  if (paramcount = 1) then
    assign(infile,paramstr(1))
  else
    assign(infile,'c:\export.huf');

  reset(infile,2);
  inread := 0;

  gettime(t1,t2,t3,t4);
  writeln('filling inbuf:',t1,'-',t2,'-',t3,'-',t4);
  refill_inbuf;
  gettime(t1,t2,t3,t4);
  writeln('building table:',t1,'-',t2,'-',t3,'-',t4);
  buildhuftable;

  hufin := inbuf[incntr];
  inc(incntr);
  bitsleft := 16;

  readln;

  repeat
    if (incntr > REAL_OUTBUF_SIZE - 10*1024) then
      refill_inbuf;

  gettime(t1,t2,t3,t4);
    writeln('start:',t1,'-',t2,'-',t3,'-',t4);

    asm
      mov edi, outcntr
      add edi, offset outbuf
      mov esi, incntr
      shl esi, 1
      add esi, offset inbuf

      mov ebx, hufin
      mov dl, bitsleft

      xor ecx, ecx // introduce system invariant, [ecx>>16 == 0], :-P
     @loop1:

      { if less than 16 valid bits left, we need some new ones }
      cmp dl, 16
      ja @nonewinp

      { read new word into eax }
      xor eax, eax
      lodsw

      { shift the new word to the left so it fits with our remaining bits }
      mov cl, dl
      shl eax, cl
      or ebx, eax
      add dl, 16

     @nonewinp:
      { do the lookup in our table }
      mov cx, bx // we assume high part of ecx is zero here
      mov ax, [offset huftable + ecx*2]

      { get number of bits that this code was long }
      mov cl, al
      and cl, $1F

      { throw away the bits we read }
      shr ebx, cl
      sub dl, cl

      { get decoded value of this code }
      shr ax, 6

      { check if it was a literal }
      jc @noliteral

{-----literal--------------------}
     @literal:

      { store literal in output }
      stosb

      cmp edi, offset outbuf + REAL_OUTBUF_SIZE
      jnae @loop1

{-----not a literal--------------------}
     @noliteral:
      { not a literal, we need the next 16 bits as offset }
      { store bx (the length) for now in mem }
      mov tin, ax

      { if less than 16 bits left, get some new ones, same as above }
      cmp dl, 16
      ja @nonewinp2

      xor eax, eax
      lodsw

      mov cl, dl
      shl eax, cl
      or ebx, eax
      add dl, 16

     @nonewinp2:
      { put the 16 needed bits(bx) into eax, and throw them away(the bits) }
      movzx eax, bx
      sub dl, 16
      shr ebx, 16

      { restore the length into cx }
      mov cx, tin

{-----lz77-copy--------------------}
//      mov bitsleft, dl { save dl cause we will corrupt it }
                       { hmm, saving in mem is just as fast as push/popping it...}
      push esi

//        and ecx, $FF
        mov esi, edi

        sub esi, eax

        cmp esi, offset outbuf
        jae @nosign
        add esi, REAL_OUTBUF_SIZE
       @nosign:

        mov eax,esi
        cmp eax,edi
        jae @higher
        mov eax,edi
       @higher:

        { eax := #words left in buffer }
        sub eax, offset outbuf + REAL_OUTBUF_SIZE
        neg eax

        { if ecx < eax then goto @go }
        cmp ecx, eax
        jb @go

        { ecx, eax := eax, ecx-eax }        // !whoa, multiple assignment!!
        sub eax, ecx
        add ecx, eax
        neg eax

        rep movsb

        mov ecx, eax
        cmp esi, offset outbuf + REAL_OUTBUF_SIZE
        jne @noesi
        mov esi, offset outbuf
       @noesi:
        cmp edi, offset outbuf + REAL_OUTBUF_SIZE
        jne @noedi
        mov edi, offset outbuf
        jmp @done
       @noedi:
        jmp @nosign
       @go:

        rep movsb

      pop esi

//      mov dl, bitsleft
{-----lz77-copy--------------------}


      cmp edi, offset outbuf + REAL_OUTBUF_SIZE
      jnae @loop1

      jmp @done
     @done:
      sub edi, offset outbuf
      mov outcntr, edi
      sub esi, offset inbuf
      shr esi, 1
      mov incntr, esi
    end;

    gettime(t1,t2,t3,t4);
    writeln('stop:',t1,'-',t2,'-',t3,'-',t4);

    assign(outfile,'c:\export.tst');
    rewrite(outfile,1);

    blockwrite(outfile, outbuf, REAL_OUTBUF_SIZE);

    close(outfile);
    halt;

  until keypressed or ineof;


  writeln('done');
end.
