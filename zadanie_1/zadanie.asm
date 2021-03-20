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

	FileName db 80h dup(?)
SegData ends



; -------------------------------------------------------------------
; Programový segment
; -------------------------------------------------------------------
SegCode segment para public 'code'
	assume cs:SegCode, ds:SegData, ss:SegStack

	include args.inc
	include string.inc

Main:
; Nastavenie segmentovych registrov
; es - Datovy segment
; ds - Ostava na PSP
	mov ax, seg SegData
	mov es, ax

; Nasmerovanie na PSP
	mov si, 81h
DFOSJH:
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
	jz ShowHelp

ShowHelp:
	push seg HelpMessage
	push offset HelpMessage
	call StrPrint
	jmp MainExit

ArgsInvalidSwitch:
	push seg ErrorInvalidSwitch
	push offset ErrorInvalidSwitch
	call StrPrint
	jmp MainExit


AgrsParseFileName:
	cmp al, CharCr ; Sme na konci argumentov?
	je ProcessFile
	lea di, FileName

AgrsParseFileNameLoop:
	stosb
	lodsb
	cmp al, CharCr ; Sme na konci argumentov?
	je ArgsFinishFileName
	call ArgsIsDelimiter
	jne AgrsParseFileNameLoop

ArgsFinishFileName:
	mov al, 0
	stosb
	dec si
	jmp DFOSJH


ProcessFile:
	push seg FileName
	push offset FileName
	call StrPrint
	jmp MainExit

MainExit:
	mov ah, 4ch
	int 21h
SegCode ends
end Main
