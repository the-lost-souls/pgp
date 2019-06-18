;------------------------------ MACROS --------------------------------------

;------------------------------- ASSEMBLER ROUTINE --------------------------
;Name         : VGAMode
;Type         : Macro
;Last update  : Spring '96
;Action       : Sets the VGA-mode
;Size         : 5 bytes
;
;Input variables : Mode
;
;Registers changed : ax
;----------------------------------------------------------------------------
VGAMode       MACRO VGA_Mode
                mov    ax, VGA_Mode             ;Initialize screenmode
                int    10h                      ;VGA-services
              ENDM






;Builds the sinetable
;Size = 70 bytes

BuildSine     MACRO
                mov    di, SineTable
                mov    si, SineTable + 716
                mov    bx, OFFSET Sin
                mov    cx, 90
BS_1:
                movzx  ax, BYTE PTR [ bx ]               ;Load sine-value
                movzx  dx, BYTE PTR [ bx + 1 ]
                add    dx, ax
                shr    dx, 1

                mov    [ di ], ax               ;Save at 0-90 degrees
                mov    [ di + 2 ], dx           ;/
                mov    [ di + 1440 ], ax        ;Save at 360-450 degrees
                mov    [ di + 1442 ], dx        ;/
                mov    [ si + 2 ], ax           ;Save at 180-90 degrees
                mov    [ si ], dx

                neg    ax
                neg    dx
                mov    [ di + 720 ], ax        ;Save at 180-270 degrees
                mov    [ di + 722 ], dx
                mov    [ si + 722 ], ax           ;Save at 180-90 degrees
                mov    [ si + 720 ], dx

                sub    si, 4
                add    di, 4
                inc    bx
                loop   BS_1
              ENDM



;38 bytes
InitSegs      MACRO
;---- Setting segment-addresses (26 bytes)
                mov    di, OFFSET Sin
                mov    cx, 90
                xor    al, al
                rep    stosb

                mov    ax, ds                   ;Original segment
                mov    WORD PTR Sin[ 32 ], ax   ;/
                mov    gs, ax

                mov    si, OFFSET Sin[ 34 ]
                mov    cx, 7
SegmentL:

                add    ax, 4096
                mov    [ si ], ax

                add    si, 2
                loop   SegmentL

              ENDM


;sets a 64-color greyscale palette (the first 64 colors)
;size = 16

SetPal1       MACRO
                mov   si, Palette
                mov   cx, 64                    ;Set 128 color greyscale
P1_Loop1:
                mov   [ si ], cl
                mov   [ si + 1 ], cl
                mov   [ si + 2 ], cl
                add   si, 3
                loop  P1_Loop1

                mov    si, Palette + 64*3
                call   SetPhong

                mov    es, WORD PTR Sin[ 32 ]
                mov    si, OFFSET Phong1
                mov    di, OFFSET AmbientR
                mov    cx, 10
                rep    movsb

                mov    si, Palette + 128*3
                call   SetPhong

                mov    si, OFFSET Phong2
                mov    di, OFFSET AmbientR
                mov    cx, 10
                rep    movsb

                mov    si, Palette + 3
                call   SetPhong

                mov   dx, 3C8h                  ;Port to send out startingcolor
                mov   al, 0                     ;Starting color = 0
                out   dx, al                    ;Send it!
                inc   dx                        ;Next port = RGB values

                mov   cx, 255*3
                mov   si, Palette
                rep   outsb

                mov   si, Palette
                mov   di, Palette + 768
                mov   cx, 768
                rep   movsd

               ENDM





;copies canvas (WORD PTR Sin[ 06 ] to VGAAddress

;!!!changes ds!!!
;18 bytes

ViewCanvas    MACRO
                mov    es, VGAAddress
                mov    ds, WORD PTR Sin[ 34 ]   ;Canvasaddress
                mov    cx, 64000                ;move 64k
                xor    si, si
                xor    di, di
                rep    movsb
              ENDM

;14 bytes
ClearCanvas   MACRO
                mov    es, WORD PTR Sin[ 34 ]
                xor    di, di
                xor    al, al
                mov    cx, 64000
                rep    stosb
              ENDM




InitTimer     MACRO
                mov  al,00110100b
                out  43h,al                     ;Control word
                xor  al, al                     ;Frequenzy = 1234DDh
                out  40h,al                     ;Channel 0
                out  40h,al                     ;/

                                                ;Get initial timer-value
                mov  ah,0                       ;get tick count from Dos and use For hi 3 Bytes
                int  01ah                       ;lo order count in DX, hi order in CX

                mov  al, dh
                mov  ah, cl
                shl  eax, 16
                mov  ah, dl

                mov  al,00000100b               ;Latch counter, mode 2
                out  43h,al                     ;/
                in   al,40h                     ;read in lo Byte (and discard)
                in   al,40h                     ;hi Byte into al
                neg  al                         ;make it so counting up instead of down

                mov  DWORD PTR Sin[ 50 ], eax   ;LastTime
;                mov  DWORD PTR Sin[ 54 ], 0    ;Time
             ENDM






;reads timer
;50 bytes

ReadTimer    MACRO
                mov  ah,0                  ;get tick count from Dos and use For hi 3 Bytes
                int  01ah                  ;lo order count in DX, hi order in CX

                mov  bl, dh
                mov  bh, cl
                shl  ebx, 16
                mov  bh, dl

                mov  ax,00000100b          ;Mode 2, latch counter
                out  43h,al                ;/
                in   al,40h                ;read in lo Byte (and discard)
                in   al,40h                ;hi Byte into al

                mov   cx, 0255
                movzx dx, al
                sub  cx, dx
                mov  bl, cl

                mov  edx, DWORD PTR Sin[ 50 ] ;LastTime
                mov  DWORD PTR Sin[ 50 ], ebx ;LastTime
                sub  ebx, edx

                jg   NoTimerFix
                xor  ebx, ebx
NoTimerFix:

                mov  WORD PTR Sin[ 58 ], bx  ;TimeDiff
                add  DWORD PTR Sin[ 54 ], ebx ;Time
              ENDM





Timing        MACRO
                cmp  WORD PTR Sin[ 55 ], 1234h ;Just check middle word
ActionTime = $ - 2
                jl   NoAction
                jmp  Action1
ActionJmp = $ - 1

Action1:        mov  ax, 50000/256              ;Starting dots
                mov  dl, Action2 - Action1
                mov  dh, 0001b
                jmp  DoneAction

Action2:        mov  ax, 90000/256              ;Fading in logo
                mov  dl, Action3 - Action1
                mov  dh, 0111b
                jmp  DoneAction

Action3:        mov  ax, 110000/256              ;Fading out logo
                mov  dl, Action4 - Action1
                mov  dh, 01000011b
                jmp  DoneAction

Action4:
                mov  ax, 160000/256
                mov  dl, Action5 - Action1      ;Fading in toruses
                mov  dh, 10110000b
                jmp  DoneAction

Action5:        mov  ax, 200000/256
                mov  dl, Action6 - Action1
                mov  Sin[ 09 ], 011b
                mov  dh, 10110000b

                mov  es, WORD PTR Sin[ 32 ]     ;White palette
                mov  di, Palette + 768
                mov  al, 63
                mov  cx, 255*3
                rep  stosb
                mov  es, WORD PTR Sin[ 34 ]
                jmp  DoneAction

Action6:
                mov    es, WORD PTR Sin[ 32 ]
                mov    di, Palette
                mov    cx, 256*3
                xor    al, al
                rep    stosb

                mov  ax, 230000/256
                mov  dl, Action7 - Action1
                mov  dh, 10110000b
                jmp  DoneAction

Action7:        VGAMode(03h)
                mov  ax, 4C00h
                int  21h

DoneAction:     mov  WORD PTR ActionTime, ax
                mov  BYTE PTR Actionjmp, dl
                mov  Sin[ 08 ], dh

NoAction:
              ENDM



;---------------------------------- PARTS ----------------------------------

Dotties       MACRO
                test   Sin[ 08 ], 01b
                jz     NoAction1

                mov    fs, WORD PTR Sin[ 36 ]
                mov    gs, WORD PTR Sin[ 32 ]

                mov    di, OFFSET Sin[ 70 ]
                or     Sin[ 08 ], 01000b
                mov    WORD PTR FlareColor, 64

MoreFlares:
                mov    si, OFFSET TestCoord
                call   Rotate12

                sar    cx, 1
                add    cx, 110

;---- Saving flares
                mov    si, WORD PTR Sin[ 06 ]   ;Headpointer
                add    si, NrStars*StarsPrPoint*8
                mov    bp, StarsPrPoint
AddFlare:
                mov    fs:[ si ], ax
                mov    fs:[ si + 2 ], bx
                mov    fs:[ si + 4 ], cx

                mov    WORD PTR fs:[ si + 06 ], 1234h
FlareColor = $ - 2

                add    si, 8
                dec    bp
                jnz    AddFlare

                test   Sin[ 08 ], 01000b
                jz     NoMoreFlares
                mov    di, OFFSET Sin[ 72 ]
                and    Sin[ 08 ], 011110111b
                mov    WORD PTR FlareColor, 128
                add    WORD PTR Sin[ 06 ], 8*StarsPrPoint   ;Move tail
                jmp    MoreFlares
NoMoreFlares:

                mov    es, WORD PTR Sin[ 34 ]
                mov    si, WORD PTR Sin[ 06 ]   ;Headpointer
                mov    di, NrStars*StarsPrPoint


;---- Drawing flares...
FlareLoop:
                mov    al,00000100b             ;Mode 2, latch counter
                out    43h,al                   ;/
                in     al,40h                   ;read in lo Byte (and discard)
                and    ax, 010b
                dec    ax
                sub    fs:[ si + 00 ], ax
                in     al,40h

                mov    cx, fs:[ si + 04 ]       ;Load Z

                mov    ax, fs:[ si ]
                sal    ax, 6                    ;Calculate 2D-X
                cwd
                idiv   cx
                add    ax, 160
                mov    bp, ax

                mov    ax, fs:[ si + 02 ]       ;Calculate 2D-Y
                sal    ax, 6
                cwd
                idiv   cx
                add    ax, 100
                cmp    ax, 200
                jg     NoStar

                                                ;bp = 2DX
                                                ;ax = 2DY

                mov    bx, ax                   ;Mul by 320
                shl    bx, 8                    ;/
                shl    ax, 6                    ;/
                add    bx, ax                   ;/
                add    bx, bp

                mov    cl, fs:[ si + 06 ]
                mov    es:[bx], cl

NoStar:

                mov    ax, 00000100b            ;Mode 2, latch counter
                out    43h, al                  ;/
                in     al, 40h                  ;read in lo Byte (and discard)
                and    al, 01b

                sub    fs:[ si + 02 ], ax
                inc    WORD PTR fs:[ si + 06 ]
                inc    WORD PTR fs:[ si + 02 ]
                in     al, 40h

                add    si, 8

                dec    di
                jnz    FlareLoop

                add    WORD PTR Sin[ 06 ], 8*StarsPrPoint   ;Move tail

NoAction1:
              ENDM


LogoM         MACRO
                test   Sin[ 08 ], 100b
                jz     NoAction3
                mov    ax, WORD PTR Sin[ 55 ]
                sub    ax, 49000/256
                neg    ax
                add    ax, 64
                mov    BYTE PTR LogoPixel, al
                cmp    al, 1
                ja     NoAction3
                and    Sin[ 08 ], 11111011b
NoAction3:

                test   Sin[ 08 ], 010b
                jz     NoAction2

                mov    es, WORD PTR Sin[ 34 ]   ;Canvas
                mov    di, 90*320 + 145
                mov    si, OFFSET Logo
                mov    bh, 20
LogoYLoop:
                mov    bl, 4
LogoXLoop:
                mov    dl, [ si ]
                mov    cx, 8
LogoBitLoop:
                test   dl, 1
                jz     NoLogoPixel
                mov    BYTE PTR es:[ di ], 127
LogoPixel = $ - 1
NoLogoPixel:
                inc    di

                shr    dl, 1
                loop   LogoBitLoop

                inc    si
                dec    bl
                jnz    LogoXLoop

                add    di, 320-32
                dec    bh
                jnz    LogoYLoop


NoAction2:

              ENDM




PhongMap      MACRO
                mov    es, WORD PTR Sin[ 38 ]
                xor    di, di
                mov    cx, 65535
                mov    al, 128
                rep    stosb

                mov    WORD PTR Sin[ 18 ], 120      ;Set x-coord
                mov    si, OFFSET Sin[ 18 ]         ;Offset coordinate
                mov    di, OFFSET Sin[ 10 ]         ;Offset angles
RLoop:
                mov    WORD PTR Sin[ 14 ], 359*2*2  ;Zero z-angle
RingLoop:
                call   Rotate12
                add    ax, 128
                add    bx, 128                      ;128 that is

                mov    cx, WORD PTR Sin[ 18 ]
                shr    cx, 1
                mov    bp, cx

                mov    bh, bl
                mov    bl, al
                mov    BYTE PTR es:[bx], cl

                sub    WORD PTR Sin[ 14 ], 2
                jnz    RingLoop

                dec    WORD PTR Sin[ 18 ]
                jnz    RLoop

                xor    di, di
                mov    cx, 65535
FixMap:
                cmp    BYTE PTR es:[ di ], 128
                jne    NoFixMap
                mov    al, es:[ di - 1 ]
                mov    es:[ di ], al
NoFixMap:
                inc    di
                loop   FixMap
              ENDM




segments EQU 24
sides    EQU 8
radius1  EQU 60
radius2  EQU 300
AngMove1 EQU (180*2/Sides)*2
AngMove2 EQU (360*2/Segments)*2

Vertices EQU (Segments+1)*Sides
Faces    EQU Segments*(Sides-1)*2
VertOffs EQU 0
FaceOffs EQU (Vertices*32)


BuildTorus    MACRO
                mov    fs, WORD PTR Sin[ 40 ]   ;DataSeg1, Vectorobject
                xor    di, di

                mov    bp, Segments+1
SegmentLoop:
                push   bp
                mov    cx, Sides
                mov    WORD PTR Sin[ 74 ], 180*2
SideLoop:
                push   cx
                push   di
                mov    si, OFFSET TorusCoord
                mov    di, OFFSET Sin[ 70 ]
                call   Rotate12                 ;Rotate small rings
                add    ax, Radius2
                push   cx
                push   bx
                push   ax

                mov    si, sp
                mov    di, OFFSET Sin[ 76 ]
                call   Rotate12                 ;Rotate normalvector
                add    sp, 6
                pop    di
                push   di
                mov    fs:[ di + 10 ], ax       ;Save vertice
                mov    fs:[ di + 12 ], bx
                mov    fs:[ di + 14 ], cx

                mov    si, OFFSET NVector
                mov    di, OFFSET Sin[ 70 ]
                call   Rotate12                 ;Rotate normalvector
                push   cx
                push   bx
                push   ax

                mov    si, sp
                mov    di, OFFSET Sin[ 76 ]
                call   Rotate12
                add    sp, 6
                pop    di
                mov    fs:[ di + 04 ], ax
                mov    fs:[ di + 06 ], bx
                mov    fs:[ di + 08 ], cx

                pop    cx

                add    di, 32
                add    WORD PTR Sin[ 74 ], AngMove1
                loop   SideLoop

                add    WORD PTR Sin[ 78 ], AngMove2
                cmp    WORD PTR Sin[ 78 ], 360*2*2
                jb     BuildAngOK
                mov    WORD PTR Sin[ 78 ], 0
BuildAngOK:
                pop    bp
                dec    bp
                jnz    SegmentLoop



;---- Building faces...

                mov    bp, Segments
                xor    bx, bx
                mov    dx, 32
FaceSegLoop:
                mov    cx, Sides-1
FaceSidLoop:
                mov    ax, bx
                add    ax, Sides*32
                mov    si, dx
                add    si, Sides*32

                mov    fs:[ di + 00 ], dx          ;V1
                mov    fs:[ di + 02 ], bx          ;V2
                mov    fs:[ di + 04 ], ax          ;V3

                mov    fs:[ di + 06 ], ax          ;V1
                mov    fs:[ di + 08 ], dx          ;V2
                mov    fs:[ di + 10 ], si          ;V3

                add    bx, 32
                add    dx, 32
                add    di, 12
                loop   FaceSidLoop

                add    bx, 32
                add    dx, 32

                dec    bp
                jnz    FaceSegLoop
              ENDM



Vector        MACRO
                test   Sin[ 08 ], 010000b
                jz     DoneVectors

                mov    ax, WORD PTR Sin[ 55 ]
                sub    ax, 110000/256
                mov    dx, 3
                imul   dx
                mov    dx, 400
                sub    dx, ax
                cmp    dx, 0
                jle    NoVectorMove
                mov    VectorY, dx
NoVectorMove:


                mov    BYTE PTR PixelAdd, 64
                mov    di, OFFSET Sin[ 70 ]
                or     Sin[ 08 ], 1000b

VectorLoop:
                mov    gs, WORD PTR Sin[ 40 ]
                mov    es, WORD PTR Sin[ 34 ]

                mov    cx, Vertices
                mov    si, 4                    ;Normalvector
RotateVerts:
                push   cx

                call   Rotate12
                add    ax, 128
                add    bx, 128
                shl    ax, 8
                shl    bx, 8
                mov    gs:[ si - 4 ], ax
                mov    gs:[ si - 2 ], bx

                add    si, 6
                call   Rotate12

;                add    ax, ...                  ;Add position
                add    bx, VectorY
;                add    cx, ...

                mov    bp, cx
                add    bp, 900

                cwd
                shld   dx, ax, 8
                shl    ax, 8
                idiv   bp
                add    ax, 160
                shl    ax, 6
                mov    gs:[ si + 08 ], ax

                mov    ax, bx
                cwd
                shld   dx, ax, 8
                shl    ax, 8

                idiv   bp
                add    ax, 100
                mov    gs:[ si + 10 ], ax

                shl    cx, 6
                mov    gs:[ si + 06 ], cx

                add    si, 26
                pop    cx
                loop   RotateVerts




                mov    cx, Faces
                mov    si, FaceOffs
FaceLoop:
                mov    fs, WORD PTR Sin[ 40 ]
                push   cx
                push   si

                call   PhongTri

                pop    si
                add    si, 6
                pop    cx
                loop   FaceLoop

                test   Sin[ 08 ], 1000b
                jz     DoneVectors
                mov    di, OFFSET Sin[ 72 ]
                mov    BYTE PTR PixelAdd, 128
                and    Sin[ 08 ], 011110111b    ;Zero flag
                jmp    VectorLoop
DoneVectors:

              ENDM


CheckerY        EQU 150 *256
CheckerSlope    EQU 400

InitCheckers  MACRO
                mov    di, CheckerZ
                mov    cx, 30

ICheckers:
                mov    ax, CheckerY
                xor    dx, dx
                idiv   cx
                mov    [ di ], ax           ;Save 3D-Z/V

                add    di, 2
                inc    cx
                cmp    cx, 100
                jb     ICheckers

              ENDM




Checkers      MACRO
                test   Sin[ 08 ], 0100000b
                jz     NoAction5

                mov    fs, WORD PTR Sin[ 38 ]

                mov    si, CheckerZ
                mov    cx, 70
                mov    di, 130*320

                mov    bx, WORD PTR Sin[ 70 ]
                mov    ax, SineTable[ bx + 90*2*2 ]
                shl    ax, 9
                mov    VCoord, ax

                mov    ax, SineTable[ bx ]
                shl    ax, 9
                mov    UCoord, ax
                mov    gs, WORD PTR Sin[ 42 ]   ;Z-buffer 1

CheckerL:
                push   cx
                mov    bp, [ si ]
                mov    ax, CheckerSlope
                imul   bp
                shrd   ax, dx, 8
                mov    CSlopeU, ax

                mov    dx, [ si ]
                shl    dx, 8
                add    dx, VCoord

                mov    ax, -160
                imul   ax, CSlopeU
                add    ax, UCoord

                sub    bp, 220h
                sar    bp, 4

                mov    cx, 320

                mov    cx, 320
CheckerHL:
                mov    bh, dh
                mov    bl, ah
                movzx  bx, BYTE PTR fs:[ bx ]
                shr    bx, 1
                add    bx, 11

                add    bx, bp
                cmp    bx, 60
                jbe    ColOK
                mov    bx, 60
ColOK:
                mov    es:[ di ], bl

                add    ax, CSlopeU           ;Move U

                inc    di
                loop   CheckerHL


;---- Setting Z-buffer
                mov    ax, [ si ]            ;Load z-value

                sub    ax, 600h /2
                sal    ax, 5

                push   si

                mov    si, di
                shl    si, 1
                cmp    di, 32000
                jb     FirstZB
                sub    si, 32000
                mov    gs, WORD PTR Sin[ 44 ]
FirstZB:

                mov    cx, 320
ZLoop:
                mov    gs:[ si ], ax
                add    si, 2
                dec    cx
                jnz    ZLoop
                pop    si



                pop    cx
                add    si, 2
                dec    cx
                jnz    CheckerL

NoAction5:


              ENDM




FadeOut       MACRO
                test   Sin[ 08 ], 01000000b
                jz     NoAction6

                mov    si, Palette + 768
                mov    cx, 768
FadeOutL:
                cmp    BYTE PTR [ si ], 0
                jz     NoStop
                dec    BYTE PTR [ si ]
NoStop:
                inc    si
                loop   FadeOutL


                mov   dx, 3C8h                  ;Port to send out startingcolor
                mov   al, 1                     ;Starting color = 0
                out   dx, al                    ;Send it!
                inc   dx                        ;Next port = RGB values

                mov   cx, 254*3
                mov   si, Palette + 768 + 3
                rep   outsb

NoAction6:



              ENDM


Fadein        MACRO
                test   Sin[ 08 ], 10000000b
                jz     NoAction7

                mov    si, Palette
                mov    cx, 768
FadeInL:
                mov    al, [ si ]
                cmp    BYTE PTR [ si + 768 ], al
                jb     FadeUp
                je     DoneFade
                nop
                nop
                nop
                nop
                dec    BYTE PTR [ si + 768 ]
                jmp    DoneFade
FadeUp:
                inc    BYTE PTR [ si + 768 ]
DoneFade:
                inc    si
                loop   FadeInL

                mov    dx, 3C8h                  ;Port to send out startingcolor
                mov    al, 1                     ;Starting color = 0
                out    dx, al                    ;Send it!
                inc    dx                        ;Next port = RGB values

                mov    cx, 254*3
                mov    si, Palette + 768 + 3
                rep    outsb

NoAction7:

              ENDM

InitGlass     MACRO
                mov    es, WORD PTR Sin[ 46 ]

                mov    bp, 200
                xor    di, di
SinYLoop1:
                mov    cx, 320
                xor    si, si
                mov    SinAdd, 1600
SinXLoop1:
                mov    ax, SineTable[ si ]
                sar    ax, 4                   ;!!!!!!!!!!!!!!!
                add    ax, 256                  ;!!!!!!!!!!!!!!!
                stosb
                mov    ax, SinAdd
                shr    ax, 4
                and    ax, -2
                add    si, ax
                cmp    si, 360*2*2
                jb     NoSinFix1
                sub    si, 360*2*2
NoSinFix1:
                sub    SinAdd, 4

                loop   SinXLoop1

                dec    bp
                jnz    SinYLoop1


              ENDM

Glass         MACRO
                test   Sin[ 09 ], 00000010b
                jz     NoGlass

                mov    fs, WORD PTR Sin[ 46 ]    ;Glass-map
                mov    gs, VGAAddress
                mov    es, WORD PTR Sin[ 34 ]

;----

                mov    cx, 198
                mov    di, 320
GYLoop:

                mov    bp, 320
GXLoop:
                mov    al, fs:[ di +   1 ]       ;X + 1
                mov    ah, fs:[ di -   1 ]       ;X - 1
                mov    dl, fs:[ di + 320 ]       ;Y + 1
                mov    dh, fs:[ di - 320 ]       ;Y - 1

                sub    dh, dl
                sub    ah, al

                movsx  si, dh
                mov    bx, si
                shl    si, 8
                shl    bx, 6
                add    si, bx
                movsx  bx, ah
                add    si, bx
                add    si, di

                mov    bl, es:[ si ]             ;Load pixel
                mov    gs:[ di ], bl             ;Set pixel to VGA

                inc    di
                dec    bp
                jnz    GXLoop

                loop   GYLoop

NoGlass:


              ENDM




