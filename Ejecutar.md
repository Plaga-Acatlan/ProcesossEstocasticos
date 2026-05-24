# Ejecutar el proyecto localmente

## Requisitos previos

- Python 3.11+ instalado
- `gcc` disponible en el sistema (MinGW en Windows o build-essential en Linux)
- Git Bash / PowerShell / terminal disponible

## 1. Crear y activar el entorno virtual

En Windows:

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
```

En Linux/macOS/GitHub Codespaces:

```bash
python3 -m venv .venv
source .venv/bin/activate
```

## 2. Instalar dependencias de Python

```bash
pip install -r requirements.txt
```

## 3. Generar la librería compartida con `build.py`

Este proyecto usa `build.py` para compilar los archivos C y generar la librería `procesos.dll` en Windows.

```bash
python build.py
```

Si no hay errores, se generará el archivo:

- `procesos.dll` en Windows
- `procesos.so` en Linux
- `procesos.dylib` en macOS

## 4. Ejecutar la aplicación Flask

Con el entorno virtual activado y la librería compilada:

```bash
python app.py
```

## 5. Abrir la aplicación en el navegador

Accede a la URL generada

## Notas adicionales

- El proyecto usa `ctypes` para cargar la librería C desde `app.py`.
- Asegúrate de ejecutar `python build.py` cada vez que cambies el código en `src/`.
- Si `gcc` no está instalado en Windows, instala MinGW y agrega `gcc.exe` al `PATH`.
