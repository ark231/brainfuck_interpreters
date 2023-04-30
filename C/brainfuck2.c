#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum Instruction {
  INCL_POINTER,
  DECL_POINTER,
  INCL_VALUE,
  DECL_VALUE,
  OUTPUT,
  INPUT
};

enum NodeValueType { NODE_INSTRUCTION, NODE_VEC };
typedef struct {
  enum NodeValueType type;
  void *value;
} Node;
void node_assign(Node *this, enum NodeValueType type, void *value);

enum VecValueType { VEC_INSTRUCTION, VEC_NODE, VEC_CHAR };
typedef struct {
  enum VecValueType type;
  size_t capasity_p;
  size_t size;
  void *memory_p;
} Vec;
void vec_init(Vec *this, enum VecValueType type);
void vec_push_back(Vec *this, void *value);
void *vec_pop_back(Vec *this);
void *vec_at(Vec *this, size_t index);
void *vec_back(Vec *this);

typedef struct {
  Vec nodes;
} Code;
void code_init_from_file(Code *this, FILE *infile);

typedef struct {
  Vec memory;
  size_t index;
} Interpreter;
void interpreter_init(Interpreter *this);
void interpreter_execute(Interpreter *this, Code *code);
void interpreter_execute_recursive_p(Interpreter *this, Vec *nodes);

#define failed(msg)                                                            \
  fprintf(stderr, "%s\n", (msg));                                              \
  exit(EXIT_FAILURE)

int main(int argc, char **argv) {
  if (argc != 2) {
    failed("the number of argument must be 1");
  }
  FILE *infile;
  if ((infile = fopen(argv[1], "r")) == NULL) {
    failed("failed to open source file");
  }
  Vec nodes;
  vec_init(&nodes, VEC_NODE);
  Interpreter interpreter;
  interpreter_init(&interpreter);
  Code code;
  code_init_from_file(&code, infile);
  interpreter_execute(&interpreter, &code);
  return 0;
}

void node_assign(Node *this, enum NodeValueType type, void *value) {
  this->type = type;
  switch (type) {
  case NODE_INSTRUCTION:
    this->value = malloc(sizeof(enum Instruction));
    *(enum Instruction *)this->value = *(enum Instruction *)value;
    break;
  case NODE_VEC:
    this->value = value;
    *(Vec *)this->value = *(Vec *)value;
    break;
  }
}

void vec_init(Vec *this, enum VecValueType type) {
  this->type = type;
  this->capasity_p = 0;
  this->size = 0;
  this->memory_p = NULL;
}
void vec_check_capasity_for_push_back_p(Vec *this, size_t value_size) {
  if (this->capasity_p == 0) {
    this->memory_p = calloc(1, value_size);
    this->capasity_p = 1;
  } else if (this->capasity_p < this->size + 1) {
    void *new_memory =
        reallocarray(this->memory_p, this->capasity_p *= 2, value_size);
    if (new_memory == NULL) {
      free(this->memory_p);
      failed("error: failed to allocate memory");
    }
    this->memory_p = new_memory;
  }
}
void vec_push_back(Vec *this, void *value) {
  switch (this->type) {
  case VEC_CHAR:
    vec_check_capasity_for_push_back_p(this, sizeof(char));
    ((char *)this->memory_p)[this->size] = *(char *)value;
    break;
  case VEC_INSTRUCTION:
    vec_check_capasity_for_push_back_p(this, sizeof(enum Instruction));
    ((enum Instruction *)this->memory_p)[this->size] =
        *(enum Instruction *)value;
    break;
  case VEC_NODE:
    vec_check_capasity_for_push_back_p(this, sizeof(Node));
    ((Node *)this->memory_p)[this->size] = *(Node *)value;
    break;
  }
  this->size++;
}
void *vec_pop_back(Vec *this) { return vec_at(this, --this->size); }
void *vec_at(Vec *this, size_t index) {
  switch (this->type) {
  case VEC_CHAR:
    return &((char *)this->memory_p)[index];
    break;
  case VEC_INSTRUCTION:
    return &((enum Instruction *)this->memory_p)[index];
    break;
  case VEC_NODE:
    return &((Node *)this->memory_p)[index];
    break;
  }
}
void *vec_back(Vec *this) { return vec_at(this, this->size - 1); }

void code_init_from_file(Code *this, FILE *infile) {
  vec_init(&this->nodes, VEC_NODE);
  char instruction = '\0';
  Vec loop_stack;
  vec_init(&loop_stack, VEC_NODE);
  vec_push_back(&loop_stack, &this->nodes);
  while ((instruction = fgetc(infile)) != EOF) {
    Node new_node;
    enum Instruction new_instruction;
    Vec new_loop;
    Vec *current_loop;
    if (loop_stack.size == 0) {
      current_loop = &this->nodes;
    } else {
      current_loop = vec_back(&loop_stack);
    }
    switch (instruction) {
    case '>':
      new_instruction = INCL_POINTER;
      node_assign(&new_node, NODE_INSTRUCTION, &new_instruction);
      vec_push_back(current_loop, &new_node);
      break;
    case '<':
      new_instruction = DECL_POINTER;
      node_assign(&new_node, NODE_INSTRUCTION, &new_instruction);
      vec_push_back(current_loop, &new_node);
      break;
    case '+':
      new_instruction = INCL_VALUE;
      node_assign(&new_node, NODE_INSTRUCTION, &new_instruction);
      vec_push_back(current_loop, &new_node);
      break;
    case '-':
      new_instruction = DECL_VALUE;
      node_assign(&new_node, NODE_INSTRUCTION, &new_instruction);
      vec_push_back(current_loop, &new_node);
      break;
    case '.':
      new_instruction = OUTPUT;
      node_assign(&new_node, NODE_INSTRUCTION, &new_instruction);
      vec_push_back(current_loop, &new_node);
      break;
    case ',':
      new_instruction = INPUT;
      node_assign(&new_node, NODE_INSTRUCTION, &new_instruction);
      vec_push_back(current_loop, &new_node);
      break;
    case '[':
      vec_init(&new_loop, VEC_NODE);
      vec_push_back(&loop_stack, &new_loop);
      break;
    case ']':
      node_assign(&new_node, NODE_VEC, vec_pop_back(&loop_stack));
      if (loop_stack.size == 1) {
        vec_push_back(&this->nodes, &new_node);
      } else {
        vec_push_back(vec_back(&loop_stack), &new_node);
      }
      break;
    }
  }
}
void interpreter_init(Interpreter *this) {
  vec_init(&this->memory, VEC_CHAR);
  char initial_value = 0;
  vec_push_back(&this->memory, &initial_value);
  this->index = 0;
}
void interpreter_execute(Interpreter *this, Code *code) {
  interpreter_execute_recursive_p(this, &code->nodes);
}
void interpreter_execute_recursive_p(Interpreter *this, Vec *nodes) {
  for (size_t i = 0; i < nodes->size; i++) {
    Node current_node = *(Node *)vec_at(nodes, i);
    switch (current_node.type) {
    case NODE_INSTRUCTION:
      switch (*(enum Instruction *)current_node.value) {
      case INCL_POINTER:
        this->index++;
        if (this->memory.size == this->index) {
          char new_value = 0;
          vec_push_back(&this->memory, &new_value);
        }
        break;
      case DECL_POINTER:
        this->index--;
        break;
      case INCL_VALUE:
        (*(char *)vec_at(&this->memory, this->index))++;
        break;
      case DECL_VALUE:
        (*(char *)vec_at(&this->memory, this->index))--;
        break;
      case OUTPUT:
        putchar(*(char *)vec_at(&this->memory, this->index));
        break;
      case INPUT:
        *(char *)vec_at(&this->memory, this->index) = getchar();
        break;
      }
      break;
    case NODE_VEC:
      interpreter_execute_recursive_p(this, current_node.value);
      break;
    }
  }
}
