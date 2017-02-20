SomebodyÍ
NobodyÍ
BEGIN function
L14:
mov 0(101), 104
add $fffffffc, 104
mov 104, 102
mov $0x8, 109
push 109call initRecord
mov initRecord, 100
mov L11, 0(100)
mov $0x1000, 113
mov 113, 4(100)
mov 100, (102)
mov 0(101), 119
mov -4(119), 118
mov L12, 0(118)
mov 0(101), 122
mov -4(122), 121
jmp L13
L13:

END function

