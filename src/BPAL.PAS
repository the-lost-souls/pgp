Program BSine;

Uses
  Crt;

Const
  NrStars = 150;

Var
  I, C : Integer;
  B : Byte;
  F : Text;
  R1, R2 : Real;
  Rnd : Array[1..NrStars] of Byte;

Begin
  Randomize;

  R1 := 0;
  R2 := 63/NrStars;
  For I := 1 To NrStars Do
  Begin
    Rnd[I] := Round(R1);
    R1 := R1 + R2;
  End;


  For I := 1 To (NrStars DIV 10) Do
  Begin
    Rnd[I*10] := Random(20);
  End;



  Assign(F, 'rnd.txt');
  Rewrite(F);
  Randomize;

  For I := 1 To NrStars Do Write(F, Rnd[I]:3,', ');

  Close(F);
End.