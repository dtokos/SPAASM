; Preskoci oddelovace prikazoveho riadku
; Ocakava datovy segment nasmerovany na PSP a spravne nastaveny SI
ArgsSkipDelimiters proc
	lodsb
	call ArgsIsDelimiter
	je ArgsSkipDelimiters
	ret
ArgsSkipDelimiters endp

; Overi ci znak v AL je oddelovac
; IN:
; 	al - obsahuje overovany znak
ArgsIsDelimiter proc
	cmp al, CharSpace
	je ArgsIsDelimiterRet
	cmp al, CharTab
ArgsIsDelimiterRet:
	ret
ArgsIsDelimiter endp
