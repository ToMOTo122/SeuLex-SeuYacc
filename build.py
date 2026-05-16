#!/usr/bin/env python3
"""Build SeuLex and SeuYacc using MSVC compiler."""
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

def run_cl(files, output, includes):
    args = [CL, "/std:c++17", "/EHsc", "/W3", "/nologo", "/Fe:" + output]
    for inc in includes:
        args.append("/I" + inc)
    args.extend(files)
    print(" ".join(args))
    r = subprocess.run(args, cwd=SRCDIR, env=env, capture_output=True, text=True)
    if r.stdout: print(r.stdout)
    if r.stderr:
        err = r.stderr
        if len(err) > 2000: err = err[-2000:]
        print(err)
    return r.returncode

def build(target):
    src = lambda d, f: os.path.join(SRCDIR, d, f)
    if target in ("all", "seulex"):
        files = [src("seulex", f) for f in ["LexParser.cpp","REProcessor.cpp","NFABuilder.cpp","DFABuilder.cpp","DFAMinimizer.cpp","LexCodeGen.cpp","SeuLex.cpp"]]
        includes = [os.path.join(SRCDIR, "shared"), os.path.join(SRCDIR, "seulex")]
        ret = run_cl(files, os.path.join(SRCDIR, "SeuLex.exe"), includes)
        if ret != 0:
            print("SeuLex build FAILED!")
            if target != "all": sys.exit(1)
        else:
            print("SeuLex build OK.")
    if target in ("all", "seuyacc"):
        files = [src("seuyacc", f) for f in ["GrammarParser.cpp","DFAGenerator.cpp","Emitter.cpp","SeuYacc.cpp"]]
        includes = [os.path.join(SRCDIR, "shared"), os.path.join(SRCDIR, "seuyacc"), os.path.join(SRCDIR, "seulex")]
        ret = run_cl(files, os.path.join(SRCDIR, "SeuYacc.exe"), includes)
        if ret != 0:
            print("SeuYacc build FAILED!")
            if target != "all": sys.exit(1)
        else:
            print("SeuYacc build OK.")

if __name__ == "__main__":
    build(sys.argv[1] if len(sys.argv) > 1 else "all")
