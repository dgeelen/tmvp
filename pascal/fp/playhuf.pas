uses dos, crt




;


const
  // sizes in kilobytes
  INBUF_SIZE = 256;
  OUTBUF_SIZE = 128;

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
begin
  ind := 0;
  for len := 1 to 16 do begin
    inw := read_word;
    while (inw <> $FFFF) do begin
      val := (len - 1) + (inw shl 4);
      cnt := $8000 shr (len - 1);
      for i:=0 to cnt-1 do
        huftable[ind+i] := val;
      inc(ind, cnt);
      inw := read_word;
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

begin

  if (paramcount = 1) then
    assign(infile,paramstr(1))
  else
    assign(infile,'c:\export.huf');

  reset(infile,2);
  inread := 0;

  buildhuftable;

  bitsleft := 0;
  repeat
    tin := decode_huf;
    asm

     @sloop:
    end;
(*
    if (tin <= 255) then begin
      outbuf[outcntr] := tin and $FF;
      inc(outcntr);
      if outcntr=REAL_OUTBUF_SIZE then outcntr := 0;
    end else begin
      tin2 := decode_nohuf;
      asm
        and tin, $FF
        xor ecx,ecx
        and ecx, $FF
        mov esi, outcntr
        add esi, offset outbuf
        mov edi, outcntr
        add edi, offset outbuf
        xor edx, edx
        mov dx, tin2
        sub esi, edx
        cmp esi, offset outbuf
        jae @nosign
        add esi, REAL_OUTBUF_SIZE
       @nosign:


        mov ebx,esi
        cmp ebx,edi
        jae @higher
        mov ebx,edi
       @higher:

        { edx := #words left in buffer }
        mov edx, offset outbuf + REAL_OUTBUF_SIZE
        sub edx, ebx

        { if ecx < edx then goto @go }
        cmp ecx, edx
        jb @go

        { ecx, edx := edx, ecx-edx }        // !whoa, multiple assignment!!
        sub edx, ecx
        add ecx, edx
        neg edx

        rep stosb

        mov ecx, edx
        cmp esi, offset outbuf + REAL_OUTBUF_SIZE
        jne @noesi
        mov esi, offset outbuf
       @noesi:
        cmp edi, offset outbuf + REAL_OUTBUF_SIZE
        jne @noedi
        mov edi, offset outbuf
       @noedi:
        jmp @nosign
       @go:

        rep stosb

      end;
      inc(outcntr, tin);
      if outcntr>=REAL_OUTBUF_SIZE then dec(outcntr, REAL_OUTBUF_SIZE);
    end;
    if (lcntr div (8000*12))<>(outcntr div (8000*12)) then
      writeln('sec');

    lcntr := outcntr;
//    writeln(outcntr);

*)
    writeln($FFFF);
  until keypressed or ineof;


  writeln('done');
end.
