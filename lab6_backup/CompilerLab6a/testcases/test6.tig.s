str2Í
strÍ
BEGIN function
L16:
push 100
call L11
jmp L15
L15:

END function

BEGIN function
L18:
mov 0(100), 107
push 107
call L11
jmp L17
L17:

END function

BEGIN function
L20:
mov 0(100), 112
push 112
call L12
jmp L19
L19:

END function

