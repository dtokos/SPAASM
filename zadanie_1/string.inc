; Porovna ci znak v AL je prazdny znak
; IN: al = x
StrIsWhitespace proc
	cmp al, CharTab
	je StrIsWhitespaceRet
	cmp al, CharSpace
	je StrIsWhitespaceRet
	cmp al, CharCr
	je StrIsWhitespaceRet
	cmp al, CharLf
StrIsWhitespaceRet:
	ret
StrIsWhitespace endp


; Vypise retazec na obrazovku
StrPrint proc uses ax dx ds, pSegment, pOffset
	mov ax, pSegment
	mov ds, ax
	mov dx, pOffset

	mov ah, 09h
	int 21h
	ret
StrPrint endp
