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

	ArgsParseStatus db 00h
	ShowHelp db 00h
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


ProcessFile:
	push seg FileName
	push offset FileName
	call StrPrint
	jmp MainExit

; Vypis hlasku o tom ze sa nasiel neplatny prepinac
ShowErrorInvalidSwitch:
	push seg ErrorInvalidSwitch
	push offset ErrorInvalidSwitch
	call StrPrint
	jmp MainExit

; Vypis hlasku o tom ze chyba nazov suboru
ShowErrorNoFileName:
	push seg ErrorNoFileName
	push offset ErrorNoFileName
	call StrPrint
	jmp MainExit

; Vypis hlasku o tom ze sa nenasieli viacere nazovy suboru
ShowErrorMultipleFileNames:
	push seg ErrorMultipleFileNames
	push offset ErrorMultipleFileNames
	call StrPrint
	jmp MainExit

; Vypis pomocnu hlasku
ShowHelpMessage:
	push seg HelpMessage
	push offset HelpMessage
	call StrPrint
	jmp MainExit

; Ukonci program
MainExit:
	mov ah, 4ch
	int 21h
SegCode ends
end Main
