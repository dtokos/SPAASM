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
