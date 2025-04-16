# Lingo Lang
An esoteric mini-lang for very basic LSystem stuff

## Syntax
A `lingo` file looks something like this:
```python
def System [
    (A) -> (BA)
    (B) -> (AA)
]

def Actions [
    (A) >> {print -(0)}
    (B) >> {print -(1)}
]

print -expand -System -(ABB)

do_all -scan -Actions -(ABB)
```

To create a variable, use `def <varname> <value>`. A valid value is an int (eg. `100`), a boolean (eg. `true`), or a string (eg. `(ABC)`).

Similarly for functions, use `def <funcname> <func>`. You can create a function by using an anonymous function `{<body>}`. To specify arguments, you can either put them after `<funcname>` or inside the anonymous function `{<args> <body>}`.

Arguments are broken into two kinds: values and functions. If an argument is a value, it is specified with `-<arg>`, and if it is a function it is specified with `:<arg>`.

To specify a string (for an LSystem), use `(<string>)`.

To create an array, use `[<item> <item> <...>]`

To import another `ll` file, you can use `import <path>`. Then, you can refer to things from that file. For example:
```py
# a.fib.lingo
def System [
    (A) -> (AB)
    (B) -> (A)
]

def do_n :f -n { loop :f n }
# x.lingo
import a/fib

fib/do_n :{print -(Hello)} -5
```
You can do an `import file/*`.

There are a very few operators:
- `->` between two strings, creates a rule
- `>>` between a value and anything, creates a map entry
- `.` between two strings or arrays, concatenates them
- `-` to signify a value argument
- `:` to signify a function argument
- `/` between a parent and child in a path

# Standard Library
The language is already minimal, so it doesn't really have much in the standard library. In the core library, accessible by default, the following are provided
- Io
    - `print` - writes to stdout
    - `input` - reads from stdin
    - `report` - writes to stderr 
- LSystem
    - `expand` - applies an LSystem to an LString
    - `scan` - goes through an LString and performs an action
    - `len` - gets the lentgh of an LString
- Turtle 
    - `draw` - draws a string using the given rules
    - `save` - saves the given states (in order: position, rotation, color(if i end up supporting color))
    -  `recall` - recalls the state at the top of the state stack
-  Math
    - `add` - adds two numbers
    - `sub` - subtracts two numbers
    - `mul` - multiplies two numbers
    - `div` - divides two numbers, rounding down
    - `exp` - exponentiates two numbers, rounding down
    - `shl` - binary shift left
    - `shr` - binary shift right
    - `and` - boolean and
    - `or` - boolean or
    - `xor` - boolean xor
    - `not` - boolean not
