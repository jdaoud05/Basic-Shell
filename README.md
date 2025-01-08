# Mini-Shell

## Overview
Mini-Shell is a custom command-line shell implemented in C, providing basic shell functionalities such as command execution, input/output redirection, pipes, and built-in commands. It also supports executing scripts, re-running the previous command, and handling multiple commands separated by semicolons.

## Features
- Execute standard shell commands.
- Built-in commands:
  - `cd [path]`: Change the current directory to the specified `path`.
  - `source [file]`: Execute commands from a script file.
  - `prev`: Re-execute the previous command.
  - `help`: Display help information for built-in commands.
  - `exit`: Exit the shell.
- Input/output redirection using `<` and `>`.
- Pipe support for commands separated by `|`.
- Multi-command support using `;`.

## File Structure
- **`shell.c`**: Main shell implementation, including command parsing, built-in command handling, and pipe/redirection logic.
- **`tokenize.c`**: Reads user input and tokenizes it into commands.
- **`tokens.c`**: Implements tokenization logic for commands, handling special characters and quoted strings.
