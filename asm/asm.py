#!/usr/bin/env python3

import re
import sys

progf = sys.argv[1]

inst = {
    "nop": 0x00,
    "call": 0b00000001,
    "ret": 0b00000010,
    "lda": 0b10000111,
    "out": 0b00000011,
    "in": 0b00000100,
    "hlt": 0b00000101,
    "cmp": 0b00000110,
    "sta": 0b10111000,
    "jmp": 0b00011000,
    "jz": 0b00011001,
    "jnz": 0b00011010,
    "je":  0b00011001,
    "jne": 0b00011010,
    "jc":  0b00011011,
    "jnc": 0b00011100,
    "push": 0b00100000,
    "pop": 0b00101000,
    "add": 0b01000000,
    "sub": 0b01001000,
    "inc": 0b01010000,
    "dec": 0b01011000,
    "and": 0b01100000,
    "or": 0b01101000,
    "xor": 0b01110000,
    "adc": 0b01111000,
    "ldi": 0b00010000,
    "mov": 0b10000000,
}

reg = {
    "A": 0b000,
    "B": 0b001,
    "C": 0b010,
    "D": 0b011,
    "E": 0b100,
    "F": 0b101,
    "G": 0b110,
    "M": 0b111,
}

TEXT, DATA = 0, 1
MEM_SIZE = 256

mem = [0 for _ in range(MEM_SIZE)]
cnt = 0

labels = {}
data = {}
data_addr = {}

def rich_int(v):
    if v.startswith("0x"):
        return int(v, 16)
    elif v.startswith("0b"):
        return int(v, 2)
    else:
        return int(v)

with open(progf) as f:
    section = None  # Ensure section is initialized
    for l in f:
        l = re.sub(";.*", "", l).strip()
        if not l:
            continue

        if l == ".text":
            section = TEXT
            continue
        elif l == ".data":
            section = DATA
            continue

        if section == DATA:
            n, v = map(str.strip, l.split("=", 1))
            data[n] = int(v)
        elif section == TEXT:
            kw = l.split()
            if kw[0].endswith(":"):
                labels[kw[0].rstrip(":")] = cnt
                continue

            current_inst = kw[0]

            if current_inst == "ldi":
                r = reg[kw[1]]
                opcode = (inst["ldi"] & 0b11111000) | r
                imm = rich_int(kw[2])
                mem[cnt] = opcode
                mem[cnt + 1] = imm
                cnt += 2

            elif current_inst in ("push", "pop"):
                r = reg[kw[1]]
                opcode = (inst[current_inst] & 0b11111000) | r
                mem[cnt] = opcode
                cnt += 1

            elif current_inst == "mov":
                op1 = reg[kw[1]]
                op2 = reg[kw[2]]
                opcode = (inst["mov"] & 0b11000111) | (op1 << 3) | op2
                mem[cnt] = opcode
                cnt += 1

                # Check if memory reference uses %label
                if len(kw) > 3 and kw[3].startswith("%"):
                    mem[cnt] = kw[3]
                    cnt += 1

            elif current_inst in ("add", "sub", "cmp"):
                # handle instructions like add M %k
                opcode = inst[current_inst]
                mem[cnt] = opcode
                cnt += 1

                for arg in kw[1:]:
                    if arg in reg:
                        mem[cnt] = reg[arg]
                    elif arg.startswith("%"):
                        mem[cnt] = arg
                    else:
                        mem[cnt] = rich_int(arg)
                    cnt += 1

            else:
                # Generic fallback
                mem[cnt] = inst.get(current_inst, 0)
                cnt += 1

# Write data into memory
for k, v in data.items():
    data_addr[k] = cnt
    mem[cnt] = v
    cnt += 1

# Update label and data address mapping
data_addr.update(labels)

# Replace %labels with numeric addresses
for i, b in enumerate(mem):
    if isinstance(b, str) and b.startswith("%"):
        label_name = b.lstrip("%")
        if label_name in data_addr:
            mem[i] = data_addr[label_name]
        else:
            print(f"⚠️ Warning: Undefined label {label_name}")
            mem[i] = 0

# Print memory dump
print(' '.join(['%02x' % int(b) for b in mem[:cnt]]))
