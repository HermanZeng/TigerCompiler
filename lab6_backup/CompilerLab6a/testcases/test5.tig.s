BEGIN function
L12:
mov 0(102), 106
add $fffffffc, 106
mov 106, 104
mov $0x8, 111
push 111
call initRecord
mov initRecord, 101
mov $0x0, 113
mov 113, 0(101)
add $4, 101
mov 101, 103
mov $0x0, 120
push 120
call initRecord
mov initRecord, 100
mov 100, (103)
mov 101, (104)
mov 0(102), 127
mov -4(127), 126
jmp L11
L11:

END function

