ArgsSkipWhitespace proc
	lodsb
	call ArgsIsDelimiter
	je ArgsSkipWhitespace
	ret
ArgsSkipWhitespace endp

ArgsIsDelimiter proc
	cmp al, CharSpace
	je ArgsIsDelimiterRet
	cmp al, CharTab
ArgsIsDelimiterRet:
	ret
ArgsIsDelimiter endp
