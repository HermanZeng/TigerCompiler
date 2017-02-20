BEGIN function
L17:
mov $0x1, 102
mov 102, 101
mov $0x1, 104
mov 104, 100
mov $0xa, 106
mov $0x14, 107
cmp 106, 107
jg L11
L12:
mov $0x0, 108
mov 108, 100
L11:
mov $0x1, 111
cmp 100, 111
je L13
L14:
mov $0x0, 112
mov 112, 101
jmp L15
L13:
jmp L15
L18:
jmp L16
L16:

END function

