BEGIN function
L14:
mov $0x0, 101
jmp L13
L13:

END function

BEGIN function
L16:
mov 0(100), 104
mov 4(104), 103
jmp L15
L15:

END function

BEGIN function
L18:
mov 0(100), 108
mov 4(108), 107
jmp L17
L17:

END function

