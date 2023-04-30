#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <variant>
#include <vector>
#define failed(msg)                  \
    std::cerr << (msg) << std::endl; \
    std::exit(EXIT_FAILURE);
namespace brainfuck {
namespace impl_ {
enum class Instruction : std::uint8_t { InclPointer, DeclPointer, InclValue, DeclValue, Output, Input };
struct Loop {
    std::vector<std::variant<Instruction, Loop>> nodes;

    Loop(decltype(nodes) nodes) : nodes(nodes) {}
};
}  // namespace impl_
using Node = std::variant<impl_::Instruction, impl_::Loop>;
struct Code {
    class ParseError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    std::vector<Node> nodes;

    Code(std::ifstream& infile) {
        char c;
        std::stack<decltype(this->nodes)> code_fragments;
        code_fragments.push(decltype(this->nodes){});
        while ((c = infile.get()) != EOF) {
            switch (c) {
                case '>':
                    code_fragments.top().push_back(impl_::Instruction::InclPointer);
                    break;
                case '<':
                    code_fragments.top().push_back(impl_::Instruction::DeclPointer);
                    break;
                case '+':
                    code_fragments.top().push_back(impl_::Instruction::InclValue);
                    break;
                case '-':
                    code_fragments.top().push_back(impl_::Instruction::DeclValue);
                    break;
                case '.':
                    code_fragments.top().push_back(impl_::Instruction::Output);
                    break;
                case ',':
                    code_fragments.top().push_back(impl_::Instruction::Input);
                    break;
                case '[':
                    code_fragments.push(decltype(this->nodes){});
                    break;
                case ']':
                    if (code_fragments.size() == 1) {
                        throw ParseError("unmatching bracket (too many ']')");
                    }
                    auto completed_loop = code_fragments.top();
                    code_fragments.pop();
                    code_fragments.top().push_back(impl_::Loop(completed_loop));
                    break;
            }
        }
        if (code_fragments.size() != 1) {
            throw ParseError("unmatching bracket (too many '[')");
        }
        nodes = code_fragments.top();
    }
};
class Interpreter {
   public:
    class ExecutionError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

   private:
    std::vector<char> memory_;
    std::size_t index_;

    void execute_recursive_(const decltype(Code::nodes)& nodes) {
        for (const auto& node : nodes) {
            if (std::holds_alternative<impl_::Instruction>(node)) {
                switch (std::get<impl_::Instruction>(node)) {
                    case impl_::Instruction::InclPointer:
                        if (index_ == memory_.size() - 1) {
                            memory_.push_back(0);
                        }
                        index_++;
                        break;
                    case impl_::Instruction::DeclPointer:
                        if (index_ == 0) {
                            throw ExecutionError("negative index is not allowed");
                        }
                        index_--;
                        break;
                    case impl_::Instruction::InclValue:
                        memory_[index_]++;
                        break;
                    case impl_::Instruction::DeclValue:
                        memory_[index_]--;
                        break;
                    case impl_::Instruction::Output:
                        std::cout.put(memory_[index_]);
                        break;
                    case impl_::Instruction::Input:
                        memory_[index_] = std::cin.get();
                        break;
                }
            } else {
                const auto& loop = std::get<impl_::Loop>(node);
                while (memory_[index_] != 0) {
                    execute_recursive_(loop.nodes);
                }
            }
        }
    }

   public:
    Interpreter() : memory_(1), index_(0) {}
    void execute(const Code& code) { execute_recursive_(code.nodes); }
};
}  // namespace brainfuck
int main(int argc, char** argv) {
    if (argc != 2) {
        failed("the number of argument must be 1");
    }
    std::ifstream infile(argv[1]);
    if (not infile) {
        failed("failed to open source file");
    }
    brainfuck::Interpreter interpreter{};
    try {
        brainfuck::Code code(infile);
        interpreter.execute(code);
    } catch (brainfuck::Code::ParseError& e) {
        std::cerr << "failed to parse source file. reason:" << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    } catch (brainfuck::Interpreter::ExecutionError& e) {
        std::cerr << "execution of source code has failed. reason:" << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
