Program ConPic;

Uses
  Crt;

Type
  ScreenType = Array[1..1] Of Byte;

Var
  X, Y, K : Integer;
  I : Word;
  F : File;
  F2 : Text;
  Pic : Array[0..32*20*3] Of Byte;
  BitMap : Array[1..32*20 DIV 8] Of Byte;
  B1, B2 : Byte;
  P : ^ScreenType;

Begin
  GetMem(P, 64000);
  FillChar(P^, 64000, 0);

  Assign(F, 'logo.tga');
  Reset(F, 1);
  Seek(F, 18);

  BlockRead(F, Pic, 32*20*3);
  Close(F);

  Asm
    mov   ah, 0
    mov   al, $13
    int   10h
  End;

  For Y := 0 To 19 Do
  For X := 0 To 31 Do
  Begin
    B1 := Pic[(Y*32*3) + (X*3)];
    If B1 = 255 Then B1 := 1;
    P^[((19-Y)*320)+X] := B1;
  End;

  For I := 1 To 64000 Do Mem[$A000:I] := P^[I];

  I := 0;
  K := 1;
  For Y := 0 To 19 Do
  For X := 0 To 31 Do
  Begin
    B2 := P^[(Y*320)+X];

    If I > 0 Then
    Begin
      B2 := B2 SHL I;
      B1 := B1 or B2;
      Inc(I);
    End;

    If I = 0 Then
    Begin
      B1 := B2;
      Inc(I);
    End;

    If I = 8 Then
    Begin
      I := 0;
      BitMap[K] := B1;
      Inc(K);
    End;
  End;

  Assign(F2, 'logo.txt');
  Rewrite(F2);

  For I := 1 To 80 Do Write(F2, BitMap[I]:3,', ');

  Close(F2);

  Repeat Until Keypressed;

  Asm
    mov   ah, 0
    mov   al, $03
    int   10h
  End;

  FreeMem(P, 64000);
End.