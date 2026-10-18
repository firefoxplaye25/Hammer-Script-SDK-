# Hammer Script Language Documentation & Guide

## Overview
Hammer Script is an expressive, high-performance scripting language with dedicated standalone PE compilation (HamHook), native internet networking (asock / hmhookwww), cryptographic security (hmripa), operating system integrations (hamsys), terminal control (hmshell), 2D graphics canvas (hmcanvas), and a modular community library system (LibLoader).

## CLI Quick Reference
```powershell
hammer run <file.hmscp>                 # Run script via interpreter
hammer hamhook <file.hmscp> [-o app.exe]# Compile to native standalone PE executable (0 external tools)
hammer build <file.hmscp> [-o app.exe]  # Compile with default compiler module
hammer inst hmx --author <author> <pkg> # Install community package
hammer inst hmx --monolithiiun <pkg>    # Install verified monolithic package
hammer create-lib <name> [--author me]  # Create a new module extending the language
hammer libs                             # List installed packages
hammer remove-lib <name>                # Uninstall package
hammer check <file.hmscp>               # Syntax and diagnostic check
hammer repl                             # Interactive shell
```

## Syntax Highlights
- Variable declaration: `set x to 10`
- Print output: `show "Hello World"`
- Functions:
  ```hammer
  action add(a, b)
      give a + b
  end
  ```
- Imports:
  ```hammer
  use my_module
  import matrix_math
  include "ansi_colors"
  ```
- Loops:
  ```hammer
  repeat 5 times
      show "Iteration"
  end

  repeat with i from 1 to 10
      show i
  end

  for each item in [1, 2, 3]
      show item
  end
  ```
- Error Handling:
  ```hammer
  attempt:
      show 10 / 0
  on error err:
      show "Error occurred: " + err
  end
  ```
- Block Comments:
  ```hammer
  note
      multi-line notes here
  end note
  ```
