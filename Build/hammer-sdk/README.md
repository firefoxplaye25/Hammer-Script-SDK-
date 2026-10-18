# 🔨 Hammer Script (`.hmscp`)

> **The friendly programming language designed to be simpler, clearer, and more intuitive than Python.**

Hammer Script was created with one goal in mind: **making programming effortless**. Instead of quirky rules, indentation errors, or cryptic syntax errors, Hammer Script uses natural English grammar, friendly block endings (`end`), and an **ultra-detailed diagnostic error engine** that tells you *why* something went wrong and *how* to fix it.

---

## 🌟 Why is Hammer Script easier than Python?

| Feature | Python 🐍 | Hammer Script 🔨 |
| :--- | :--- | :--- |
| **Output** | `print("Hello")` | `show "Hello"` or `print "Hello"` |
| **String + Number** | `"Score: " + str(score)` *(Crashes without `str()`!)* | `"Score: " + score` *(Seamlessly works!)* |
| **Variables** | `x = 10` | `set x to 10` or `x = 10` |
| **Simple Loops** | `for _ in range(5):` | `repeat 5 times ... end` |
| **Blocks** | Invisible whitespace *(IndentationError traps!)* | Explicit `end` blocks |
| **Comparisons** | `x >= 18 and x == 20` | `x is at least 18 and x is 20` |
| **User Input** | `name = input("Name: ")` | `set name to ask "Name: "` |
| **Compilation** | Bytecode `.pyc` | Compiles to native `.exe` using MSYS2 GCC/G++! |
| **Diagnostics** | `SyntaxError: invalid syntax` | Full visual code pointer + Cause + Fix + Code Example |

---

## 🚀 Quick Start

### 1. Run a script directly (Interpreter)
```powershell
.\bin\hammer.exe run examples\01_hello.hmscp
```

### 2. Compile to native Windows executable with HamHook (Primitive EXE Compiler)
```powershell
# Directly using the hamhook shortcut:
.\bin\hammer.exe hamhook examples\01_hello.hmscp -o bin\hello.exe

# Or via build with module selection:
.\bin\hammer.exe build --module hamhook examples\01_hello.hmscp -o bin\hello.exe

# Run the compiled native binary:
.\bin\hello.exe
```

### 3. List available compilation modules
```powershell
.\bin\hammer.exe modules
```
- **`hamhook`**: *Primitive EXE Compiler* - Produces standalone, optimized PE `.exe` with static runtime linking via MSYS2 GCC/G++.
- **`hamc`**: *C++ Source Transpiler* - Generates clean, modern C++17 code for auditing or manual compilation.

### 4. Check for errors and get suggestions
```powershell
.\bin\hammer.exe check examples\08_error_demo.hmscp
```

### 5. Interactive REPL Shell
```powershell
.\bin\hammer.exe repl
```

---

## 🔍 The Ultra-Detailed Error Engine

When you make a mistake in Hammer Script, the compiler doesn't just crash. It acts like a patient pair-programming mentor:

```text
================================================================================
[Hammer Script Syntax Error] Missing 'to' in variable assignment.
--> examples/08_error_demo.hmscp:16:12
    |
 15 | # Notice the typo below: 'set health 100' instead of 'set health to 100'
 16 | set health 100
    |            ^--- Here is where the error occurred
    |
[Why did this happen?]
  You wrote 'set health ...' without the keyword 'to'. Hammer Script uses natural English grammar: 'set <variable> to <value>'.

[How to fix this]
  Insert 'to' between the variable name and the value, or use '=' without 'set'.

[Correct Example]
  set health to 100
  # or simply:
  health = 100
================================================================================
```

---

## 🎨 Antigravity IDE Syntax Highlighting

Hammer Script syntax highlighting is pre-installed in your Antigravity IDE extensions directory:
`~/.antigravity-ide/extensions/hammer-script-lang/`

All `.hmscp` files feature rich color coding for:
- 🟨 **Actions & Functions**: `action`, `greet()`
- 🟩 **Built-in SDK tools**: `random()`, `length()`, `push()`
- 🟪 **Control flow**: `if`, `then`, `else`, `repeat`, `times`, `for each`
- 🟦 **Keywords**: `set`, `to`, `show`, `ask`, `is at least`
- 🟧 **Strings**: `"Hello"`
- 🟩 **Comments**: `# Notes`

---

## 📖 Syntax Guide

### 1. Output & Input
```hammer
show "Hello World!"
set username to ask "What is your name? "
show "Welcome, " + username + "!"
```

### 2. Conditions
```hammer
set age to 21

if age is at least 18 then
    show "Adult access granted"
elseif age is greater than 12 then
    show "Teen pass"
else
    show "Child pass"
end
```

### 3. Loops
```hammer
# Repeat a set number of times
repeat 3 times
    show "Hammer is awesome!"
end

# Loop through lists
set tools to ["Hammer", "Drill", "Wrench"]
for each tool in tools
    show "Tool: " + tool
end

# While loops
set count to 5
while count is greater than 0
    show "Counting: " + count
    set count to count - 1
end
```

### 4. Actions (Functions)
```hammer
action add(a, b)
    give a + b
end

show "Result: " + add(10, 20)
```

### 5. Lists & Maps
```hammer
# Lists
set scores to [95, 88, 72]
push(scores, 100)
show "Total scores: " + length(scores)

# Maps (Dictionaries)
set config to {
    "theme": "Dark",
    "volume": 80
}
show "Current Theme: " + config["theme"]
```

### 6. String Interpolation
```hammer
set player to "Arthur"
set score to 1500
show "Welcome {player}! You have {score} points, next rank in {score + 500}."
```

### 7. Range Loops
```hammer
# Count up
repeat with i from 1 to 5
    show "Count: " + i
end

# With custom step
repeat with n from 0 to 10 step 2
    show "Even: " + n
end
```

### 8. Multi-file Modules (`include`)
```hammer
include "math_utils.hmscp"

show "Square of 8 is: " + square(8)
```

### 9. Safe Error Handling (`attempt`)
```hammer
attempt
    set data to read_file("config.txt")
on error problem
    show "Caught error gracefully: {problem}"
end
```

### 10. File System & OS Runtime (`hamfs`)
```hammer
# Create directories & write files
hamfs_make_dir("my_folder")
hamfs_write("my_folder/app.log", "Initiated engine...")
hamfs_append("my_folder/app.log", "\nSecond line added!")

# Read & check attributes
set logs to hamfs_read("my_folder/app.log")
show "File size: " + hamfs_size("my_folder/app.log") + " bytes"
show "Is directory? " + hamfs_is_folder("my_folder")
show "Directory items: " + hamfs_list("my_folder")

# Native OS command execution
set cmdResult to hamfs_exec("cmd /c dir")
show "OS Exit Code: " + cmdResult["exit_code"]

# Cleanup
hamfs_delete("my_folder")
```

### 11. High-Performance Internet & Web (`asock` & `hmhookwww`)
Hammer Script includes two interchangeable names for its networking subsystem: **`asock`** and **`hmhookwww`**, as well as the smart helper **`fetch()`**.

```hammer
# HTTP GET request
set response to asock_get("https://httpbin.org/get")
# or: set response to hmhookwww_get("https://httpbin.org/get")
# or: set response to fetch("https://httpbin.org/get")

show "Status Code: " + response["status"]
show "Payload: " + response["body"]

# HTTP POST request with payload and custom headers
set headers to {"Content-Type": "application/json", "Authorization": "Bearer MyToken123"}
set body to '{"query": "HammerScript", "action": "test"}'

set postRes to asock_post("https://httpbin.org/post", body, headers)
# or: set postRes to hmhookwww_post("https://httpbin.org/post", body, headers)
# or smart fetch: set postRes to fetch("https://httpbin.org/post", body, headers)

show "POST Status: " + postRes["status"]

# Stream download directly to file
asock_download("https://example.com/logo.png", "logo.png")
# or: hmhookwww_download("https://example.com/logo.png", "logo.png")
```

### 12. Primitive File & Data Encryptor / Decryptor (`hmripa`)
The **`hmripa`** engine provides symmetric file and data encryption/decryption with key integrity checks and keystream generation:

```hammer
# Encrypt and decrypt strings
set cipherHex to hmripa_encrypt("Secret password", "MyKey123")
show "Encrypted hex: " + cipherHex

set originalText to hmripa_decrypt(cipherHex, "MyKey123")
show "Decrypted text: " + originalText

# Encrypt and decrypt files with key validation
hmripa_encrypt_file("credentials.txt", "locked.enc", "StrongSecretKey")

# Decrypting with wrong key safely returns false:
set wrongAttempt to hmripa_decrypt_file("locked.enc", "wrong.txt", "BadKey")
# Decrypting with the right key restores the original file:
set success to hmripa_decrypt_file("locked.enc", "restored.txt", "StrongSecretKey")

# Compute 64-bit cryptographic hash
show "Data Hash: " + hmripa_hash("Hammer Script")
```

### 13. Text & List Manipulation (Stdlib)
```hammer
# Strings
set greeting to "  Hello Hammer World!  "
show trim(greeting)                         # "Hello Hammer World!"
show to_upper("hammer")                     # "HAMMER"
show to_lower("HAMMER")                     # "hammer"
show replace("Hello World", "World", "Hammer") # "Hello Hammer"
show contains("HammerScript", "Script")     # true
show starts_with("HammerScript", "Hammer")  # true
show ends_with("HammerScript", "Script")    # true

# Splitting & Joining
set parts to split("apple,banana,orange", ",")
show join(parts, " -> ")                    # "apple -> banana -> orange"

# List Operations
set nums to [40, 10, 50, 20, 30]
sort(nums)                                  # [10, 20, 30, 40, 50]
reverse(nums)                               # [50, 40, 30, 20, 10]
set last to pop(nums)                       # removes and returns 10
insert(nums, 1, 99)                         # inserts 99 at index 1
remove_at(nums, 2)                          # removes element at index 2

# Natural Slicing (Inclusive)
set list to ["a", "b", "c", "d", "e", "f"]
show list[1 to 3]                           # ["b", "c", "d"]
show "HammerScript"[0 to 5]                 # "Hammer"

# Maps & Dictionaries
set hero to {"name": "Thor", "weapon": "Mjolnir"}
show keys(hero)                             # ["name", "weapon"]
show values(hero)                           # ["Thor", "Mjolnir"]
show has_key(hero, "weapon")                # true
```

### 14. Native JSON Support (`json_parse` & `json_encode`)
```hammer
# Parse JSON string into native Maps and Lists
set jsonStr to '{"name": "Admin", "id": 101, "roles": ["developer", "root"]}'
set user to json_parse(jsonStr)

show "User: " + user["name"]
show "First role: " + user["roles"][0]

# Encode native Maps and Lists into formatted JSON
set payload to {"service": "Auth", "port": 443, "active": true}
set serialized to json_encode(payload)
show "JSON: " + serialized
```

### 15. Multi-line Block Comments (`note ... end note`)
Hammer Script supports readable documentation and multi-line notes:

```hammer
note
    This whole block is completely ignored by the compiler and interpreter!
    You can document architectures, write design notes, or comment out blocks:
    show "This will not run"
end note

show "Active code runs normally!"
```

### 16. System, Time, Audio & Platform Library (`hamsys`)
Hammer Script provides native, zero-dependency access to the operating system, high-resolution time, sound playback, hardware specs, clipboard, and alert dialogs:

```hammer
# Date, Time & Uptime
show "Formatted: " + hamsys_datetime()    # "2026-10-18 02:35:00"
show "Date: " + hamsys_date()            # "2026-10-18"
show "Time: " + hamsys_time_str()        # "02:35:00"
show "Uptime: " + hamsys_uptime() + "s"  # System uptime in seconds

set t to hamsys_now()                    # Detailed map: year, month, day, hour, min, sec, epoch
show "Year: " + t["year"] + ", Month: " + t["month"]

# Hardware & System Info
show "OS: " + hamsys_os()                # "Windows (64-bit)"
show "CPU Cores: " + hamsys_cpu_count()  # Logical CPU threads
show "User: " + hamsys_user()            # Logged Windows user
show "Computer: " + hamsys_hostname()    # Machine host name

set ram to hamsys_memory()               # Map: total_mb, free_mb, used_percent
show "Free RAM: " + ram["free_mb"] + " MB / " + ram["total_mb"] + " MB"

# Read Environment Variables
show "System Drive: " + hamsys_env("SystemDrive")

# Windows Clipboard
hamsys_clipboard_set("Copied from Hammer Script!")
set clip to hamsys_clipboard_get()
show "Clipboard content: " + clip

# Native Sounds & Audio
hamsys_beep(880, 200)                    # Beep frequency 880 Hz for 200ms
hamsys_system_sound("ok")                # System notification sounds: "ok", "error", "warning", "info"
hamsys_play_sound("sound.wav", true)     # Play WAV audio (sync or async)

# Native Alert Dialog
hamsys_alert("Application Notice", "Task completed successfully!")

# Object namespace style is also supported:
show "Date via object: " + hamsys["date"]()
```

### 16. Terminal & Shell Control (`hmshell`)
```hammer
hmshell_title("Hammer Studio v1.0")          # Set terminal title
hmshell_cursor(10, 5)                        # Move cursor to X=10, Y=5
hmshell_color(10)                            # Set terminal text color attribute
hmshell_clear()                              # Clear terminal screen
hmshell_exec("dir")                          # Execute shell command and capture output
```

### 17. 2D Console Canvas Engine (`hmcanvas`)
```hammer
hmcanvas_init(40, 10)                        # Initialize 40x10 character grid
hmcanvas_clear(" ")                          # Fill canvas with blank character
hmcanvas_box(0, 0, 40, 10, "#")              # Draw box border
hmcanvas_text(4, 2, "HAMMER 2D CANVAS")      # Render text at coordinates (X, Y)
hmcanvas_set(20, 5, "@")                     # Set single pixel/character
hmcanvas_render()                            # Display canvas to console
```

---

## 📦 Community Libraries & Module Creation (Extending Language Functions)

Libraries in Hammer Script create **modules that directly add functions to the language**:

### Creating a New Module
```powershell
hammer create-lib my_module --author seu_nome
```
This generates:
- `manifest.json`: Package metadata and author credentials
- `my_module.lib`: Function signatures and documentation header
- `my_module.hmscp`: Function implementations (using `action my_func(...)`)

### Using Functions from Modules
When a library is created or installed, **its functions are automatically loaded into the global language scope**:
```hammer
# Functions are immediately available globally:
show my_module_hello("Developer")

# Or explicitly import using clean syntax:
use my_module
import matrix_math
include "ansi_colors"
```

### Library Management Commands
```powershell
hammer create-lib <nome> [--author <autor>]   # Create a new module
hammer inst hmx --author <autor> <pkg>        # Install community library
hammer inst hmx --monolithiiun <pkg>          # Install verified official library
hammer libs                                   # List all installed libraries
hammer remove-lib <pkg>                       # Uninstall a library
```

---

## 🪝 HamHook (Dedicated Native PE Compiler - 0 External Dependencies)

HamHook is a **dedicated standalone compiler** that packages scripts and bundled modules into true Windows native `.exe` binaries in under 50ms with **zero external compiler or MSYS2 dependencies**:

```powershell
# Compile to native standalone .exe (runs on any Windows machine with 0 dependencies)
hammer hamhook script.hmscp -o script.exe

# Compile without console window (GUI mode - patches PE subsystem 2)
hammer hamhook my_gui.hmscp --no-console -o my_app.exe

# All installed community and monolithiiun modules are automatically bundled into the .exe!
```

---

## 🛠️ Building the Compiler from Source (MSYS2)

The compiler is built in C++17 using MSYS2 GCC:
```powershell
.\build.bat
```
or with MSYS2 make:
```powershell
make
```
