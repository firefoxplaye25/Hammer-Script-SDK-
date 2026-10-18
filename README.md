<div align="center">

# 🔨 Hammer Script

**A modern, intuitive programming language designed for clarity, high performance, and effortless standalone native compilation.**

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/firefoxplaye25/Hammer-Script/releases/tag/v1.0.0)
[![Platform](https://img.shields.io/badge/platform-Windows%20x64-brightgreen.svg)]()
[![Compiler](https://img.shields.io/badge/compiler-HamHook%20Standalone%20PE-orange.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/badge/download-SDK%20%26%20Extension-purple.svg)](https://github.com/firefoxplaye25/Hammer-Script/releases/tag/v1.0.0)

</div>

---

## 💡 Overview

Hammer Script (`.hmscp`) was created with a clear philosophy: **programming should be intuitive, readable, and free of frustrating setup hurdles**. 

- **No cryptic syntax:** Uses expressive English grammar, clean block structures with `end`, and natural expressions.
- **Dedicated Native Compiler (HamHook):** Produces true standalone Windows `.exe` executables in **~45 ms** with **zero external dependencies** — no MSYS2, no MinGW, and no toolchains required on end-user machines.
- **Empathetic Diagnostic Engine:** Instead of throwing silent errors or unhelpful traces, Hammer Script pinpoints the exact line, explains *why* the error happened, and provides a runnable correction example.
- **Built-in Power Subsystems:** Ready-to-use native modules for HTTP/sockets (`asock`), disk I/O (`hamfs`), cryptographic encryption (`hmripa`), OS/audio/hardware (`hamsys`), terminal styling (`hmshell`), and 2D canvas graphics (`hmcanvas`).

---

## 📦 Downloads & Releases

Everything is pre-packaged and ready to download in our [**Official Releases**](https://github.com/firefoxplaye25/Hammer-Script/releases/tag/v1.0.0):

| Download | File | Description |
| :--- | :--- | :--- |
| **Hammer SDK** | [`hammer-sdk-v1.0.0.zip`](https://github.com/firefoxplaye25/Hammer-Script/releases/download/v1.0.0/hammer-sdk-v1.0.0.zip) | Complete SDK: CLI (`hammer.exe`), standalone compiler (`hamhook.exe`), transpiler (`hamc.exe`), headers (`.lib`), and standard library. |
| **VS Code Extension** | [`hammer-script-lang-1.0.0.vsix`](https://github.com/firefoxplaye25/Hammer-Script/releases/download/v1.0.0/hammer-script-lang-1.0.0.vsix) | Full syntax highlighting and language support for VS Code & Antigravity IDE. |
| **Extension Archive** | [`hammer-script-vscode-extension.zip`](https://github.com/firefoxplaye25/Hammer-Script/releases/download/v1.0.0/hammer-script-vscode-extension.zip) | Portable extension folder for manual installation. |

---

## ⚡ Quick Start

### 1. Running a Script Directly (Interpreter)
```powershell
hammer run examples\01_hello.hmscp
```

### 2. Compiling to a Standalone Executable (HamHook)
```powershell
# Compile to a native Windows .exe with zero external dependencies
hammer hamhook examples\01_hello.hmscp -o hello.exe

# Or call hamhook directly:
hamhook examples\01_hello.hmscp -o hello.exe

# Run your standalone app anywhere:
.\hello.exe
```

### 3. Interactive REPL Shell
```powershell
hammer repl
```

### 4. Syntax Verification & Diagnostics
```powershell
hammer check my_script.hmscp
```

---

## 📖 Language Documentation

### 1. Variables & Types
Variables are dynamically typed and declared using `set <var> to <val>` or `<var> = <val>`:
```hammer
set name to "Alice"
set score to 100
set is_online to true
set items to ["Key", "Potion", "Shield"]
set profile to {"id": 1, "role": "admin"}
```

### 2. Output & Input
```hammer
show "Hello, Hammer Script!"
set player to ask "What is your name? "
show "Welcome back, " + player + "! Your score is: " + score
```

### 3. String Interpolation
Embed variables and expressions cleanly with `{expr}`:
```hammer
set user to "Arthur"
set level to 42
show "Player {user} has reached level {level}! Next milestone at {level + 10}."
```

### 4. Conditions & Natural Expressions
Hammer Script supports both mathematical symbols and readable English operators:
```hammer
set health to 75

if health is at least 100 then
    show "Full health!"
elseif health is greater than 20 then
    show "In combat shape"
else
    show "Critical danger!"
end
```
*Comparison keywords:* `is`, `is not`, `greater than`, `less than`, `at least`, `at most`, `and`, `or`, `not`.

### 5. Loops
```hammer
# Fixed iterations
repeat 3 times
    show "Hammer is fast!"
end

# Range loop (ascending or descending with optional step)
repeat with i from 1 to 10 step 2
    show "Step: " + i
end

# Collection iteration
for each item in ["Sword", "Armor", "Bow"]
    show "Equipped: " + item
end

# While loop
while score is greater than 0
    set score to score - 25
end
```

### 6. Functions (Actions)
```hammer
action calculate_bonus(base, multiplier)
    give (base * multiplier) + 50
end

set total to calculate_bonus(100, 2)
show "Total Bonus: " + total
```

### 7. Natural Slicing (Inclusive)
```hammer
set phrase to "HammerScript"
show phrase[0 to 5]                 # "Hammer"

set list to ["A", "B", "C", "D", "E"]
show list[1 to 3]                  # ["B", "C", "D"]
```

### 8. Robust Error Handling (`attempt`)
```hammer
attempt:
    set content to read_file("settings.json")
    show content
on error err:
    show "Caught error gracefully: " + err
end
```

### 9. Block Comments (`note ... end note`)
```hammer
note
    This is a multi-line note block in Hammer Script.
    Everything inside is completely ignored by the engine.
end note
```

---

## 🔌 Built-in Subsystems & Modules

Hammer Script comes out-of-the-box with dedicated native engines:

### 🌐 1. Internet & Networking (`asock` / `hmhookwww`)
Full HTTP/HTTPS client and raw TCP socket networking:
```hammer
# HTTP GET
set res to asock_get("https://httpbin.org/get")
show "Status: " + res["status"]
show "Payload: " + res["body"]

# HTTP POST with headers
set headers to {"Content-Type": "application/json"}
set postRes to asock_post("https://httpbin.org/post", '{"id": 42}', headers)

# Direct stream download to disk
asock_download("https://example.com/logo.png", "logo.png")

# TCP Socket connection
set sock to asock_connect("tcp.example.com", 8080)
asock_send(sock, "PING\n")
set reply to asock_recv(sock, 1024)
asock_close(sock)
```

### 📁 2. File System Engine (`hamfs`)
```hammer
hamfs_mkdir("game_data")
hamfs_write("game_data/save.txt", "Level=5\nCoins=250")
hamfs_append("game_data/save.txt", "\nCheckpoint=2")

set save to hamfs_read("game_data/save.txt")
show "File size: " + hamfs_size("game_data/save.txt") + " bytes"
show "Directory listing: " + hamfs_list("game_data")
```

### 🔐 3. Symmetric Cryptography (`hmripa`)
Built-in encryption and hashing with automatic key validation:
```hammer
# String encryption/decryption
set cipher to hmripa_encrypt("My secret credentials", "StrongKey2026")
set plain to hmripa_decrypt(cipher, "StrongKey2026")

# Direct file encryption/decryption
hmripa_encrypt_file("credentials.txt", "credentials.enc", "MasterKey")
hmripa_decrypt_file("credentials.enc", "credentials_restored.txt", "MasterKey")

# Cryptographic hashing
show "64-bit Hash: " + hmripa_hash("Hammer Script")
```

### 💻 4. Operating System & Hardware (`hamsys`)
```hammer
show "Date & Time: " + hamsys_datetime()
show "OS Version: " + hamsys_os()
show "CPU Cores: " + hamsys_cpu_count()
show "RAM Available: " + hamsys_memory()["available_mb"] + " MB"

# System clipboard
hamsys_clipboard_set("Copied from Hammer Script!")
show "Clipboard: " + hamsys_clipboard_get()

# Audio & System alerts
hamsys_beep(880, 200)                    # 880 Hz for 200ms
hamsys_system_sound("ok")                # System sound: "ok", "error", "warning"
hamsys_alert("Notice", "Task finished!") # Windows alert popup
```

### 🖥️ 5. Terminal Control (`hmshell`)
```hammer
hmshell_title("Hammer Studio v1.0")
hmshell_cursor(10, 5)
hmshell_color(10)                        # High-intensity green
hmshell_clear()                          # Clear terminal screen
```

### 🎮 6. 2D Console Graphics (`hmcanvas`)
```hammer
hmcanvas_init(40, 10)                    # 40x10 character grid
hmcanvas_clear(" ")
hmcanvas_box(0, 0, 40, 10, "#")          # Draw border box
hmcanvas_text(4, 2, "HAMMER 2D CANVAS")  # Centered text
hmcanvas_set(20, 5, "@")                 # Draw sprite
hmcanvas_render()                        # Render buffer to console
```

### 📦 7. Native JSON Manipulation
```hammer
set data = json_parse('{"user": "Admin", "roles": ["developer", "root"]}')
show "User role: " + data["roles"][0]

set serialized = json_encode({"status": 200, "message": "Success"})
```

---

## 🧩 Community Libraries & Language Extensions

Hammer Script allows developers to build modules that **directly extend the language with new functions**:

### Creating a New Module
```powershell
hammer create-lib my_module --author developer_name
```
This generates:
- `manifest.json`: Package metadata and configuration
- `my_module.lib`: Function signatures and API header
- `my_module.hmscp`: Function implementations using `action`

### Auto-Loading & Importing
Any module installed in `lib loader/` is **automatically loaded on engine startup**:
```hammer
# Functions from installed modules are directly accessible globally:
show my_module_hello("Developer")

# Or explicitly import using clean syntax:
use my_module
import matrix_math
include "ansi_colors"
```

### Package Manager CLI
```powershell
hammer create-lib <name> [--author <name>]  # Scaffold a new module
hammer inst hmx --author <author> <pkg>     # Install community library
hammer inst hmx --monolithiiun <pkg>         # Install verified monolithic library
hammer libs                                  # List all installed libraries
hammer remove-lib <pkg>                      # Uninstall a library
```

---

## 🪝 HamHook: Dedicated Native Standalone PE Compiler

Unlike traditional scripting languages that require installing a 500 MB compiler toolchain, **HamHook** is built directly into the Hammer Script executable:

```powershell
# Compile to native Windows PE executable in ~45 ms (Zero external dependencies)
hammer hamhook my_script.hmscp -o my_app.exe

# Compile silent GUI application (No console window)
hammer hamhook my_gui.hmscp --no-console -o my_app.exe
```

- **Portable Output:** Produced `.exe` files run on any modern Windows 64-bit machine with zero runtime dependencies.
- **Auto-Bundling:** HamHook automatically embeds all installed community modules and scripts into the binary payload.

---

## 🎨 Friendly Diagnostic Engine

When mistakes happen, Hammer Script provides human-first diagnostics:

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
================================================================================
```

---

## 🛠️ Building from Source

To compile the Hammer Script SDK from source code (requires MSYS2 GCC/G++ with C++17 support):

```powershell
.\build.bat
```
This compiles `hammer.exe`, `hamc.exe`, `hamhook.exe`, and packages the complete public distribution into `Build\hammer-sdk\`.

---

## 📄 License

Hammer Script is released under the permissive [MIT License](LICENSE).
