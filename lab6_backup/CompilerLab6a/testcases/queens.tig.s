
Í

Í
 .Í
 OÍ
BEGIN function
L37:
mov $0x8, 115
mov 0(100), 116
mov 115, -4(116)
mov 0(100), 119
add $fffffff8, 119
mov 119, 114
mov $0x0, 124
push 124mov 0(100), 126
mov -4(126), 125
push 125call initArray
mov initArray, 113
mov 113, (114)
mov 0(100), 132
add $fffffff4, 132
mov 132, 112
mov $0x0, 137
push 137mov 0(100), 139
mov -4(139), 138
push 138call initArray
mov initArray, 111
mov 111, (112)
mov 0(100), 145
add $fffffff0, 145
mov 145, 110
mov $0x0, 150
push 150mov 0(100), 154
mov -4(154), 153
mov 0(100), 157
mov -4(157), 156
add 156, 153
sub $1, 153
push 153call initArray
mov initArray, 109
mov 109, (110)
mov 0(100), 163
add $ffffffec, 163
mov 163, 108
mov $0x0, 168
push 168mov 0(100), 172
mov -4(172), 171
mov 0(100), 175
mov -4(175), 174
add 174, 171
sub $1, 171
push 171call initArray
mov initArray, 107
mov 107, (108)
push 100call L12
jmp L36
L36:

END function

BEGIN function
L39:
mov $0x1, 184
mov 184, 106
mov 0(100), 187
mov 4(187), 186
mov 0(100), 191
mov 0(191), 190
mov -4(190), 189
cmp 186, 189
je L33
L34:
mov $0x0, 193
mov 193, 106
jmp L35
L33:
mov 0(100), 198
push 198call L11
jmp L35
L40:
jmp L38
L38:

END function

BEGIN function
L42:
mov 0(100), 206
mov 0(206), 205
push 205call L0
jmp L41
L41:

END function

