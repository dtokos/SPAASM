; Porovna ci znak v AL je prazdny znak
; IN:
; 	al - Obsahuje provnavany znak
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


; Vypise retazec ukonceny s "$" na obrazovku
; IN:
; 	strSegment - adresa segmentu v ktorom sa nachadza retazec
; 	strOffset - offset na ktorom sa nachadza retazec
StrPrint proc uses ax dx ds, strSegment, strOffset
	mov ax, strSegment
	mov ds, ax
	mov dx, strOffset

	mov ah, 09h
	int 21h
	ret
StrPrint endp
