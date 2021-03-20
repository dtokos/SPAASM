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



; -------------------------------------------------------------------
; Zásobníkovy segment
; -------------------------------------------------------------------
SegStack segment para stack 'stack'
	dw 256 dup(?)
SegStack ends



; -------------------------------------------------------------------
; Dátový segment
; -------------------------------------------------------------------
SegData segment para public 'data'
	asd db 1
SegData ends



; -------------------------------------------------------------------
; Programový segment
; -------------------------------------------------------------------
SegCode segment para public 'code'
	assume cs:SegCode, ds:SegData, ss:SegStack

Main:
	mov ah, 1
	int 21h
	cmp al, 13
	jz MainEnd
	mov dl, al
	mov ah, 2
	int 21h
	jmp Main

MainEnd:
	Mov ah, 4ch
	int 21h
SegCode ends
end Main

