ArgsParse proc paramSegment, statusOffset, fileNameOffset, helpOffset
	;local HasFileName:byte
	;mov HasFileName, 00h
	mov es, paramSegment
	mov si, 81h

ArgsParseLoop:
	call ArgsSkipDelimiters

; Zisti ci to je nazov suboru alebo prepinac
	cmp al, '/'
	jnz AgrsParseFileName

; Over ci to je /h
	lodsb
	cmp al, 'h'
	jnz ArgsInvalidSwitch
	lodsb ; Nasledujuci znak musi byt prazdny aby to bol platny prepinac
	cmp al, CharCr ; Sme na konci argumentov?
	jz ArgsShowHelp

AgrsParseFileName:
	cmp al, CharCr ; Sme na konci argumentov?
	je ArgsParseSuccess
	mov di, fileNameOffset

AgrsParseFileNameLoop:
	stosb
	lodsb
	cmp al, CharCr ; Sme na konci argumentov?
	je ArgsFinishFileName
	call ArgsIsDelimiter
	jne AgrsParseFileNameLoop

ArgsFinishFileName:
	mov al, 00h
	stosb
	dec si
	jmp ArgsParseLoop

ArgsInvalidSwitch:
	mov di, statusOffset
	mov byte ptr es:[di], 01h
	jmp ArgsParseRet

ArgsShowHelp:
	mov di, helpOffset
	mov byte ptr es:[di], 01h
	jmp ArgsParseSuccess

ArgsParseSuccess:
	mov di, statusOffset
	mov byte ptr es:[di], 00h
	jmp ArgsParseRet

ArgsParseRet:
	ret
ArgsParse endp

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
