#!/usr/bin/env python3
"""Build demo executables: demo_lexer.exe and demo_parser.exe"""
import subprocess, os, sys

VC_PATH = r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.37.32822"
SDK_PATH = r"C:\Program Files (x86)\Windows Kits\10"
SDK_VER = "10.0.22621.0"
SRCDIR = os.path.dirname(os.path.abspath(__file__))

env = os.environ.copy()
env["INCLUDE"] = os.pathsep.join([
    VC_PATH + "\\include",
    SDK_PATH + "\\Include\\" + SDK_VER + "\\ucrt",
    SDK_PATH + "\\Include\\" + SDK_VER + "\\shared",
    SDK_PATH + "\\Include\\" + SDK_VER + "\\um",
])
env["LIB"] = os.pathsep.join([
    VC_PATH + "\\lib\\x64",
    SDK_PATH + "\\Lib\\" + SDK_VER + "\\ucrt\\x64",
    SDK_PATH + "\\Lib\\" + SDK_VER + "\\um\\x64",
])
env["PATH"] = os.pathsep.join([
    VC_PATH + "\\bin\\Hostx64\\x64",
    SDK_PATH + "\\bin\\" + SDK_VER + "\\x64",
    env.get("PATH", ""),
])

CL = VC_PATH + "\\bin\\Hostx64\\x64\\cl.exe"

def cc(src_file):
    """Compile a .c file to .obj. Returns path to .obj or None on failure."""
    obj = src_file.replace(".c", ".obj")
    args = [CL, "/std:c++17", "/EHsc", "/W3", "/nologo",
            "/c", src_file, "/Fo:" + obj]
    print(f"  CC {os.path.basename(src_file)}")
    r = subprocess.run(args, cwd=SRCDIR, env=env, capture_output=True, text=True)
    if r.returncode != 0:
        print("    FAILED")
        if r.stdout: print("    stdout:", r.stdout[-400:])
        if r.stderr: print("    stderr:", r.stderr[-400:])
        return None
    return obj

def link(exe_name, objs, extra_flags=None):
    """Link .obj files into an .exe."""
    args = [CL, "/nologo", "/Fe:" + os.path.join(SRCDIR, exe_name)] + objs
    args.extend(["/link", "/FORCE:MULTIPLE"])
    print(f"  LINK -> {exe_name}")
    r = subprocess.run(args, cwd=SRCDIR, env=env, capture_output=True, text=True)
    # /FORCE:MULTIPLE turns duplicate symbol errors into warnings
    if r.returncode != 0:
        print("    FAILED")
        if r.stdout: print("    stdout:", r.stdout[-400:])
        if r.stderr: print("    stderr:", r.stderr[-400:])
        return False
    return True

def run_exe(args, desc):
    """Run an executable (SeuLex/SeuYacc)."""
    print(f"  {desc}")
    r = subprocess.run(args, cwd=SRCDIR, capture_output=True, text=True)
    return r.returncode == 0

def build_demo():
    src = lambda f: os.path.join(SRCDIR, f)

    # Step 0: Rebuild SeuLex and SeuYacc tools first to ensure they are up to date
    print("=" * 60)
    print("Step 0: Rebuild SeuLex.exe and SeuYacc.exe")
    import importlib.util, types as _types
    build_py = os.path.join(SRCDIR, "build.py")
    spec = importlib.util.spec_from_file_location("build", build_py)
    build_mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(build_mod)
    build_mod.build("all")

    # Step 1: Generate yylex.c and yyparse.c
    print("\n" + "=" * 60)
    print("Step 1: Generate yylex.c and yyparse.c")
    run_exe([src("SeuLex.exe"), src("课程资料/minic.l"), "-o", src("yylex.c")],
            "SeuLex -> yylex.c")
    run_exe([src("SeuYacc.exe"), src("课程资料/minic.y"), "-o", src("yyparse.c")],
            "SeuYacc -> yyparse.c")

    # Step 2: Build demo_lexer.exe
    print("\n" + "=" * 60)
    print("Step 2: Build demo_lexer.exe (token printer)")

    lex_sources = ["demo_lexer.c", "yylex.c", "names.c", "symtab.c", "types.c"]
    lex_objs = []
    for s in lex_sources:
        obj = cc(src(s))
        if not obj: return
        lex_objs.append(obj)
    if not link("demo_lexer.exe", lex_objs): return

    # Step 3: Build demo_parser.exe (with YY_TRACE)
    print("\n" + "=" * 60)
    print("Step 3: Build demo_parser.exe (parser with trace)")

    # Compile yyparse.c with YY_TRACE defined
    print("  CC yyparse.c (with YY_TRACE)")
    yyparse_obj = src("yyparse.obj")
    r = subprocess.run([CL, "/std:c++17", "/EHsc", "/W3", "/nologo", "/DYY_TRACE",
                        "/c", src("yyparse.c"), "/Fo:" + yyparse_obj],
                       cwd=SRCDIR, env=env, capture_output=True, text=True)
    if r.returncode != 0:
        print("    FAILED:", r.stderr[-400:] if r.stderr else "")
        return

    parse_sources = ["yylex.c", "names.c", "symtab.c",
                     "types.c", "check.c"]
    # demo_parser.c must be compiled and linked FIRST so its main() wins
    demo_parser_obj = cc(src("demo_parser.c"))
    if not demo_parser_obj: return
    parse_objs = [demo_parser_obj, yyparse_obj]
    for s in parse_sources:
        obj = cc(src(s))
        if not obj: return
        parse_objs.append(obj)
    if not link("demo_parser.exe", parse_objs): return

    print("\n" + "=" * 60)
    print("Done! Demo executables:")
    print("  demo_lexer.exe  - token stream printer")
    print("  demo_parser.exe - parser with reduction trace")
    print("=" * 60)

if __name__ == "__main__":
    build_demo()