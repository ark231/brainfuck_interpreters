#!/usr/bin/env python3
import sys
import io


def failed(msg):
    print(msg, file=sys.stderr)
    exit(1)


def main():
    if len(sys.argv) != 2:
        failed("the number of argument must be 1")
    with open(sys.argv[1], "r") as infile:
        code = io.StringIO(infile.read())
    instruction: str
    memory = bytearray(1)
    index = 0
    loop_stack = []
    instruction = code.read(1)
    while instruction != "":
        if instruction == ">":
            if index == len(memory) - 1:
                memory.append(0)
            index += 1
        elif instruction == "<":
            if index == 0:
                failed("negative index is not allowed")
            index -= 1
        elif instruction == "+":
            memory[index] = (memory[index] + 1) % (2**8)
        elif instruction == "-":
            memory[index] = (memory[index] - 1) % (2**8)
        elif instruction == ".":
            print(chr(memory[index]), end="")
        elif instruction == ",":
            memory[index] = ord(sys.stdin.read(1)) % (2**8)
        elif instruction == "[":
            if memory[index] != 0:
                loop_stack.append(code.tell())
            else:
                depth = 1
                while True:
                    tmp_instruction = code.read(1)
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
                code.seek(loop_stack[-1])
            else:
                loop_stack.pop()

        instruction = code.read(1)


if __name__ == "__main__":
    main()
