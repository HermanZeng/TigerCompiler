BEGIN function
L12:
mov 0(100), 104
add $fffffffc, 104
mov 104, 102
mov $0x0, 109
push 109mov $0x10, 110
push 110call initArray
mov initArray, 101
mov 101, (102)
mov 0(100), 115
mov -4(115), 114
jmp L11
L11:

END function

