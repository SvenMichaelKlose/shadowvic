    org $1000

    ; $22 0 "Test description" 0 wanted-A wanted-X wanted-Y

    jsr clr
    $22 0 "Test starts." 0 0 0 0

    adc #1
    $22 0 "adc #1 without carry" 0 1 0 0
    sec
    adc #0
    $22 0 "adc #1 with carry" 0 2 0 0

    sec
    sbc #1
    $22 0 "sbc #1 with carry" 0 1 0 0
    clc
    sbc #1
    $22 0 "sbc #1 without carry" 0 255 0 0

    ; Load immediate
    lda #10
    ldx #11
    ldy #12
    $22 0 "load immediate #12" 0 10 11 12

    sta 0
    stx 1
    sty 2
    sta 2048
    stx 2049
    sty 2050
    lda 2
    ldx 0
    ldy 1
    $22 0 "load zeropage" 0 12 10 11

    lda #0
    ldx #0
    ldy #0
    lda 2050
    ldx 2049
    ldy 2048
    $22 0 "load absolute" 0 12 11 10

    jsr clr
    lda #$11
    and #$10
    $22 0 "AND" 0 $10 0 0

    jsr clr
    lda #$11
    ora #$44
    $22 0 "ORA" 0 $55 0 0

    jsr clr
    lda #$11
    asl
    $22 0 "ASL" 0 $22 0 0

    jsr clr
    lda #$11
    lsr
    $22 0 "LSR" 0 $08 0 0

    jsr clr
    lda #$11
    sec
    rol
    $22 0 "ROL" 0 $23 0 0
    rol
    $22 0 "ROL" 0 $46 0 0

    jsr clr
    lda #0
    sec
    ror
    $22 0 "ROR" 0 $80 0 0
    ror
    $22 0 "ROR" 0 $40 0 0

    jsr clr
    lda #$ff
    clc
    adc #2
    adc #0
    $22 0 "ADC with carry" 0 $02 0 0

    jsr clr
    lda #$1
    ldx #3
l:  sta 3,x
    dex
    bpl -l
    lda #$0
    ldx #3
l:  clc
    adc 3,x
    dex
    bpl -l
    $22 0 "zero page X" 0 4 255 0

    jsr clr
    lda #$1
    ldx #3
l:  sta 3000,x
    dex
    bpl -l
    lda #$0
    ldx #3
l:  clc
    adc 3000,x
    dex
    bpl -l
    $22 0 "absolute X" 0 4 255 0

    jsr clr
    lda #$2
    ldy #3
l:  sta 3000,y
    dey
    bpl -l
    lda #$0
    ldy #3
l:  clc
    adc 3000,y
    dey
    bpl -l
    $22 0 "absolute Y" 0 8 0 255

    jsr clr
    lda #$00
    sta 100
    lda #$40
    sta 101
    lda #$2
    ldy #3
l:  sta (100),y
    dey
    bpl -l
    lda #$00
    sta 100
    lda #$40
    sta 101
    lda #$0
    ldy #3
l:  clc
    adc (100),y
    dey
    bpl -l
    $22 0 "indirect Y" 0 8 0 255

    jsr clr
    lda #0
    beq +n
    lda #1
n:
    $22 0 "LDA BEQ" 0 0 0 0

    jsr clr
    lda #1
    bne +n
    lda #0
n:
    $22 0 "LDA BNE" 0 1 0 0

    jsr clr
    lda #20
    cmp #20
    beq +n
    lda #1
n:
    $22 0 "CMP BEQ" 0 20 0 0

    jsr clr
    lda #0
    cmp #20
    bne +n
    lda #1
n:
    $22 0 "CMP BNE" 0 0 0 0

    jsr clr
    lda #10
    cmp #20
    bcc +n
    lda #1
n:
    $22 0 "CMP BCC" 0 10 0 0

    jsr clr
    lda #10
    cmp #10
    bcs +n
    lda #1
n:
    $22 0 "CMP BCS" 0 10 0 0

    jsr clr
    lda #10
    cmp #5
    bcs +n
    lda #1
n:
    $22 0 "CMP BCS" 0 10 0 0

    jsr clr
    lda #10
    cmp #5
    bcs +n
    lda #1
n:
    $22 0 "CMP BCS" 0 10 0 0

    ; Exit emulator.
    $22 1 255

clr:clc
    clv
    cld
    lda #0
    tax
    tay
    rts
