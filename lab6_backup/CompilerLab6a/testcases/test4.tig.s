BEGIN function
L16:
push 100
call L11
jmp L15
L15:

END function

BEGIN function
L18:
mov $0x1, 108
mov 108, 101
mov 0(100), 111
mov 4(111), 110
mov $0x0, 113
cmp 110, 113
je L12
L13:
mov $0x0, 114
mov 114, 101
mov 0(100), 117
mov 4(117), 116
mov 116, 107
mov 0(100), 122
push 122
call L11
mov L11, 106
mul 107, 106
jmp L14
L12:
jmp L14
L19:
jmp L17
L17:

END function

