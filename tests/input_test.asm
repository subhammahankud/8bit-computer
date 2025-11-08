.text

ldi A 10
mov M A %i
ldi A 20
mov M A %j
mov A M %i
add M %j
mov M A %k
mov A M %k
cmp A 30
jne else_part_1
mov A M %k
add M %g
mov M A %k
jmp end_if_2
else_part_1:
mov A M %k
sub M %g
mov M A %k
end_if_2:
end:
hlt

.data
i = 100
j = 101
k = 102
g = 103