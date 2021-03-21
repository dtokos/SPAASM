; Asemblery - zadanie 1
; SPAASM - Ak. rok 2020/2021
; Dobroslav Tokos

; Zadanie:
; 20.Vstup, ktorý obsahuje na každom riadku jedno číslo, vypísať v zarovnanom tvare (so správne podpísanými rádmi).

; Spresnenie zadania:
; Program ktory precita textovy subor a z neho vypise naformatovane cisla (tisice, miliony...).
; Vstupny subor musi obsahovat na kazdom riadku iba jedno pozitivne 16 bitove cele cislo <0; 65535>.
; V subore sa mozu nachadzat medzery, taby, prazdne riadky alebo ich kombinacia.
; Program vypisuje iba najdene cisla a prazdne znaky vynechava.

.model small,pascal

; -------------------------------------------------------------------
; Zásobníkovy segment
; -------------------------------------------------------------------
SegStack segment para stack 'stack'
	dw 100h dup(?)
SegStack ends



; -------------------------------------------------------------------
; Dátový segment
; -------------------------------------------------------------------
SegData segment para public 'data'
	CharTab equ 09h
	CharSpace equ 20h
	CharCr equ 0dh
	CharLf equ 0ah

	TestMessage db 'Process file$'
	HelpMessage db 'zadanie [prepinace] subor',CharCr,CharLf,'/h - zobrazi tuto hlasku$'
	ErrorInvalidSwitch db 'Neplatny prepinac!',CharCr,CharLf,'Pozrite napovedu pre viac info$'
	ErrorNoFileName db 'Nezadany nazov suboru!',CharCr,CharLf,'Pozrite napovedu pre viac info$'
	ErrorMultipleFileNames db 'Bolo zadanych viac suborov!',CharCr,CharLf,'Pozrite napovedu pre viac info$'
	ErrorOpenFailed db 'Nepodarilo sa otvorit subor!',CharCr,CharLf,'Ukoncujem program$'
	ErrorBadRead db 'Chyba citania zo suboru!',CharCr,CharLf,'Ukoncujem program$'
	ErrorInvalidChar db 'Neplatny znak v subore!',CharCr,CharLf,'Ukoncujem program$'
	ErrorNumberTooBig db 'Priliz velke cislo v subore!',CharCr,CharLf,'Ukoncujem program$'

	ArgsParseStatus db 00h
	ShowHelp db 00h
	FileName db 80h dup(?)

	FileHandle dw ?
	FileBuffer db 400h dup(?)
	FileBufferSize equ $ - FileBuffer
	BytesRead dw ?

	NumberBuffer db 07h dup('$')
SegData ends



; -------------------------------------------------------------------
; Programový segment
; -------------------------------------------------------------------
SegCode segment para public 'code'
	assume cs:SegCode, ds:SegData, ss:SegStack

	include args.inc
	include string.inc
	include dos.inc

Main:
	; Spustenie parsovania command line argumentov
	; Na zaciatku programu registre DS a ES smeruju na PSP
	; Do procedury ArgsParse posielame adresu segmentu a offsety do ktorych ma ulozit vysledky
	push seg SegData
	push offset ArgsParseStatus
	push offset FileName
	push offset ShowHelp
	call ArgsParse

	; Nastavime ds na SegData kvoli lahsiemu adresovaniu
	; PSP uz nebudeme potrebovat
	mov ax, seg SegData
	mov ds, ax
	
	; Vysporiadame sa s vysledkom parsovania
	; Nenulovy error kod znamena chybu
	cmp ArgsParseStatus, 01h
	je ShowErrorInvalidSwitch
	cmp ArgsParseStatus, 02h
	je ShowErrorNoFileName
	cmp ArgsParseStatus, 03h
	je ShowErrorMultipleFileNames

	; Pri parsovani argumentov nenastala chyba (error kod bol nula)
	; Mozme teda zacat nieco robit podla vstupnych prepinacov a flagov
	cmp ShowHelp, 01h
	je ShowHelpMessage
	jmp ProcessFile

; Vypise pomocnu hlasku
ShowHelpMessage:
	push seg HelpMessage
	push offset HelpMessage
	call DosPrintString
	jmp MainExit

; Vypise hlasku o tom ze sa nasiel neplatny prepinac
ShowErrorInvalidSwitch:
	push seg ErrorInvalidSwitch
	push offset ErrorInvalidSwitch
	call DosPrintString
	jmp MainExit

; Vypise hlasku o tom ze chyba nazov suboru
ShowErrorNoFileName:
	push seg ErrorNoFileName
	push offset ErrorNoFileName
	call DosPrintString
	jmp MainExit

; Vypise hlasku o tom ze sa nenasieli viacere nazovy suboru
ShowErrorMultipleFileNames:
	push seg ErrorMultipleFileNames
	push offset ErrorMultipleFileNames
	call DosPrintString
	jmp MainExit

; Vypise hlasku o tom ze sa nepodaril otvorit subor
ShowErrorOpenFailed:
	push seg ErrorOpenFailed
	push offset ErrorOpenFailed
	call DosPrintString
	jmp MainExit

ShowBadReadMessage:
	push seg ErrorBadRead
	push offset ErrorBadRead
	call DosPrintString
	jmp DosCloseFile

; Zaciatok spracovania suboru
; Subor treba najprv otvorit
ProcessFile:
	push offset FileName
	call DosOpenFile
	jc ShowErrorOpenFailed
	mov FileHandle, ax

ReadLoop:
	push FileHandle
	push offset FileBuffer
	push FileBufferSize
	call DosReadFile
	jc ShowBadReadMessage
	mov BytesRead, ax
	cmp BytesRead, 00h
	jne ReadLoop2
	jmp CloseFile
ReadLoop2:
	lea si, FileBuffer

SkipWhitespace:
	cmp BytesRead, 00h
	je ReadLoop

	dec BytesRead
	lodsb
	call StrIsWhitespace
	je SkipWhitespace

	mov bx, 00h

NumberLoop:
	cmp al, '0'
	jb ShowInvalidChar
	cmp al, '9'
	jg ShowInvalidChar
	xchg ax, bx
	mov cx, 0ah
	mul cx
	xchg ax, bx
	and al, 0fh
	mov ah, 00h
	add bx, ax
	jc ShowNumberTooBig
	
	dec BytesRead
	cmp BytesRead, 00h
	je FormatNumber
	lodsb
	cmp al, CharCr
	je FormatNumber
	call StrIsWhitespace
	je FormatNumber
	jmp NumberLoop

ShowInvalidChar:
	push seg ErrorInvalidChar
	push offset ErrorInvalidChar
	call DosPrintString
	jmp CloseFile

ShowNumberTooBig:
	push seg ErrorNumberTooBig
	push offset ErrorNumberTooBig
	call DosPrintString
	jmp CloseFile

FormatNumber:
	mov di, offset NumberBuffer
	mov ax, bx
	mov bx, 0ah
	mov cl, 00h
FormatNumberLoop:
	mov dx, 00h
	div bx
	add dx, '0'
	xchg ax, dx
	stosb
	inc cl
	cmp cl, 03h
	jne FormatNumberLoop2
	cmp dx, 00h
	je FormatNumberLoop2
	mov al, CharSpace
	stosb
FormatNumberLoop2:
	xchg ax, dx
	cmp ax, 00h
	jne FormatNumberLoop

PrintNumber:
	dec di
	mov dl, [di]
	call DosPrintChar
	cmp di, offset NumberBuffer
	jne PrintNumber

	mov dl, CharCr
	call DosPrintChar
	mov dl, CharLf
	call DosPrintChar

SkipWhitespaceEnd:
	dec BytesRead
	lodsb
	cmp al, CharCr
	je SkipWhitespaceEndJump
	cmp al, CharLf
	je SkipWhitespaceEndJump
	call StrIsWhitespace
	je SkipWhitespaceEnd

SkipWhitespaceEndJump:
	dec si
	inc BytesRead
	jmp SkipWhitespace

CloseFile:
	push FileHandle
	call DosCloseFile
	jmp MainExit

; Ukonci program
MainExit:
	mov ah, 4ch
	int 21h
SegCode ends
end Main
