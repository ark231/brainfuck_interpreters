#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define failed(msg)                                                            \
  fprintf(stderr, "%s\n", (msg));                                              \
  exit(EXIT_FAILURE)
typedef struct {
  fpos_t *memory;
  size_t index;
  size_t size;
} stack;
void init_stack(stack *dst);
void push(stack *dst, fpos_t item);
fpos_t get_top_of(const stack *src);
void pop(stack *src);
int main(int argc, char **argv) {
  if (argc != 2) {
    failed("the number of argument must be 1");
  }
  FILE *infile;
  if ((infile = fopen(argv[1], "r")) == NULL) {
    failed("failed to open source file");
  }
  char instruction = '\0';
#define INITIAL_MEMSIZE 1
  char *memory = calloc(INITIAL_MEMSIZE, sizeof(char));
  size_t index = 0;
  size_t size = INITIAL_MEMSIZE;
  stack loop_stack;
  init_stack(&loop_stack);
  if (memory == NULL) {
    failed("failed to allocate memory");
  }
  while ((instruction = fgetc(infile)) != EOF) {
    switch (instruction) {
    case '>':
      if (index == size - 1) {
        size_t old_size = size;
        char *new_memory = realloc(memory, size *= 2);
        if (new_memory == NULL) {
          free(memory);
          failed("failed to allocate memory");
        }
        memset(new_memory + old_size, 0, (size - old_size));
        memory = new_memory;
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
      putchar(memory[index]);
      break;
    case ',':
      memory[index] = getchar();
      break;
    case '[':
      if (memory[index] != 0) {
        fpos_t pos;
        fgetpos(infile, &pos);
        push(&loop_stack, pos);
      } else {
        int depth = 1;
        for (;;) {
          char tmp_instruction = fgetc(infile);
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
        fpos_t pos = get_top_of(&loop_stack);
        fsetpos(infile, &pos);
      } else {
        pop(&loop_stack);
      }
      break;
    }
  }
  free(memory);
  return 0;
}
void init_stack(stack *dst) {
#define INITIAL_STACK_DEPTH 1
  assert(dst != NULL);
  dst->memory = malloc(sizeof(fpos_t) * INITIAL_STACK_DEPTH);
  dst->size = INITIAL_STACK_DEPTH;
  dst->index = 0;
  if (dst->memory == NULL) {
    failed("failed to allocate memory");
  }
}
void push(stack *dst, fpos_t item) {
  assert(dst != NULL);
  if (dst->index == dst->size) {
    fpos_t *new_memory =
        realloc(dst->memory, sizeof(fpos_t) * (dst->size *= 2));
    if (new_memory == NULL) {
      free(dst->memory);
      failed("failed to allocate memory");
    }
    dst->memory = new_memory;
  }
  dst->memory[dst->index] = item;
  dst->index++;
}
fpos_t get_top_of(const stack *src) {
  if (src->index == 0) {
    failed("tried to pop from empty stack");
  }
  return src->memory[src->index - 1];
}
void pop(stack *src) {
  assert(src != NULL);
  if (src->index == 0) {
    failed("tried to pop from empty stack");
  }
  src->index--;
}
