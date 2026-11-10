"""
Copies the `main` component's real compile settings from build/compile_commands.json
into the fallback fields of .vscode/c_cpp_properties.json.

cpptools uses compile_commands.json for files listed there, and the fallback fields
(includePath, defines, compilerPath, ...) for everything else: new .cpp files that
haven't been through a reconfigure yet, and headers. With the fallback mirroring
main.cpp's flags, new files in main/ get full IntelliSense immediately.

Re-run after changing REQUIRES, adding/removing components, or switching ESP-IDF
version. Adding source files or headers does NOT require a re-run.

Usage (from the project root):  python tools/update_intellisense.py
"""

import json
import shlex
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
COMPILE_COMMANDS = ROOT / "build" / "compile_commands.json"
PROPERTIES = ROOT / ".vscode" / "c_cpp_properties.json"
REFERENCE_FILE = ROOT / "main" / "main.cpp"
CONFIG_NAME = "ESP-IDF"


def find_entry(entries):
    want = REFERENCE_FILE.resolve()
    for entry in entries:
        if Path(entry["file"]).resolve() == want:
            return entry
    sys.exit(f"{REFERENCE_FILE} not found in {COMPILE_COMMANDS}; run `idf.py reconfigure` first.")


def read_response_file(path):
    return shlex.split(Path(path).read_text(), posix=True)


def parse(command):
    # Compiler path contains backslashes (Windows) but no spaces; take it verbatim.
    compiler = command.split(None, 1)[0]
    tokens = shlex.split(command, posix=True)[1:]

    includes, defines, forced, compiler_args = [], [], [], []
    std = None
    i = 0
    while i < len(tokens):
        tok = tokens[i]
        nxt = tokens[i + 1] if i + 1 < len(tokens) else None
        if tok.startswith("@"):
            # response file (e.g. build/toolchain/cxxflags: -march, -mabi, ...)
            compiler_args += [t for t in read_response_file(tok[1:]) if t.startswith(("-m", "-f"))]
        elif tok in ("-I", "-isystem") and nxt:
            includes.append(nxt)
            i += 1
        elif tok.startswith("-I"):
            includes.append(tok[2:])
        elif tok.startswith("-isystem"):
            includes.append(tok[len("-isystem"):])
        elif tok == "-D" and nxt:
            defines.append(nxt)
            i += 1
        elif tok.startswith("-D"):
            defines.append(tok[2:])
        elif tok == "-include" and nxt:
            forced.append(nxt)
            i += 1
        elif tok.startswith("-std="):
            std = tok[len("-std="):]
        i += 1

    return compiler, includes, defines, forced, compiler_args, std


def to_cpp_standard(std):
    # cpptools doesn't know the "2b"/"2a" draft names
    aliases = {"gnu++2b": "gnu++23", "c++2b": "c++23", "gnu++2a": "gnu++20", "c++2a": "c++20"}
    return aliases.get(std, std) if std else "gnu++23"


def main():
    entries = json.loads(COMPILE_COMMANDS.read_text())
    entry = find_entry(entries)
    command = entry.get("command") or shlex.join(entry["arguments"])
    compiler, includes, defines, forced, compiler_args, std = parse(command)

    props = json.loads(PROPERTIES.read_text()) if PROPERTIES.exists() else {"configurations": [], "version": 4}
    config = next((c for c in props["configurations"] if c.get("name") == CONFIG_NAME), None)
    if config is None:
        config = {"name": CONFIG_NAME}
        props["configurations"].append(config)

    config.update({
        # exact per-file flags for every file CMake knows about
        "compileCommands": "${config:idf.buildPath}/compile_commands.json",
        # fallback for everything else (new files, headers) = main.cpp's flags
        "compilerPath": compiler,
        "compilerArgs": compiler_args,
        "intelliSenseMode": "gcc-x86",
        "cStandard": "gnu17",
        "cppStandard": to_cpp_standard(std),
        "includePath": ["${workspaceFolder}/main/**"] + includes,
        "defines": defines,
        "forcedInclude": forced,
    })

    PROPERTIES.parent.mkdir(exist_ok=True)
    PROPERTIES.write_text(json.dumps(props, indent=2) + "\n")
    print(f"Updated {PROPERTIES.relative_to(ROOT)}: {len(includes)} include paths, "
          f"{len(defines)} defines, compiler {Path(compiler).name}")


if __name__ == "__main__":
    main()
