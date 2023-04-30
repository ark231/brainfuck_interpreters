#!/usr/bin/env python3
import sys


def failed(msg):
    print(msg, file=sys.stderr)
    exit(1)


def count_same(src, instruction: str) -> (int, str):
    c = src.read(1)
    count = 0
    while c == instruction:
        count += 1
        c = src.read(1)
    return (count, c)


def main():
    if len(sys.argv) != 2:
        failed("the number of argument must be 1")
    with open(sys.argv[1], "r") as infile:
        instruction: str
        memory = bytearray(1)
        index = 0
        loop_stack = []
        instruction = infile.read(1)
        while instruction != "":
            if instruction == ">":
                if index == len(memory) - 1:
                    memory.append(0)
                index += 1
            elif instruction == "<":
                count, next_instruction = count_same(infile, "<")
                index -= 1 + count
                if index < 0:
                    failed("negative index is not allowed")
                instruction = next_instruction
                continue
            elif instruction == "+":
                count, next_instruction = count_same(infile, "+")
                memory[index] = (memory[index] + (count + 1)) % (2**8)
                instruction = next_instruction
                continue
            elif instruction == "-":
                count, next_instruction = count_same(infile, "-")
                memory[index] = (memory[index] - (count + 1)) % (2**8)
                instruction = next_instruction
                continue
            elif instruction == ".":
                print(chr(memory[index]), end="")
            elif instruction == ",":
                memory[index] = ord(sys.stdin.read(1)) % (2**8)
            elif instruction == "[":
                if memory[index] != 0:
                    loop_stack.append(infile.tell())
                else:
                    depth = 1
                    while True:
                        tmp_instruction = infile.read(1)
                        if tmp_instruction == "":
                            failed("unmatching bracket")
                        if tmp_instruction == "[":
                            depth += 1
                        elif tmp_instruction == "]":
                            depth -= 1
                            if depth == 0:
                                break
            elif instruction == "]":
                if memory[index] != 0:
                    infile.seek(loop_stack[-1])
                else:
                    loop_stack.pop()

            instruction = infile.read(1)


if __name__ == "__main__":
    main()
