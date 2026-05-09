#!/usr/bin/env python3
import os
import platform
import subprocess
import glob

def main():
    system = platform.system()
    cwd = os.getcwd()
    
    # Detectar fuentes e includes
    src_files = glob.glob(os.path.join("src", "*.c")) or glob.glob("*.c")
    inc_dir = os.path.join(cwd, "include")
    
    if not src_files:
        print("No se encontraron archivos .c en 'src/' o en la raíz.")
        return

    # Flags base
    flags = ["-shared", "-O2"]
    if system == "Linux":
        flags.append("-fPIC")
    if os.path.isdir(inc_dir):
        flags.extend(["-I", inc_dir])

    # Output según SO
    ext = {"Windows": "dll", "Linux": "so", "Darwin": "dylib"}.get(system, "so")
    out_name = "matrix_ops" if system == "Windows" else "libmatrix"
    out_file = os.path.join(cwd, f"{out_name}.{ext}")

    # Comando final
    cmd = ["gcc"] + flags + ["-o", out_file] + src_files

    print(f"🔨 Compilando para {system}...")
    print(f"📂 Fuentes: {len(src_files)} archivo(s)")
    if os.path.isdir(inc_dir):
        print(f"📚 Includes: -I {inc_dir}")

    try:
        subprocess.run(cmd, check=True, cwd=cwd)
        print(f"✅ Generado: {out_file}")
    except subprocess.CalledProcessError as e:
        print(f"❌ Error en gcc (código {e.returncode})")
    except FileNotFoundError:
        print("❌ 'gcc' no encontrado. Instala 'build-essential' (Linux) o MinGW (Windows).")

if __name__ == "__main__":
    main()