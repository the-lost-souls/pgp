.Model Tiny


include 4k.inc
include 4kdata.inc


NrStars       EQU 128
StarsPrPoint  EQU 8

NrFrames      EQU 100
StarsPrFrame  EQU 10
;------------------------------ MAIN PROCEDURE ------------------------------
.CODE
.STARTUP
.386
Main          PROC
                BuildSine                       ;Setup sinetable (70 bytes)
                InitSegs                        ;Set the segment-addresses(38)
                VGAMode(13h)                    ;Set mode 13h (5 bytes)
                SetPal1                         ;Set the first palette

                InitTimer                       ;Initialize timer
                PhongMap
                BuildTorus
                InitCheckers
                InitGlass

                mov    WORD PTR ActionTime, 0000/256
;                mov    Sin[ 08 ], 00110001b
;                mov    Sin[ 09 ], 00000011b


                mov    es, WORD PTR Sin[ 36 ]
                xor    di, di
                mov    cx, NrStars*StarsPrPoint*8
                mov    al, 01h
                rep    stosb



;---- Main loop

MainLoop:

                mov    es, WORD PTR Sin[ 42 ]
                xor    di, di
                mov    cx, 65535/2
                mov    ax, 127*256
                rep    stosw

                mov    es, WORD PTR Sin[ 44 ]
                xor    di, di
                mov    cx, 65535/2
                mov    ax, 127*256
                rep    stosw


                ReadTimer
                ClearCanvas
                Checkers
                FadeOut
                FadeIn


;---- common used angles...
                mov    ax, WORD PTR Sin[ 58 ]
                shr    ax, 7
                or     ax, 1
                shl    ax, 1
                mov    bx, 4
                imul   bx, ax
                mov    cx, 7
                imul   cx, ax
                add    WORD PTR Sin[ 70 ], bx      ;ZAngle
                cmp    WORD PTR Sin[ 70 ], 360*2*2
                jb     AngYOK
                mov    WORD PTR Sin[ 70 ], 0
AngYOK:
                add    WORD PTR Sin[ 72 ], ax      ;ZAngle
                cmp    WORD PTR Sin[ 72 ], 360*2*2
                jb     AngXOK
                mov    WORD PTR Sin[ 72 ], 0
AngXOK:
                add    WORD PTR Sin[ 74 ], cx      ;ZAngle
                cmp    WORD PTR Sin[ 74 ], 360*2*2
                jb     AngZOK
                mov    WORD PTR Sin[ 74 ], 0
AngZOK:


                Timing

                Dotties                         ;The dots at the beginning
                LogoM                           ;View the logo
                Vector
                Glass


                test   Sin[ 09 ], 00000001b
                jnz    NoCanvas

                push   ds
                ViewCanvas
                pop    ds
NoCanvas:

                mov    ah, 0Bh                  ;
                int    21h                      ;Has somebody pressed a key?
                or     al, al
                jz     MainLoop

;---- Shutting down

                VGAMode(03h)                    ;Set mode 3h (5 bytes)
                mov    ah, 4Ch                  ;Halt program
                int    21h                      ;/
Main          ENDP










;gs:[ si ] = Offset to coordinate
;[ di ] = Offset to angles
;
;ax = X  bx = Y  cx = Z
;
;Registers changed : ax, bx, cx, dx, bp
;Size = 146 ( +3 with call)

Rotate12      PROC
;---- Rotating X-axis
                mov    bp, [ di + 00 ]          ;Load XAngle

                mov    bx, SineTable[ bp ]            ;sin(XAngle)
                mov    cx, SineTable[ bp + (90*2*2) ] ;cos(XAngle)

                mov    ax, gs:[ si + 02 ]
                imul   cx                       ;y*cos(XAngle)
                shrd   ax, dx, 8
                mov    bp, ax                   ;Save result

                mov    ax, gs:[ si + 04 ]
                imul   bx                       ;z*sin(XAngle)
                shrd   ax, dx, 8
                sub    bp, ax                   ;ebp = newY

                mov    ax, gs:[ si + 02 ]
                imul   bx                       ;y*sin(XAngle)
                shrd   ax, dx, 8
                mov    bx, ax

                mov    ax, gs:[ si + 04 ]
                imul   cx                       ;z*cos(XAngle)
                shrd   ax, dx, 8
                add    bx, ax                   ;cx = newZ

                mov    WORD PTR Sin[ 02 ], bp   ;Save temp-y
                mov    WORD PTR Sin[ 04 ], bx   ;Save temp-z



;---- Rotating Y-axis
                mov    bp, [ di + 02 ]          ;Load YAngle

                mov    bx, SineTable[ bp ]            ;sin(YAngle)
                mov    cx, SineTable[ bp + (90*2*2) ] ;cos(YAngle)

                mov    ax, WORD PTR Sin[ 04 ]
                imul   cx                       ;z*cos(YAngle)
                shrd   ax, dx, 8
                mov    bp, ax                   ;Save result

                mov    ax, gs:[ si + 00 ]
                imul   bx                       ;x*sin(YAngle)
                shrd   ax, dx, 8
                sub    bp, ax                   ;bp = Final Z

                mov    ax, WORD PTR Sin[ 04 ]
                imul   bx                       ;z*sin(YAngle)
                shrd   ax, dx, 8
                mov    bx, ax

                mov    ax, gs:[ si + 00 ]
                imul   cx                       ;x*cos(YAngle)
                shrd   ax, dx, 8
                add    bx, ax                   ;bx = newX

                mov    WORD PTR Sin[ 04 ], bp   ;Save final-z
                mov    WORD PTR Sin[ 00 ], bx   ;Save temp-x



;---- Rotating Z-axis
                mov    bp, [ di + 04 ]          ;Load ZAngle

                mov    bx, SineTable[ bp ]  ;sin(ZAngle)
                mov    cx, SineTable[ bp + (90*2*2) ] ;cos(ZAngle)

                mov    ax, WORD PTR Sin[ 00 ]
                imul   cx                       ;x*cos(ZAngle)
                shrd   ax, dx, 8
                mov    bp, ax                   ;Save result

                mov    ax, WORD PTR Sin[ 02 ]
                imul   bx                       ;y*sin(ZAngle)
                shrd   ax, dx, 8
                sub    bp, ax                   ;bp = Final X

                mov    ax, WORD PTR Sin[ 00 ]
                imul   bx                       ;x*sin(ZAngle)
                shrd   ax, dx, 8
                mov    bx, ax

                mov    ax, WORD PTR Sin[ 02 ]
                imul   cx                       ;y*cos(ZAngle)
                shrd   ax, dx, 8
                add    bx, ax                   ;bx = Final Y

                mov    ax, bp                   ;eax = Final X
                                                ;ebx = Final Y
                mov    cx, WORD PTR Sin[ 04 ]   ;ecx = Final Z

                ret
Rotate12      ENDP



;fs:[ si ] = offset structure
;fs (possibly gs) is changed


;00 = Ptr1
;02 = Ptr2
;04 = Ptr3
;total 6 bytes

;ds = mainseg
;map = dataseg2
;zbuffer 1 = dataseg3
;zbuffer 2 = dataseg4

SlopeZ        DW 0
SlopeX        DW 0
SlopeU        DW 0
SlopeV        DW 0

PhongTri      PROC
                mov    bx, fs:[ si ]            ;Load Ptr1
                mov    di, fs:[ si + 2 ]        ;Load Ptr2
                mov    bp, fs:[ si + 4 ]        ;Load Ptr3
                mov    ax, fs:[ bx + 20 ]       ;Y1
                mov    dx, fs:[ di + 20 ]       ;Y2
                mov    cx, fs:[ bp + 20 ]       ;Y3

                cmp    ax, dx                   ;Y1, Y2
                jl     NoFlip1
                xchg   ax, dx
                xchg   bx, di
NoFlip1:
                cmp    ax, cx                   ;Y1, Y3
                jl     NoFlip2
                xchg   ax, cx
                xchg   bx, bp
NoFlip2:
                cmp    dx, cx                   ;Y2, Y3
                jl     NoFlip3
                xchg   dx, cx
                xchg   di, bp
NoFlip3:

                cmp    cx, 200
                jge    DumpPoly
                cmp    ax, 0
                jle    DumpPoly

                mov    fs:[ si ], bx            ;Save pointers
                mov    fs:[ si + 2 ], di
                mov    fs:[ si + 4 ], bp

;---- First line (1-3)
                sub    cx, fs:[ bx + 20 ]       ;DeltaY1
                jz     DumpPoly

                push   cx

                mov    ax, fs:[ bp + 18 ]       ;X3
                sub    ax, fs:[ bx + 18 ]       ;X1
                cwd
                idiv   cx
                push   ax

                mov    ax, fs:[ bp + 16 ]       ;Z3
                sub    ax, fs:[ bx + 16 ]       ;-Z1 = DeltaZ1
                cwd
                idiv   cx
                push   ax

                mov    ax, fs:[ bp + 02 ]       ;V3
                sub    ax, fs:[ bx + 02 ]       ;-V1 = DeltaV1
                cwd
                idiv   cx
                push   ax


                mov    ax, fs:[ bp + 00 ]       ;U3
                sub    ax, fs:[ bx + 00 ]       ;-U1 = DeltaU1
                cwd
                idiv   cx
                push   ax


                pop    ebp
                pop    edi

                mov    eax, DWORD PTR fs:[ bx + 00 ] ;U1/V1
                mov    edx, DWORD PTR fs:[ bx + 16 ] ;Z1/X1

                mov    bx, OFFSET LRBuffer

;di = counterX
;bp = counterZ
;ax = counterU
;dx = counterV
;cx = loopcounter
;bx = mempointer

LLoop1:
                mov    [ bx ], edx               ;Save X
                mov    [ bx + 04 ], eax          ;Save U

                add    edx, edi
                add    eax, ebp

                add    bx, 16
                loop   LLoop1


;---- Line 2
                mov    bx, OFFSET LRBuffer + 8

                mov    di, fs:[ si + 00 ]
                mov    bp, fs:[ si + 02 ]
                mov    cx, fs:[ bp + 20 ]       ;Y2
                sub    cx, fs:[ di + 20 ]       ;-Y1 = DeltaY2
                jz     DumpL2

                mov    ax, fs:[ bp + 18 ]       ;X2
                sub    ax, fs:[ di + 18 ]       ;-X1 = DeltaX2
                cwd
                idiv   cx
                push   ax

                mov    ax, fs:[ bp + 16 ]       ;Z2
                sub    ax, fs:[ di + 16 ]       ;-Z1 = DeltaZ2
                cwd
                idiv   cx
                push   ax

                mov    ax, fs:[ bp + 02 ]       ;V2
                sub    ax, fs:[ di + 02 ]       ;-V1 = DeltaV2
                cwd
                idiv   cx
                push   ax


                mov    ax, fs:[ bp + 00 ]       ;U2
                sub    ax, fs:[ di + 00 ]       ;-U1 = DeltaU2
                cwd
                idiv   cx
                push   ax


                mov    eax, DWORD PTR fs:[ di + 00 ] ;U1/V1
                mov    edx, DWORD PTR fs:[ di + 16 ] ;Z1/X1

                pop    ebp
                pop    edi


;di = counterX
;bp = counterZ
;ax = counterU
;dx = counterV
;cx = loopcounter
;bx = mempointer

LLoop2:
                mov    [ bx ], edx              ;Save X
                mov    [ bx + 04 ], eax         ;Save U

                add    edx, edi
                add    eax, ebp

                add    bx, 16
                loop   LLoop2

DumpL2:


;---- Line 3
                mov    di, fs:[ si + 02 ]
                mov    bp, fs:[ si + 04 ]
                mov    cx, fs:[ bp + 20 ]       ;Y3
                sub    cx, fs:[ di + 20 ]       ;-Y2 = DeltaY3
                jz     DumpL3

                mov    ax, fs:[ bp + 18 ]       ;X3
                sub    ax, fs:[ di + 18 ]       ;- X2 = DeltaX3
                cwd
                idiv   cx
                push   ax

                mov    ax, fs:[ bp + 16 ]       ;Z3
                sub    ax, fs:[ di + 16 ]       ;-Z2 = DeltaZ3
                cwd
                idiv   cx
                push   ax

                mov    ax, fs:[ bp + 02 ]       ;V3
                sub    ax, fs:[ di + 02 ]       ;-V2 = DeltaV3
                cwd
                idiv   cx
                push   ax


                mov    ax, fs:[ bp + 00 ]       ;U3
                sub    ax, fs:[ di + 00 ]       ;-U2 = DeltaU3
                cwd
                idiv   cx
                push   ax


                mov    eax, DWORD PTR fs:[ di + 00 ] ;U1/V1
                mov    edx, DWORD PTR fs:[ di + 16 ] ;Z1/X1

                pop    ebp
                pop    edi

;di = counterX
;bp = counterZ
;ax = counterU
;dx = counterV
;cx = loopcounter
;bx = mempointer

LLoop3:
                mov    [ bx ], edx              ;Save X
                mov    [ bx + 04 ], eax         ;Save U

                add    edx, edi
                add    eax, ebp

                add    bx, 16
                loop   LLoop3

DumpL3:

;---------------------------------------------------------------------------

                pop    cx                       ;Loopcounter
                mov    di, fs:[ si ]               ;Load Y1
                mov    di, fs:[ di + 20 ]          ;/
                mov    ax, di                   ;Calculate mem-address
                shl    di, 8
                shl    ax, 6
                add    di, ax

                mov    bp, OFFSET LRBuffer
                mov    si, OFFSET LRBuffer + 8

                mov    fs, WORD PTR Sin[ 38 ]   ;PhongMap
                mov    gs, WORD PTR Sin[ 42 ]   ;Z-buffer 1


DrawLoop:
                push   cx
                push   di
                push   si
                push   bp

                mov    cx, [ bp + 02 ]          ;XRight
                mov    bx, [ si + 02 ]          ;XLeft
                shr    cx, 6
                shr    bx, 6
                sub    cx, bx
                jz     DumpLine                 ;Dump?
                jg     NoSideFlip               ;Eventually flip...
                neg    cx
                xchg   bp, si
                sub    bx, cx                   ;Flip side
NoSideFlip:
                add    di, bx

                mov    ax, [ bp ]               ;Zslope
                sub    ax, [ si ]
                cwd
                idiv   cx
                mov    SlopeZ, ax

                mov    ax, [ bp + 04 ]          ;USlope
                sub    ax, [ si + 04 ]
                cwd
                idiv   cx
                mov    SlopeU, ax

                mov    ax, [ bp + 06 ]          ;VSlope
                sub    ax, [ si + 06 ]
                cwd
                idiv   cx
                mov    SlopeV, ax

                mov    bp, [ si ]
                mov    ax, [ si + 04 ]
                mov    dx, [ si + 06 ]

                mov    si, di
                shl    si, 1
                cmp    di, 32000
                jb     FirstZBuffer
                sub    si, 32000
                mov    gs, WORD PTR Sin[ 44 ]

FirstZBuffer:

;ax = Ucounter
;bx = mempointer
;cx = loopcounter
;dx = Vcounter
;bp = zcounter

ScanL:
                cmp    bp, gs:[ si ]
                jg     NoPixel

                mov    gs:[ si ], bp
                mov    bx, dx
                mov    bl, ah
                mov    bl, fs:[ bx ]
                add    bl, 64
PixelAdd = $ - 1
                mov    es:[ di ], bl

NoPixel:

                add    ax, SlopeU
                add    dx, SlopeV
                add    bp, SlopeZ
                inc    di
                add    si, 2
                loop   ScanL

DumpLine:
                pop    bp
                pop    si
                pop    di
                pop    cx

                add    di, 320
                add    bp, 16
                add    si, 16

                dec    cx
                jnz    DrawLoop
DumpPoly:


                ret
PhongTri      ENDP

AmbientR        DB 12
AmbientG        DB 12
AmbientB        DB 12

DiffuseR        DB 20
DiffuseG        DB 0
DiffuseB        DB 30

SpecR           DB 30
SpecG           DB 48
SpecB           DB 20

Shiny           DB 40

Phong1          DB 15
                DB 8
                DB 8
                DB 20
                DB 0
                DB 0
                DB 25
                DB 55
                DB 55
                DB 30

Phong2          DB 0
                DB 0
                DB 0
                DB 25
                DB 20
                DB 25
                DB 33
                DB 38
                DB 38
                DB 15

Phong3          DB 0
                DB 0
                DB 0
                DB 0
                DB 20
                DB 15
                DB 30
                DB 38
                DB 30
                DB 15

Phong4          DB 0
                DB 0
                DB 0
                DB 35
                DB 20
                DB 20
                DB 28
                DB 28
                DB 20
                DB 15


Phong5          DB 0
                DB 0
                DB 0
                DB 25
                DB 15
                DB 15
                DB 38
                DB 18
                DB 28
                DB 15

SetPhong      PROC
                xor   bp, bp
PhCL:
                mov   ax, bp
                mov   bx, 720
                imul  bx, ax
                shr   bx, 8
                shl   bx, 1                     ;words...
                mov   ax, SineTable[ (90*4) + bx]
                mov   bx, ax                    ;bx = Cos(Angle)

                movzx cx, Shiny
                cwd                             ;ax -> ax:dx
ShinyL:
                imul  bx
                shrd  ax, dx, 8
                loop  ShinyL
                mov   di, ax                    ;di = cos(Angle)^Shiny * 256

;----- Red
                movzx ax, DiffuseR
                imul  bx                        ;DiffuseR * Cos(Angle)
                shrd  ax, dx, 8
                mov   cx, ax                    ;cx = Diffuse-part
                movzx ax, SpecR
                imul  di                        ;SpecR * cos(Angle)^Shiny
                shr   ax, 8                     ;/256
                add   ax, cx
                add   al, AmbientR
                mov   [si], al

;----- Green
                movzx ax, DiffuseG
                imul  bx                        ;DiffuseG * Cos(Angle)
                shrd  ax, dx, 8
                mov   cx, ax                    ;cx = Diffuse-part
                movzx ax, SpecG
                imul  di                        ;SpecG * cos(Angle)^Shiny
                shr   ax, 8                     ;/256
                add   ax, cx
                add   al, AmbientG
                mov   [si + 1], al

;----- Blue
                movzx ax, DiffuseB
                imul  bx                        ;DiffuseB * Cos(Angle)
                shrd  ax, dx, 8
                mov   cx, ax                    ;cx = Diffuse-part
                movzx ax, SpecB
                imul  di                        ;SpecB * cos(Angle)^Shiny
                shr   ax, 8                     ;/256
                add   ax, cx
                add   al, AmbientB
                mov   [si + 2], al

                add   si, 3
                inc   bp
                cmp   bp, 64
                jb    PhCL

                ret
SetPhong      ENDP

END