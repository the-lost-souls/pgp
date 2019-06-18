Program BSine;

Uses
  Crt;

Var
  I : Integer;
  B : Byte;
  F : Text;

Begin
  Assign(F, '4ksine.bin');
  Rewrite(F);


  For I := 0 To 89 Do
  Begin
    B := Round(Sin((I/360)*2*Pi)*255);
    Write(F, B:3,', ');
  End;

  Close(F);
End.