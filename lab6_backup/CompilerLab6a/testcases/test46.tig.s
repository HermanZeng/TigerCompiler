BEGIN function
L16:
mov 0(101), 106
add $fffffffc, 106
mov 106, 104
mov $0x0, 111
push 111
call initRecord
mov initRecord, 100
mov 100, (104)
mov $0x1, 115
mov 115, 103
mov 0(101), 118
mov -4(118), 117
cmp 117, 100
je L13
L14:
mov $0x0, 121
mov 121, 103
L13:
mov $0x1, 124
mov 124, 102
mov 0(101), 127
mov -4(127), 126
cmp 126, 100
jne L11
L12:
mov $0x0, 130
mov 130, 102
L11:
jmp L15
L15:

END function

