using OneOf;

if (args.Length < 1)
{
    TextWriter errorWriter = Console.Error;
    errorWriter.WriteLine("error: not enough arguments");
    Environment.Exit(1);
}
TextReader reader = new StreamReader(args[0]);
Stack<CodeBlock> codeStack = new();
codeStack.Push(new CodeBlock());
foreach (var character in reader.ReadToEnd())
{
    var instruction = InstructionFromChar(character);
    if (instruction.HasValue)
    {
        var top = codeStack.Pop();
        top.AddInstruction(instruction.Value);
        codeStack.Push(top);
    }
    else
    {
        switch (character)
        {
            case '[':
                codeStack.Push(new CodeBlock());
                break;
            case ']':
                var oldTop = codeStack.Pop();
                var newTop = codeStack.Pop();
                newTop.AddInnerblock(oldTop);
                codeStack.Push(newTop);
                break;
        }
    }
}
var code = codeStack.Pop();
Interpreter interpreter = new();
interpreter.execute(code);
static Instruction? InstructionFromChar(char c)
{
    switch (c)
    {
        case '+': return Instruction.VINC;
        case '-': return Instruction.VDEC;
        case '>': return Instruction.PINC;
        case '<': return Instruction.PDEC;
        case '.': return Instruction.PUTC;
        case ',': return Instruction.GETC;
        default: return null;
    }
}
enum Instruction
{
    VINC,
    VDEC,
    PINC,
    PDEC,
    GETC,
    PUTC,
}


class CodeBlock
{
    public List<OneOf<Instruction, CodeBlock>> instructions = new();
    public void AddInstruction(Instruction instruction) { instructions.Add(instruction); }
    public void AddInnerblock(CodeBlock innerblock) { instructions.Add(innerblock); }
}

class Interpreter
{
    List<byte> data;
    int head = 0;
    public Interpreter() { data = new List<byte> { 0 }; }
    public void execute(CodeBlock code)
    {
        foreach (var step in code.instructions)
        {
            int _ = step.Match(instruction => { execute_instruction(instruction); return 0; }, codeblock =>
            {
                while (data[head] != 0)
                {
                    execute(codeblock);
                }
                return 1;
            });
        }
    }
    private void execute_instruction(Instruction instruction)
    {
        switch (instruction)
        {
            case Instruction.VINC:
                data[head]++;
                break;
            case Instruction.VDEC:
                data[head]--;
                break;
            case Instruction.PINC:
                head++;
                if (head == data.Count())
                {
                    data.Add(0);
                }
                break;
            case Instruction.PDEC:
                head--;
                break;
            case Instruction.GETC:
                data[head] = (byte)Console.Read();
                break;
            case Instruction.PUTC:
                Console.Write((char)data[head]);
                break;
            default:
                break;
        }
    }
}
