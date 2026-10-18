<div align="center">

# 🔨 Hammer Script

**A modern, expressive programming language with human-first diagnostics, native standalone compilation, and a comprehensive engine standard library.**

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/firefoxplaye25/Hammer-Script/releases/tag/v1.0.0)
[![Platform](https://img.shields.io/badge/platform-Windows%20x64-brightgreen.svg)]()
[![Compiler](https://img.shields.io/badge/compiler-HamHook%20Standalone%20PE-orange.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/badge/download-SDK%20%26%20Extension-purple.svg)](https://github.com/firefoxplaye25/Hammer-Script/releases/tag/v1.0.0)

</div>

---

## 📑 Table of Contents

- [1. Overview & Key Philosophy](#1-overview--key-philosophy)
- [2. Official Downloads & Packages](#2-official-downloads--packages)
- [3. CLI Tooling & Quick Start](#3-cli-tooling--quick-start)
- [4. Complete Language Specification](#4-complete-language-specification)
  - [4.1 Variables, Typing & Assignments](#41-variables-typing--assignments)
  - [4.2 Console I/O & User Input](#42-console-io--user-input)
  - [4.3 String Interpolation](#43-string-interpolation)
  - [4.4 Operators & Natural English Expressions](#44-operators--natural-english-expressions)
  - [4.5 Control Flow (If / Elseif / Else)](#45-control-flow-if--elseif--else)
  - [4.6 Loops (Repeat, Range, For Each, While)](#46-loops-repeat-range-for-each-while)
  - [4.7 Actions (Functions & Closures)](#47-actions-functions--closures)
  - [4.8 Lists, Maps & Natural Slicing](#48-lists-maps--natural-slicing)
  - [4.9 Robust Error Handling (`attempt`)](#49-robust-error-handling-attempt)
  - [4.10 Comments & Block Notes](#410-comments--block-notes)
  - [4.11 Multi-file Imports & Includes](#411-multi-file-imports--includes)
- [5. Standard Library Built-in Functions](#5-standard-library-built-in-functions)
  - [5.1 Strings](#51-strings)
  - [5.2 Lists](#52-lists)
  - [5.3 Maps / Dictionaries](#53-maps--dictionaries)
  - [5.4 Mathematics & Randomness](#54-mathematics--randomness)
  - [5.5 Type Conversions & Timing](#55-type-conversions--timing)
  - [5.6 Native JSON Processing](#56-native-json-processing)
- [6. Native Engine Subsystems](#6-native-engine-subsystems)
  - [6.1 `asock` / `hmhookwww` (HTTP & TCP Sockets)](#61-asock--hmhookwww-http--tcp-sockets)
  - [6.2 `hamfs` (Disk & Filesystem)](#62-hamfs-disk--filesystem)
  - [6.3 `hmripa` (Cryptographic Security & Hashing)](#63-hmripa-cryptographic-security--hashing)
  - [6.4 `hamsys` (OS, Hardware, Audio & Clipboard)](#64-hamsys-os-hardware-audio--clipboard)
  - [6.5 `hmshell` (Terminal & Shell Control)](#65-hmshell-terminal--shell-control)
  - [6.6 `hmcanvas` (2D Console Graphics & Buffer)](#66-hmcanvas-2d-console-graphics--buffer)
- [7. Community Library System & Extensibility](#7-community-library-system--extensibility)
- [8. HamHook: Dedicated Standalone PE Compiler](#8-hamhook-dedicated-standalone-pe-compiler)
- [9. Visual Studio Code & Antigravity IDE Extension](#9-visual-studio-code--antigravity-ide-extension)
- [10. The Empathetic Diagnostic Engine](#10-the-empathetic-diagnostic-engine)
- [11. Building from Source](#11-building-from-source)
- [12. License](#12-license)

---

## 1. Overview & Key Philosophy

Hammer Script (`.hmscp`) was designed from the ground up to eliminate the friction, confusing traps, and heavy runtime dependencies found in traditional scripting languages:

| Aspect | Traditional Scripting (e.g. Python) | Hammer Script 🔨 |
| :--- | :--- | :--- |
| **Block Delimiters** | Invisible whitespace *(IndentationError traps)* | Clean, explicit `end` blocks |
| **Type Coercion** | `"Age: " + str(age)` *(Crashes without `str`)* | `"Age: " + age` *(Automatic seamless conversion)* |
| **Looping** | `for _ in range(5):` | `repeat 5 times ... end` |
| **Comparisons** | `x >= 18 and x <= 65` | `x is at least 18 and x is at most 65` |
| **Compilation** | Requires PyInstaller / 500 MB toolchains | **HamHook Standalone PE** in ~45 ms with 0 external tools |
| **Diagnostics** | `SyntaxError: invalid syntax` | Exact code pointer + Cause + Solution + Code Example |
| **Built-in Capabilities** | Requires installing `requests`, `cryptography`, etc. | Sockets, HTTP, FileSystem, Crypto, Audio, Canvas all built-in |

---

## 2. Official Downloads & Packages

Pre-built binaries and editor packages are available in our [**GitHub Releases**](https://github.com/firefoxplaye25/Hammer-Script/releases/tag/v1.0.0):

- 📦 **Hammer SDK (`hammer-sdk-v1.0.0.zip`)**: Standalone CLI (`hammer.exe`), Dedicated Native Compiler (`hamhook.exe`), C++ Transpiler (`hamc.exe`), engine headers (`.lib`), standard library, and documentation.
- 🧩 **VS Code Extension (`hammer-script-lang-1.0.0.vsix`)**: Syntax highlighting, token color customizations, bracket matching, and snippet support.
- 📁 **Extension Source Archive (`hammer-script-vscode-extension.zip`)**: Portable extension folder for manual installation into `~/.vscode/extensions/`.

---

## 3. CLI Tooling & Quick Start

Hammer Script provides a comprehensive CLI suite:

```powershell
# Run a script directly through the interpreter
hammer run examples\01_hello.hmscp

# Compile to a native Windows PE .exe (Zero external dependencies)
hammer hamhook examples\01_hello.hmscp -o my_app.exe
# (or invoke hamhook directly):
hamhook examples\01_hello.hmscp -o my_app.exe

# Compile without console window (Silent GUI Mode)
hammer hamhook my_gui.hmscp --no-console -o my_gui.exe

# Check syntax and run diagnostic analyzer
hammer check script.hmscp

# Interactive REPL shell
hammer repl

# Scaffold a new Hammer Script project
hammer new my_project

# Manage community libraries
hammer create-lib my_module --author developer_name
hammer inst hmx --author user package_name
hammer inst hmx --monolithiiun official_package
hammer libs
hammer remove-lib package_name
```

---

## 4. Complete Language Specification

### 4.1 Variables, Typing & Assignments
Hammer Script features dynamic typing with automatic memory management. Variables are declared using `set <name> to <value>` or the direct assignment `<name> = <value>`:

```hammer
set player_name to "Arthur"
set score to 1250
set health to 98.5
set is_active to true
set weapons to ["Sword", "Crossbow", "Shield"]
set profile to {
    "username": "arthur_king",
    "level": 15,
    "guild": "Knights"
}
```

Indexed assignments on lists and maps:
```hammer
set weapons[0] to "Excalibur"
set profile["level"] to 16
```

### 4.2 Console I/O & User Input
Output values using `show` (or `print`). To prompt users for interactive input, use `ask`:

```hammer
show "Welcome to Hammer Script!"
show "Current score: " + score

# Asking user for console input:
set username to ask "Enter your username: "
show "Hello, " + username + "!"
```

### 4.3 String Interpolation
Expressions placed inside `{}` are evaluated and formatted inline:

```hammer
set user to "Elena"
set level to 40
set exp to 350
show "Hero {user} is Level {level} with {exp} XP (Next level in {500 - exp} XP)."
```

### 4.4 Operators & Natural English Expressions

#### Arithmetic Operators
| Operator | Description | Example |
| :---: | :--- | :--- |
| `+` | Addition (or string concatenation) | `10 + 5` / `"Hi " + name` |
| `-` | Subtraction | `20 - 4` |
| `*` | Multiplication | `6 * 7` |
| `/` | Division | `100 / 4` |
| `%` | Modulo (Remainder) | `17 % 5` |
| `^` | Exponentiation | `2 ^ 8` |

#### Comparison & Logical Operators
Both standard symbols and expressive English keywords are supported:

| English Keyword | Symbol | Description |
| :--- | :---: | :--- |
| `is` | `==` | Equality test |
| `is not` | `!=` | Inequality test |
| `is greater than` | `>` | Greater than |
| `is less than` | `<` | Less than |
| `is at least` | `>=` | Greater than or equal to |
| `is at most` | `<=` | Less than or equal to |
| `and` | - | Logical AND |
| `or` | - | Logical OR |
| `not` | - | Logical NOT |

```hammer
if score is at least 1000 and player_name is not "Guest" then
    show "Elite Player Verified"
end
```

### 4.5 Control Flow (If / Elseif / Else)
```hammer
set temperature to 28

if temperature is greater than 35 then
    show "Heatwave warning!"
elseif temperature is at least 20 then
    show "Pleasant weather."
elseif temperature is at least 10 then
    show "Chilly day."
else
    show "Freezing cold!"
end
```

### 4.6 Loops (Repeat, Range, For Each, While)

#### Fixed Count Loop
```hammer
repeat 5 times
    show "Fast iteration!"
end
```

#### Range Loop with Variable & Step
```hammer
# Ascending
repeat with i from 1 to 5
    show "Number: " + i
end

# Custom step (e.g., even numbers)
repeat with n from 0 to 10 step 2
    show "Even: " + n
end
```

#### Collection Iteration (`for each`)
Iterate over lists or strings character-by-character:
```hammer
set inventory to ["Potion", "Elixir", "Scroll"]
for each item in inventory
    show "Item in bag: " + item
end
```

#### While Loop
```hammer
set countdown to 3
while countdown is greater than 0
    show countdown + "..."
    set countdown to countdown - 1
end
show "Blast off!"
```

*Loop controls:* Use `stop` (or `break`) to exit a loop early, and `skip` (or `continue`) to jump to the next iteration.

### 4.7 Actions (Functions & Closures)
Functions in Hammer Script are called **actions** and return values with `give` (or `return`):

```hammer
action calculate_damage(attack, defense, is_critical)
    set base_damage to attack - (defense / 2)
    if is_critical then
        set base_damage to base_damage * 2
    end
    give base_damage
end

set dmg to calculate_damage(50, 10, true)
show "Damage dealt: " + dmg
```

### 4.8 Lists, Maps & Natural Slicing

#### Lists (Dynamic Arrays)
```hammer
set list to [10, 20, 30]
push(list, 40)
show "First element: " + list[0]
show "Total elements: " + length(list)
```

#### Maps (Key-Value Dictionaries)
```hammer
set character to {
    "name": "Thor",
    "class": "Warrior",
    "stats": {"str": 18, "dex": 12}
}
show "Class: " + character["class"]
show "Strength: " + character["stats"]["str"]
```

#### Natural Slicing (Inclusive)
Slice lists and strings naturally using the `[start to end]` syntax:
```hammer
set language to "HammerScript"
show language[0 to 5]             # "Hammer"
show language[6 to 11]            # "Script"

set ranks to ["Novice", "Adept", "Master", "Grandmaster", "Legend"]
show ranks[1 to 3]                # ["Adept", "Master", "Grandmaster"]
```

### 4.9 Robust Error Handling (`attempt`)
Trap runtime exceptions safely without crashing:

```hammer
attempt:
    set content to read_file("missing_config.json")
    show content
on error failure:
    show "Could not read configuration: " + failure
end
```

### 4.10 Comments & Block Notes
```hammer
# Single line comment style 1
// Single line comment style 2

note
    Multi-line documentation block.
    Everything written here is completely ignored
    by the compiler and parser.
end note
```

### 4.11 Multi-file Imports & Includes
Import other scripts or installed libraries using clean, unquoted syntax or path strings:

```hammer
use ansi_colors
import matrix_math
include "helpers.hmscp"
```

---

## 5. Standard Library Built-in Functions

### 5.1 Strings
| Function | Signature | Return Type | Description |
| :--- | :--- | :---: | :--- |
| `length(str)` / `len(str)` | `(str)` | `Number` | Returns number of characters in string |
| `split(str, delim)` | `(str, str)` | `List` | Splits string by delimiter |
| `join(list, delim)` | `(list, str)` | `String` | Joins list elements into single string |
| `trim(str)` | `(str)` | `String` | Trims leading and trailing whitespace |
| `replace(str, target, repl)` | `(str, str, str)` | `String` | Replaces all occurrences of target |
| `to_upper(str)` | `(str)` | `String` | Converts string to uppercase |
| `to_lower(str)` | `(str)` | `String` | Converts string to lowercase |
| `contains(str, substr)` | `(str, str)` | `Boolean` | Checks if substring exists |
| `starts_with(str, prefix)` | `(str, str)` | `Boolean` | Checks if string begins with prefix |
| `ends_with(str, suffix)` | `(str, str)` | `Boolean` | Checks if string terminates with suffix |

### 5.2 Lists
| Function | Signature | Return Type | Description |
| :--- | :--- | :---: | :--- |
| `length(list)` / `len(list)` | `(list)` | `Number` | Returns number of elements |
| `push(list, item)` / `append` | `(list, any)` | `Nothing` | Appends item to end of list |
| `pop(list)` | `(list)` | `Any` | Removes and returns last item |
| `insert(list, index, item)` | `(list, num, any)` | `Nothing` | Inserts element at specified index |
| `remove_at(list, index)` | `(list, num)` | `Any` | Removes and returns element at index |
| `sort(list)` | `(list)` | `Nothing` | Sorts list in-place (ascending) |
| `reverse(list)` | `(list)` | `Nothing` | Reverses list in-place |

### 5.3 Maps / Dictionaries
| Function | Signature | Return Type | Description |
| :--- | :--- | :---: | :--- |
| `keys(map)` | `(map)` | `List` | Returns list of all keys in map |
| `values(map)` | `(map)` | `List` | Returns list of all values in map |
| `has_key(map, key)` | `(map, str)` | `Boolean` | Checks if map contains key |

### 5.4 Mathematics & Randomness
| Function | Signature | Return Type | Description |
| :--- | :--- | :---: | :--- |
| `round(num)` | `(num)` | `Number` | Rounds to nearest whole integer |
| `floor(num)` | `(num)` | `Number` | Rounds down to largest previous integer |
| `ceil(num)` | `(num)` | `Number` | Rounds up to smallest following integer |
| `abs(num)` | `(num)` | `Number` | Returns absolute value |
| `sqrt(num)` | `(num)` | `Number` | Calculates square root |
| `random(min, max)` | `(num, num)` | `Number` | Generates random integer between min and max |

### 5.5 Type Conversions & Timing
| Function | Signature | Return Type | Description |
| :--- | :--- | :---: | :--- |
| `type_of(val)` | `(any)` | `String` | Returns type name (`"number"`, `"string"`, `"list"`, etc.) |
| `to_number(val)` | `(any)` | `Number` | Converts string or boolean to number |
| `to_string(val)` | `(any)` | `String` | Converts value to string representation |
| `time()` | `()` | `Number` | Returns current Unix timestamp in seconds |
| `sleep(ms)` | `(num)` | `Nothing` | Pauses execution for specified milliseconds |

### 5.6 Native JSON Processing
Hammer Script handles JSON conversions seamlessly with native Maps and Lists:

```hammer
set raw_json to '{"server": "us-east", "port": 8080, "tags": ["prod", "secure"]}'
set cfg to json_parse(raw_json)

show "Server: " + cfg["server"]
show "Primary tag: " + cfg["tags"][0]

set payload to json_encode({
    "status": "online",
    "uptime_hours": 128
})
```

---

## 6. Native Engine Subsystems

### 6.1 `asock` / `hmhookwww` (HTTP & TCP Sockets)
High-performance networking subsystem for web requests and raw TCP communication:

```hammer
# HTTP GET request
set res to asock_get("https://httpbin.org/get")
show "HTTP Status: " + res["status"]
show "Body: " + res["body"]

# HTTP POST request with custom headers
set headers to {"Content-Type": "application/json", "Authorization": "Bearer Token"}
set postRes to asock_post("https://httpbin.org/post", '{"message": "Hello"}', headers)

# Stream download directly to file on disk
asock_download("https://example.com/file.zip", "local_file.zip")

# Low-level TCP Socket connection
set sock to asock_connect("echo.websocket.events", 80)
asock_send(sock, "GET / HTTP/1.1\r\nHost: echo.websocket.events\r\n\r\n")
set response to asock_recv(sock, 2048)
asock_close(sock)

# Smart helper (GET or POST if body is provided):
set data to fetch("https://api.github.com")
```
*Aliases:* All `asock_*` functions are also accessible under `hmhookwww_*` and object namespaces `asock["get"](...)`.

### 6.2 `hamfs` (Disk & Filesystem)
Cross-platform file and directory management:

```hammer
# Directory operations
hamfs_mkdir("backups")
hamfs_mkdir("backups/daily")

# File read / write / append
hamfs_write("backups/daily/log.txt", "System initialized.")
hamfs_append("backups/daily/log.txt", "\nBackup completed successfully.")

set content to hamfs_read("backups/daily/log.txt")
show "Log contents: " + content

# Metadata queries
show "Size: " + hamfs_size("backups/daily/log.txt") + " bytes"
show "Is directory? " + hamfs_is_dir("backups")
show "Is file? " + hamfs_is_file("backups/daily/log.txt")
show "Files: " + hamfs_list("backups/daily")

# File operations
hamfs_copy("backups/daily/log.txt", "backups/archive.txt")
hamfs_delete("backups/daily/log.txt")
```
*Aliases:* Standard aliases `read_file`, `write_file`, `delete_file`, `file_exists`, etc. are fully supported.

### 6.3 `hmripa` (Cryptographic Security & Hashing)
Symmetric cryptographic engine featuring key integrity validation and 64-bit hashing:

```hammer
set password to "SuperSecretData123"
set key to "MasterKey2026"

# Encrypt string to hex representation
set encrypted_hex to hmripa_encrypt(password, key)
show "Encrypted: " + encrypted_hex

# Decrypt string back to plaintext
set decrypted to hmripa_decrypt(encrypted_hex, key)
show "Decrypted: " + decrypted

# Encrypt file directly on disk with validation
hmripa_encrypt_file("credentials.json", "credentials.enc", key)

# Decrypting with wrong key returns false and protects data:
set failed to hmripa_decrypt_file("credentials.enc", "output.json", "WrongKey")
# Decrypting with correct key restores original file:
set success to hmripa_decrypt_file("credentials.enc", "output.json", key)

# Fast cryptographic hash
show "Hash: " + hmripa_hash("Hammer Script Source")
```

### 6.4 `hamsys` (OS, Hardware, Audio & Clipboard)
Direct access to operating system metrics, hardware status, audio, and clipboard:

```hammer
# System Information
show "OS: " + hamsys_os()
show "CPU Cores: " + hamsys_cpu_count()
show "Username: " + hamsys_user()
show "Hostname: " + hamsys_hostname()
show "RAM: " + hamsys_memory()["available_mb"] + " MB free"

# Date & Time Strings
show "Current Date: " + hamsys_date()         # "YYYY-MM-DD"
show "Current Time: " + hamsys_time_str()     # "HH:MM:SS"
show "Date & Time: " + hamsys_datetime()      # "YYYY-MM-DD HH:MM:SS"
show "System Uptime: " + hamsys_uptime() + "s"

# Clipboard manipulation
hamsys_clipboard_set("Copied from Hammer Script!")
show "Clipboard holds: " + hamsys_clipboard_get()

# Audio & Alerts
hamsys_beep(750, 250)                         # 750 Hz for 250ms
hamsys_system_sound("ok")                     # "ok", "error", "warning", "info"
hamsys_alert("Application Notice", "Task completed!") # Native Windows dialog
```

### 6.5 `hmshell` (Terminal & Shell Control)
```hammer
hmshell_title("Hammer Engine Terminal v1.0")
hmshell_cursor(12, 4)                         # Move cursor to X=12, Y=4
hmshell_color(11)                             # Light Cyan text attribute
hmshell_clear()                               # Clear console screen
set output to hmshell_exec("cmd /c dir")      # Execute shell command
```

### 6.6 `hmcanvas` (2D Console Graphics & Buffer)
High-performance character matrix for console games, UI borders, and ASCII rendering:

```hammer
# Initialize a 40x12 character matrix
hmcanvas_init(40, 12)
hmcanvas_clear(" ")

# Draw border boxes
hmcanvas_box(0, 0, 40, 12, "#")
hmcanvas_box(2, 1, 36, 4, "=")

# Render centered text
hmcanvas_text(4, 2, "HAMMER SCRIPT 2D ENGINE")
hmcanvas_text(8, 3, "Console Buffer Subsystem")

# Draw game elements
hmcanvas_set(5, 7, "*")
hmcanvas_text(7, 7, "Player 1 (Active)")

# Flush buffer to console
hmcanvas_render()
```

---

## 7. Community Library System & Extensibility

Hammer Script features a modular package management system designed to **add new functions directly to the language**:

### 1. Creating a New Library Module
```powershell
hammer create-lib game_utils --author developer_name
```
This scaffolds:
- `manifest.json`: Version, package metadata, and entry point.
- `game_utils.lib`: C-style function declarations and API signatures.
- `game_utils.hmscp`: Function implementations using `action`.

### 2. Auto-Loading Functions into the Language
Whenever a library is placed in `lib loader/community/` or `lib loader/monolithiiun/`, its functions are **automatically loaded into the global scope on startup**:
```hammer
# Functions are available without explicit includes:
show game_utils_hello("Admin")

# Or explicitly import using clean syntax:
use game_utils
import matrix_math
```

### 3. Package Management Commands
```powershell
hammer inst hmx --author <author> <pkg>   # Install community package
hammer inst hmx --monolithiiun <pkg>     # Install verified monolithic package
hammer libs                              # List installed packages
hammer remove-lib <pkg>                  # Uninstall package
```

---

## 8. HamHook: Dedicated Standalone PE Compiler

Hammer Script includes **HamHook**, a dedicated native compiler built directly into the engine:

```powershell
# Compile to native standalone .exe (Runs anywhere on Windows with 0 dependencies)
hammer hamhook script.hmscp -o app.exe

# Compile silent windowed GUI (Hides console window via PE Subsystem 2)
hammer hamhook gui_script.hmscp --no-console -o app.exe
```

### Why HamHook is Unique:
- **Zero Toolchain Dependencies:** Does not search for or require `g++`, `clang`, MSYS2, or Visual Studio on the user's computer.
- **Blazing Fast Compilation:** Packages scripts and binary stubs in **~45 ms**.
- **Self-Contained Portability:** Automatically bundles all installed community modules (`lib loader/`) directly into the `.exe` payload. The generated binary runs out-of-the-box on any other PC.

---

## 9. Visual Studio Code & Antigravity IDE Extension

The official Hammer Script language extension provides syntax highlighting, automatic indentation, bracket matching, and token coloring.

### Installation:
1. Download [`hammer-script-lang-1.0.0.vsix`](https://github.com/firefoxplaye25/Hammer-Script/releases/download/v1.0.0/hammer-script-lang-1.0.0.vsix).
2. Open VS Code or Antigravity IDE.
3. Open the Extensions view (`Ctrl + Shift + X`), click the `...` menu, and select **"Install from VSIX..."**.

---

## 10. The Empathetic Diagnostic Engine

Hammer Script features an educational, pair-programming style diagnostic analyzer:

```text
================================================================================
[Hammer Script Syntax Error] Missing 'to' in variable assignment.
--> my_script.hmscp:16:12
    |
 16 | set health 100
    |            ^--- Here is where the error occurred
    |
[Why did this happen?]
  You wrote 'set health ...' without the keyword 'to'.
  Hammer Script uses natural English grammar: 'set <variable> to <value>'.

[How to fix this]
  Insert 'to' between the variable name and the value.

[Correct Example]
  set health to 100
  # or simply:
  health = 100
================================================================================
```

---

## 11. Building from Source

To compile the Hammer Script SDK from source code (requires C++17 with MSYS2 GCC):

```powershell
git clone https://github.com/firefoxplaye25/Hammer-Script.git
cd "Hammer-Script"
.\build.bat
```
The automated build script compiles `hammer.exe`, copies standalone tools `hamhook.exe` and `hamc.exe`, and packages the public distribution into `Build\hammer-sdk\`.

---

## 12. License

Hammer Script is licensed under the permissive [MIT License](LICENSE).
You are free to use, distribute, modify, and build commercial or personal applications with Hammer Script.
