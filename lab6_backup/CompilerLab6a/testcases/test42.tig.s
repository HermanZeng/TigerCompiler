sdfÍ
sfdÍ
katiÍ
AllosÍ
KapouÍ
KapoiosÍ
Í
somewhereÍ
anameÍ
BEGIN function
L21:
mov 0(100), 115
add $fffffffc, 115
mov 115, 113
mov $0x0, 120
push 120
mov $0x10, 121
push 121
call initArray
mov initArray, 112
mov 112, (113)
mov 0(100), 126
add $fffffff8, 126
mov 126, 111
mov $0x16, 131
push 131
call initRecord
mov initRecord, 101
mov L11, 0(101)
mov L12, 4(101)
mov $0x0, 137
mov 137, 8(101)
mov $0x0, 139
mov 139, 12(101)
push 101
mov $0x5, 144
push 144
call initArray
mov initArray, 110
mov 110, (111)
mov 0(100), 149
add $fffffff4, 149
mov 149, 109
push L13
mov $0x100, 155
push 155
call initArray
mov initArray, 108
mov 108, (109)
mov 0(100), 160
add $fffffff0, 160
mov 160, 107
mov $0x16, 165
push 165
call initRecord
mov initRecord, 102
mov L14, 0(102)
mov L15, 4(102)
mov $0x2432, 171
mov 171, 8(102)
mov $0x44, 173
mov 173, 12(102)
mov 102, (107)
mov 0(100), 178
add $ffffffec, 178
mov 178, 106
mov $0x8, 183
push 183
call initRecord
mov initRecord, 103
mov L16, 0(103)
add $4, 103
mov 103, 105
mov $0x1900, 192
push 192
mov $0x3, 193
push 193
call initArray
mov initArray, 104
mov 104, (105)
mov 103, (106)
mov $0x1, 199
mov $0x4, 202
mul $0, 202
mov 0(100), 204
mov -4(204), 203
add 203, 202
mov 199, (202)
mov $0x3, 206
mov $0x4, 209
mul $9, 209
mov 0(100), 211
mov -4(211), 210
add 210, 209
mov 206, (209)
mov $0x4, 217
mul $3, 217
mov 0(100), 219
mov -8(219), 218
add 218, 217
mov (217), 214
mov L17, 0(214)
mov $0x23, 221
mov $0x4, 225
mul $1, 225
mov 0(100), 227
mov -8(227), 226
add 226, 225
mov (225), 222
mov 221, 12(222)
mov $0x4, 232
mul $22, 232
mov 0(100), 234
mov -12(234), 233
add 233, 232
mov L18, (232)
mov 0(100), 238
mov -16(238), 237
mov L19, 0(237)
mov $0x2323, 240
mov $0x4, 243
mul $0, 243
mov 0(100), 246
mov -20(246), 245
mov 4(245), 244
add 244, 243
mov 240, (243)
mov $0x2323, 248
mov $0x4, 251
mul $2, 251
mov 0(100), 254
mov -20(254), 253
mov 4(253), 252
add 252, 251
mov 248, (251)
jmp L20
L20:

END function

