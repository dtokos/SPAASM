; Sparsuje command line argumenty
; Ocakava ze DS smeruje na PSP
; IN:
; 	paramSegment - adresa segmentu v ktorom su uchovane vystupne premenne
; 	statusOffset - offset premennej do ktorej sa ulozi vysledok parsovania (0 = OK, 1 = Neplatny prepinac, 2 = Ziadny nazov suboru, 3 = Viacero nazvov suboru)
; 	fileNameOffset - offset premennej do ktorej sa ulozi nazov suboru ukonceny nulou
; 	helpOffset - offset premennej do ktorej sa ulozi ci na vstupe bol prepinac /h
ArgsParse proc paramSegment, statusOffset, fileNameOffset, helpOffset
	; Nastavime lokalne premenne, segmenty a SI na citanie stringu
	local HasFileName:byte
	mov HasFileName, 00h
	mov es, paramSegment
	mov si, 81h

; Hlavny cyklus parsovania
; Zistime ci sa jedna o nazov suboru alebo prepinac
ArgsParseLoop:
	call ArgsSkipDelimiters
	cmp al, '/'
	jne AgrsParseFileName

; Vyzera to na prepinac
; Over ci to je /h
	lodsb
	cmp al, 'h'
	jne ArgsInvalidSwitch
	lodsb ; Nasledujuci znak musi byt prazdny aby to bol platny prepinac
	cmp al, CharCr ; Sme na konci argumentov?
	je ArgsShowHelp
	call ArgsIsDelimiter
	jne ArgsInvalidSwitch
	jmp ArgsShowHelp

; Vyzera to na nazov suboru
; Overime ci uz mame nazov suboru
; Ak nie tak ho skopirujeme do premennej
AgrsParseFileName:
	cmp al, CharCr ; Sme na konci argumentov?
	je ArgsParseEnd
	cmp HasFileName, 01h ; Uz mame nazov suboru?
	je ArgsParseMultipleFileNames
	mov di, fileNameOffset

; Cyklus na kopirovanie nazvu suboru
; Mame nastavene DS, ES, DI, SI tak aby
; lodsb nacitalo znak z PSP do AL
; stosb ulozilo znak z AL do premennej nazvu suboru
AgrsParseFileNameLoop:
	stosb
	lodsb
	cmp al, CharCr ; Sme na konci argumentov?
	je ArgsFinishFileName
	call ArgsIsDelimiter
	jne AgrsParseFileNameLoop

; Nazov suboru je potrebne ukoncit nulou kvoli DOSu
ArgsFinishFileName:
	mov al, 00h
	stosb
	dec si
	mov HasFileName, 01h
	jmp ArgsParseLoop

; Presli sme celym vstupom
; Este musime zistit ci sme nasli pozadovane argumenty (nazov suboru)
ArgsParseEnd:
	cmp HasFileName, 01h
	je ArgsParseSuccess
	jmp ArgsParseNoFileName

; Sluzi na nastavenie error kodu oznacujuceho neplatny prepinac
ArgsInvalidSwitch:
	mov di, statusOffset
	mov byte ptr es:[di], 01h
	jmp ArgsParseRet

; Sluzi na nastavenie flagu oznacujuceho ze sa ma zobrazit pomocna hlaska
ArgsShowHelp:
	mov di, helpOffset
	mov byte ptr es:[di], 01h
	jmp ArgsParseSuccess

; Sluzi na nastavenie error kodu oznacujuceho chybajuci nazov suboru
ArgsParseNoFileName:
	mov di, statusOffset
	mov byte ptr es:[di], 02h
	jmp ArgsParseRet

; Sluzi na nastavenie error kodu oznacujuceho viacere nazvy suborov
ArgsParseMultipleFileNames:
	mov di, statusOffset
	mov byte ptr es:[di], 03h
	jmp ArgsParseRet

; Sluzi na nastavenie error kodu oznacujuceho bezchybne parsovanie
ArgsParseSuccess:
	mov di, statusOffset
	mov byte ptr es:[di], 00h
	jmp ArgsParseRet

; Sluzi na ukoncenie procedury
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
