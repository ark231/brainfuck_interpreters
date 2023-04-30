pub mod brainfuck {
    use std::io;
    use std::io::{Read, Write};

    enum Instruction {
        InclPointer,
        DeclPointer,
        InclValue,
        DeclValue,
        Input,
        Output,
    }
    impl Instruction {
        fn from_char(c: &char) -> Option<Instruction> {
            match c {
                '>' => Some(Instruction::InclPointer),
                '<' => Some(Instruction::DeclPointer),
                '+' => Some(Instruction::InclValue),
                '-' => Some(Instruction::DeclValue),
                '.' => Some(Instruction::Output),
                ',' => Some(Instruction::Input),
                _ => None,
            }
        }
    }
    enum Node {
        Instruction(Instruction),
        Loop(Vec<Node>),
    }
    pub struct Code {
        nodes: Vec<Node>,
    }
    impl Code {
        pub fn parse_string(data: &str) -> Result<Code, &str> {
            let mut code = Vec::new();
            code.push(Vec::new());
            for c in data.chars() {
                match Instruction::from_char(&c) {
                    Some(instruction) => {
                        let mut top = code.pop().unwrap();
                        top.push(Node::Instruction(instruction));
                        code.push(top);
                    }
                    None => match c {
                        '[' => {
                            code.push(Vec::new());
                        }
                        ']' => match code.pop() {
                            Some(node) => {
                                let top = code.pop();
                                match top {
                                    Some(mut top_node) => {
                                        top_node.push(Node::Loop(node));
                                        code.push(top_node);
                                    }
                                    None => return Err("unmatching brackets (too many ']')"),
                                }
                            }
                            None => return Err("unmatching brackets (too many ']')"),
                        },
                        _ => (),
                    },
                }
            }
            if code.len() != 1 {
                return Err("unmatching brackets (too many '[')");
            }
            Ok(Code {
                nodes: code.pop().unwrap(), //NOTE: ']'の過多は上で対処済みのため、ここでは必ず成功するはず
            })
        }
    }
    pub struct Interpreter {
        memory: Vec<u8>,
        pointer: usize,
    }
    impl Interpreter {
        pub fn new() -> Self {
            Self {
                memory: vec![0],
                pointer: 0,
            }
        }
        pub fn execute(&mut self, code: &Code) -> Result<(), String> {
            self.execute_recursive(&code.nodes)
        }
        fn execute_recursive(&mut self, nodes: &Vec<Node>) -> Result<(), String> {
            for node in nodes {
                match node {
                    Node::Instruction(instruction) => match instruction {
                        Instruction::InclPointer => {
                            if self.pointer == self.memory.len() - 1 {
                                self.memory.push(0)
                            }
                            self.pointer += 1;
                        }
                        Instruction::DeclPointer => {
                            if self.pointer == 0 {
                                return Err(String::from("negative index is not allowed"));
                            }
                            self.pointer -= 1;
                        }
                        Instruction::InclValue => {
                            let value = &mut self.memory[self.pointer];
                            *value = value.overflowing_add(1).0;
                        }
                        Instruction::DeclValue => {
                            let value = &mut self.memory[self.pointer];
                            *value = value.overflowing_sub(1).0;
                        }
                        Instruction::Output => {
                            match io::stdout().write(&self.memory[self.pointer..self.pointer + 1]) {
                                Err(e) => return Err(format!("failed to write. reason:{:?}", e)),
                                _ => (),
                            }
                        }
                        Instruction::Input => {
                            match io::stdin().read(&mut self.memory[self.pointer..self.pointer + 1])
                            {
                                Ok(0) => {
                                    self.memory[self.pointer] = 0xff; //C-like "EOF "
                                }
                                Err(e) => return Err(format!("failed to read. reason:{:?}", e)),
                                _ => (),
                            }
                        }
                    },
                    Node::Loop(inside_nodes) => {
                        while self.memory[self.pointer] != 0 {
                            self.execute_recursive(&inside_nodes)?
                        }
                    }
                }
            }
            Ok(())
        }
    }
}

use std::env;
use std::fs::File;
use std::io::Read;
use std::process;
fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        eprintln!("the number of artument must be 1");
        process::exit(1);
    }
    let mut file = match File::open(&args[1]) {
        Ok(f) => f,
        Err(e) => {
            eprintln!("failed to open file {}. reason: {:?}", &args[1], e);
            process::exit(1);
        }
    };
    let mut code = String::new();
    match file.read_to_string(&mut code) {
        Err(e) => {
            eprintln!("failed to read from file {}. reason: {:?}", &args[1], e);
            process::exit(1);
        }
        _ => (),
    }
    let code = match brainfuck::Code::parse_string(&code) {
        Ok(i) => i,
        Err(e) => {
            eprintln!("failed to parse source code. reason: {}", e);
            process::exit(1);
        }
    };
    let mut interpreter = brainfuck::Interpreter::new();
    if let Err(e) = interpreter.execute(&code) {
        eprintln!("failed to execute source code. reason: {}", e);
        process::exit(1);
    }
}
