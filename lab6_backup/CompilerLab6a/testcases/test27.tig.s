BEGIN function
L13:
mov $0x0, 101
mov 0(100), 102
mov 101, -4(102)
push 100call L11
jmp L12
L12:

END function

BEGIN function
L15:
mov 0(100), 109
mov 4(109), 108
jmp L14
L14:

END function

