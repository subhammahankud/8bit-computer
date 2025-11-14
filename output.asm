ldi A 1
mov M A 100
ldi A 2
mov M A 101
mov A M 100
add M 101
mov M A 102
mov A M 102
cmp A 3
jne else_part_1
mov A M 102
add M 103
mov M A 102
jmp end_if_2
else_part_1:
mov A M 102
sub M 103
mov M A 102
end_if_2:
end:
hlt
