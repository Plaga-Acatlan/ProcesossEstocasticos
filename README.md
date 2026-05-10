# ProcesossEstocasticos
Desarrollo de proyecto para la materia de procesos estocásticos.

## Instalación y Ejecución

Para ejecutar los archivos desde la terminal de VS Code, necesitas instalar MSYS y MinGW.

### Instalación de MSYS y MinGW

1. Descarga e instala MSYS2 desde [msys2.org](https://www.msys2.org/).
2. Instala MinGW-w64: Abre MSYS2 UCRT64 y ejecuta `pacman -S mingw-w64-x86_64-gcc`.

### Compilación y Ejecución

Para compilar el programa, usa el siguiente comando:

```bash
gcc -Wall -Wextra -std=c11 -I include main.c src/*.c -o programa -lm
```

Luego, ejecuta el programa con:

```bash
./programa
```

Asegúrate de estar en el directorio raíz del proyecto.
