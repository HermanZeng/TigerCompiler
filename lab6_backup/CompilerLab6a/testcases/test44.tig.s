BEGIN function
L12:
mov 0(101), 104
add $fffffffc, 104
mov 104, 102
mov $0x0, 109
push 109
call initRecord
mov initRecord, 100
mov 100, (102)
mov 0(101), 114
mov 100, -4(114)
jmp L11
L11:

END function

