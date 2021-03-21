; Vypise retazec ukonceny s "$" na obrazovku
; IN:
; 	strSegment - adresa segmentu v ktorom sa nachadza retazec
; 	strOffset - offset na ktorom sa nachadza retazec
DosPrintString proc uses ax dx ds, strSegment, strOffset
	mov ax, strSegment
	mov ds, ax
	mov dx, strOffset

	mov ah, 09h
	int 21h
	ret
DosPrintString endp

; Vpise znak na obrazovku
; IN:
; 	dl - znak retazca
DosPrintChar proc
	mov ah, 02h
	int 21h
	ret
DosPrintChar endp

; Otvori subor v read only mode
; IN:
; 	nameOffset - offset nazvu suboru
DosOpenFile proc uses dx, nameOffset
	mov dx, nameOffset
	mov al, 00h

	mov ah, 3dh
	int 21h
	ret
DosOpenFile endp

; Zatvori subor
; IN:
; 	fHandle - file handle od operacneho systemu
DosCloseFile proc fHandle
	mov bx, fHandle

	mov ah, 3eh
	int 21h
	ret
DosCloseFile endp


; Precita zo suboru do bufferu n bajtov
; IN:
; 	fHandle - file handle od operacneho systemu
; 	bufferOffset - offset bufferu na ulozenie dat
; 	bufferSize - velkost bufferu
; OUT:
; 	AX - pocet precitanych
DosReadFile proc uses cx dx, fHandle, bufferOffset, bufferSize
	mov bx, fHandle
	mov cx, bufferSize
	mov dx, bufferOffset
	
	mov ah, 3fh
	int 21h
	ret
DosReadFile endp
