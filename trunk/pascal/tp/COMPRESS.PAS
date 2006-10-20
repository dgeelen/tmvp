{$M 8000 65536 655360}

uses crtclone;

const CODE_MAX=$1FF;

type
  tce = ^code_entry;
  code_entry = record
    code: word;
    next: array[char] of tce;
  end;

{  code_table = array[0..CODE_MAX-1] of code_entry;}
  code_table = array[0..CODE_MAX-1] of ^string;

var
   infile: file of char;
   outfile: file of word;
   ncodes: word;
   is: string;
   ic: char;
   ib: byte;
   table: tce;
   ow: word;
   lr,nr: word;
   ci,co: longint;
   x:char;
   cnt: integer;

function find_string(s: string): word;
var
  i,j: word;
  node: tce;
begin
  node := table;
  find_string := $FFFF;
  for i:=1 to length(s) do begin
    node := node^.next[s[i]];
    if node = nil then
      exit;
  end;
  find_string := node^.code;
{  for i:=0 to ncodes-1 do begin
    if table[i]^ = s then begin
       find_string := i;
       exit;
    end;
  end;
  find_string := $FFFF;}
end;

procedure add_string(s: string);
var
 i,j: word;
 node: tce;
 len: byte;

begin
  if ncodes = CODE_MAX then
    exit;

  len := length(s);

  node := table;
  for i:= 1 to len-1 do
    node := node^.next[s[i]];

  new(node^.next[s[len]]);
  node := node^.next[s[len]];

  for i:= 0 to 255 do
    node^.next[char(i)] := nil;

  node^.code := ncodes;

  inc(ncodes);
end;

begin
  assign(infile,'c:\stuff\b800.aaa');
  reset(infile);

  assign(outfile,'c:\stuff\b800.lzw');
  rewrite(outfile);

  new(table);
  ncodes := 0;
  for cnt := 0 to 255 do begin
    table^.next[char(cnt)] := nil;
    add_string(char(cnt));
  end;

  ncodes := 256;

  read(infile, ic);
  is := ic;

  while(not eof(infile)) do begin
    read(infile, ic);
    inc(ci);
    if (find_string(is + ic) <> $FFFF) then begin
      is := is + ic;
    end else begin
      ow := find_string(is);
      write(outfile, ow);
      inc(co);
      add_string(is + ic);
      is := ic;
    end;
    if keypressed then begin
      writeln('codes:',ncodes,' ci:',ci,' co:',co,' r:',(co*200) div (ci),'   is:',is,'  ic:',ic);
      if readkey=chr(27) then halt;
    end;
  end;
  ow := find_string(is);
  write(outfile, ow);
  close(infile);
  close(outfile);
end.