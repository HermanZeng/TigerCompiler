str2Í
 Í
strÍ
BEGIN function
L17:
push 100
call L11
jmp L16
L16:

END function

BEGIN function
L19:
mov 0(100), 107
push 107
call L11
jmp L18
L18:

END function

BEGIN function
L21:
mov 0(100), 113
push 113
call L12
jmp L20
L20:

END function

