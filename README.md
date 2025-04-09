# Lingo Lang
An esoteric mini-lang for very basic LSystem stuff

## Syntax
An `ll` file looks something like this:
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
# a.fib.ll
def System [
    (A) -> (AB)
    (B) -> (A)
]

def do_n :f -n { loop :f n }
# x.ll
import a/fib

fib/do_n :{print -(Hello)} -5
```
You can do an `import file/*`.

There are a very few operators:
- `->` between two strings, creates a rule
- `>>` between a value and anything, creates a map entry
- `.` between two strings or array, concatenates them
