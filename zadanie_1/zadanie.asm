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

; Ulohy z 2. casti:
; 7. Plus 1 bod: Ak budú korektne spracované vstupné súbory s veľkosťou nad 64 kB. (vid test_6.txt)
; 11.Plus 1 bod je možné získať za (zmysluplné) použitie reťazcových inštrukcií (MOVS, CMPS, STOS, etc.).
; 12.Plus 1 bod je možné získať za (dobré) komentáre, resp. dokumentáciu, v anglickom jazyku.

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
	FileBuffer db 8000h dup(?)
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

; Vypise hlasku o tom ze pri citani zo suboru nastala chyba
; Zatvori subor
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
	jc ShowErrorOpenFailed ; Podarilo sa otvorit subor?
	mov FileHandle, ax

; Cyklus na citanie zo suboru
; Ak je pocet precitanych bajtov = 0 tak skoncime
; Zatvori subor
ReadLoop:
	push FileHandle
	push offset FileBuffer
	push FileBufferSize
	call DosReadFile
	jc ShowBadReadMessage ; Podarilo sa citanie zo suboru?
	mov BytesRead, ax
	cmp BytesRead, 00h ; Sme uz na konci suboru?
	jne ReadLoop2 ; Ak nie tak pokracuj v spracovani
	jmp CloseFile ; Inak zatvor subor a skonci
ReadLoop2:
	lea si, FileBuffer

; V subore su ignorovane prazdne znaky na zaciatkoch riadkov
SkipWhitespace:
	cmp BytesRead, 00h ; Dostali sme sa na koniec suboru?
	je ReadLoop

	dec BytesRead
	lodsb
	call StrIsWhitespace
	je SkipWhitespace

	mov bx, 00h ; Naslo sa potencionalne cislo. Pripravime si BX na ukladanie sparsovaneho cisla.

; Nasiel sa znak ktory nie je prazdny
; Overime ci to je cislo a zacneme parsovat
; Parsovanie prebieha aritmeticky (stare * 10 + nove)
; Ak cislo pretecie (> 65535) tak skoncime
NumberLoop:
	cmp al, '0' ; Je znak cislo z rozsahu <0, 9>?
	jb ShowInvalidChar ; Ak nie tak koncime
	cmp al, '9' ; Je znak cislo z rozsahu <0, 9>?
	jg ShowInvalidChar ; Ak nie tak koncime
	xchg ax, bx ; Prehodime si stare cislo a novy prirastok
	mov cx, 0ah ; Pripravime si cislo 10
	mul cx ; Vynasobime stare cislo
	xchg ax, bx ; Prehodime cisla spat
	and al, 0fh ; Spravime z ascii znaku cislo pomocou bitwise and
	mov ah, 00h ; Zresetujeme AH aby vysledok bol platne 16bit cislo
	add bx, ax ; Scitame stare cislo a prirastok
	jc ShowNumberTooBig ; Ak sa presiahla hranica tak skoncime
	
	; Ak sme na konci buffera alebo uz nenasleduje dalsie cislo tak ho mozme naformatovat a vypisat
	dec BytesRead
	cmp BytesRead, 00h
	je FormatNumber
	lodsb
	call StrIsWhitespace
	je FormatNumber
	jmp NumberLoop

; Vypise hlasku o tom ze v subore sa nachadza neplatny znak
; Zatvori subor
ShowInvalidChar:
	push seg ErrorInvalidChar
	push offset ErrorInvalidChar
	call DosPrintString
	jmp CloseFile

; Vypise hlasku o tom ze v subore sa nachadza priliz velke cislo
; Zatvori subor
ShowNumberTooBig:
	push seg ErrorNumberTooBig
	push offset ErrorNumberTooBig
	call DosPrintString
	jmp CloseFile

; Naformatuje cislo, oddeli skupiny cifier medzerami
; Pri formatovani delime desiatimi a uchovavame si zvysok
FormatNumber:
	mov di, offset NumberBuffer ; Nasmerujeme si DI na buffer aby sme veteli pouzivat stringove instrukcie
	mov ax, bx ; Presunieme cislo do akumulatora
	mov bx, 0ah ; Do BX si pripravime delitel 10
	mov cl, 00h ; V CL budeme pocitat pocet cifier
FormatNumberLoop:
	mov dx, 00h ; Prevencia pred delenim nulou
	div bx ; Vydelime cislo 10
	add dx, '0' ; Spravime zo zvysku (poslednej cifri) ascii znak
	xchg ax, dx ; Vymenime AX a DX aby sme vedeli pouzit stringove instrukcie
	stosb ; Pridame znak do buffera
	inc cl ; Zvysime pocet cifier
	cmp cl, 03h ; Vypisali sme skupinu troch cifier?
	jne FormatNumberLoop2 ; Ak nie tak pokracujeme
	cmp dx, 00h ; Nasleduju este dalsie cifry? (Aby sa zle neformatovali stovky " 123")
	je FormatNumberLoop2 ; Ak nie tak pokracujeme
	mov al, CharSpace ; Inak pridame medzeru
	stosb
FormatNumberLoop2:
	xchg ax, dx ; Vymenime AX, DX aby sme mohli pokracovat v deleni
	cmp ax, 00h ; Minuli sa nam uz vsetky cifry?
	jne FormatNumberLoop ; Ak nie tak pokracujeme vo formatovani

; Vypise cislo na obrazovku
; Kedze pri deleni sa nam cislo "otoci" tak ho vypisujeme odzadu
PrintNumber:
	dec di ; Na konci formatovania sme jeden znak za poslednou cifrou tak sa musime vratit
	mov dl, [di] ; Nacitame si znak z adresy ulozenej v di
	call DosPrintChar ; Vypiseme ho
	cmp di, offset NumberBuffer ; Vypisali sme uz vsetky znaky?
	jne PrintNumber ; Ak nie tak vypisujeme dalej

	mov dl, CharCr ; Vypiseme este CR
	call DosPrintChar
	mov dl, CharLf ; Vypiseme este LF
	call DosPrintChar

; Preskocime prazdne znaky na konci suboru
SkipWhitespaceEnd:
	dec BytesRead ; Pokracujeme preskakovanim prazdnych znakov na konci riadka
	lodsb
	cmp al, CharCr ; Nasiel sa CR?
	je SkipWhitespaceEndJump ; Ideme na dalsi riadok a hladame dalsie cisla
	cmp al, CharLf ; Nasiel alebo LF? (mal som problem so subormi vytvorenymi na Unix-like pocitacoch)
	je SkipWhitespaceEndJump ; Ideme na dalsi riadok a hladame dalsie cisla
	call StrIsWhitespace ; Je to iba prazdny znak?
	je SkipWhitespaceEnd ; Pokracujeme v preskakovani

; Relativny skok uz nestacil tak toto je skokansky mostik :)
; Skace na pokracovanie citania znakov na zaciatku riadku
SkipWhitespaceEndJump:
	dec si
	inc BytesRead
	jmp SkipWhitespace

; Zatvori subor
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
