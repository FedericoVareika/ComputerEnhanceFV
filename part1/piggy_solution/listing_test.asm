bits 16

; add bx, [bx+si]
; add bx, [bp]
add bh, [bp + si + 4]
add si, 2
add bp, 2
add cx, 8
