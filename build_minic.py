#!/usr/bin/env python3
"""Build the complete MiniC compiler from generated yylex.c + yyparse.c + support libs."""
import subprocess
import os
import sys

VC_PATH = r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.37.32822"
SDK_PATH = r"C:\Program Files (x86)\Windows Kits\10"
SDK_VER = "10.0.22621.0"
SRCDIR = os.path.dirname(os.path.abspath(__file__))

env = os.environ.copy()
env["INCLUDE"] = os.pathsep.join([
    VC_PATH + "\\include", SDK_PATH + "\\Include\\" + SDK_VER + "\\ucrt",
    SDK_PATH + "\\Include\\" + SDK_VER + "\\shared", SDK_PATH + "\\Include\\" + SDK_VER + "\\um",
])
env["LIB"] = os.pathsep.join([
    VC_PATH + "\\lib\\x64", SDK_PATH + "\\Lib\\" + SDK_VER + "\\ucrt\\x64",
    SDK_PATH + "\\Lib\\" + SDK_VER + "\\um\\x64",
])
env["PATH"] = os.pathsep.join([
    VC_PATH + "\\bin\\Hostx64\\x64", SDK_PATH + "\\bin\\" + SDK_VER + "\\x64", env.get("PATH", ""),
])

CL = VC_PATH + "\\bin\\Hostx64\\x64\\cl.exe"

def compile_file(src, obj):
    args = [CL, "/std:c++17", "/EHsc", "/W3", "/nologo", "/c", src, "/Fo:" + obj]
    print(f"  CC {os.path.basename(src)}")
    r = subprocess.run(args, cwd=SRCDIR, env=env, capture_output=True, text=True)
    if r.returncode != 0:
        print("FAILED:")
        if r.stdout: print("  STDOUT:", r.stdout[-1000:])
        if r.stderr: print("  STDERR:", r.stderr[-1000:])
        return False
    return True

def build_minic():
    objs = []
    # Driver must be FIRST so its main() takes precedence over yylex.c's main()
    fp = os.path.join(SRCDIR, "minic_driver.c")
    obj = os.path.join(SRCDIR, "minic_driver.obj")
    if not compile_file(fp, obj): return False
    objs.append(obj)

    # Support files
    support = ["names.c", "symtab.c", "types.c", "check.c"]
    for f in support:
        fp = os.path.join(SRCDIR, f)
        obj = os.path.join(SRCDIR, f.replace(".c", ".obj"))
        if not compile_file(fp, obj): return False
        objs.append(obj)

    # yylex.c
    fp = os.path.join(SRCDIR, "yylex.c")
    obj = os.path.join(SRCDIR, "yylex.obj")
    if not compile_file(fp, obj): return False
    objs.append(obj)

    # yyparse.c
    fp = os.path.join(SRCDIR, "yyparse.c")
    obj = os.path.join(SRCDIR, "yyparse.obj")
    if not compile_file(fp, obj): return False
    objs.append(obj)

    # Link with /FORCE:MULTIPLE to handle duplicate main() symbols
    out = os.path.join(SRCDIR, "minic.exe")
    args = [CL, "/nologo", "/Fe:" + out] + objs + ["/link", "/FORCE:MULTIPLE"]
    print(f"  LINK -> minic.exe")
    r = subprocess.run(args, cwd=SRCDIR, env=env, capture_output=True, text=True)
    # /FORCE:MULTIPLE makes duplicate symbols warnings not errors
    if r.returncode != 0:
        print("LINK FAILED:")
        if r.stdout: print("  STDOUT:", r.stdout[-2000:])
        if r.stderr: print("  STDERR:", r.stderr[-2000:])
        return False
    if r.stdout and "warning" in r.stdout.lower():
        print("  (duplicate main warning expected, ignored)")
    print("  OK")
    return True

if __name__ == "__main__":
    if build_minic():
        print("\nMiniC compiler built: minic.exe")
    else:
        print("\nBuild FAILED")
        sys.exit(1)
