#include <fstream>
#include <iostream>
#include <stack>
#include <vector>
#define failed(msg)                  \
    std::cerr << (msg) << std::endl; \
    exit(EXIT_FAILURE);
int main(int argc, char** argv) {
    if (argc != 2) {
        failed("the number of argument must be 1");
    }
    std::ifstream infile(argv[1]);
    if (not infile) {
        failed("failed to open source file");
    }
    char instruction;
    std::vector<char> memory(1);
    size_t index = 0;
    std::stack<decltype(infile.tellg())> loop_stack;
    while ((instruction = infile.get()) != EOF) {
        switch (instruction) {
            case '>':
                if (index == memory.size() - 1) {
                    memory.push_back(0);
                }
                index++;
                break;
            case '<':
                if (index == 0) {
                    failed("negative index is not allowed");
                }
                index--;
                break;
            case '+':
                memory[index]++;
                break;
            case '-':
                memory[index]--;
                break;
            case '.':
                std::cout.put(memory[index]);
                break;
            case ',':
                memory[index] = std::cin.get();
                break;
            case '[':
                if (memory[index] != 0) {
                    loop_stack.push(infile.tellg());
                } else {
                    int depth = 1;
                    while (true) {
                        char tmp_instruction = infile.get();
                        if (tmp_instruction == EOF) {
                            failed("unmatching bracket");
                        }
                        if (tmp_instruction == '[') {
                            depth++;
                        } else if (tmp_instruction == ']') {
                            depth--;
                            if (depth == 0) {
                                break;
                            }
                        }
                    }
                }
                break;
            case ']':
                if (memory[index] != 0) {
                    infile.seekg(loop_stack.top());
                } else {
                    loop_stack.pop();
                }
                break;
        }
    }
}
