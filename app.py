import os
import platform
import ctypes
import numpy as np
from flask import Flask, request, render_template

app = Flask(__name__)

# Auto-detección de librería compilada
_EXT_MAP = {"Windows": "dll", "Linux": "so", "Darwin": "dylib"}
_SYSTEM = platform.system()
_LIB_EXT = _EXT_MAP.get(_SYSTEM, "so")
_LIB_NAME = f"procesos.{_LIB_EXT}" 
_LIB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), _LIB_NAME)

if not os.path.exists(_LIB_PATH):
    raise RuntimeError(f"⚠️ No se encontró '{_LIB_NAME}'.")

lib = ctypes.CDLL(_LIB_PATH)
lib.sumar_matrices.argtypes = [
    ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double), ctypes.c_int, ctypes.c_int
]
lib.sumar_matrices.restype = None

def parsear_matriz(texto: str) -> np.ndarray:
    filas = [f.strip() for f in texto.strip().split(';') if f.strip()]
    if not filas: raise ValueError("Matriz vacía")
    return np.array([[float(v) for v in f.split(',')] for f in filas], dtype=np.float64)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/sumar', methods=['POST'])
def sumar():
    try:
        txt_a = request.form.get('matriz_a', '').strip()
        txt_b = request.form.get('matriz_b', '').strip()
        if not txt_a or not txt_b:
            return render_template('index.html', error="Completa ambas matrices.")

        A = parsear_matriz(txt_a)
        B = parsear_matriz(txt_b)
        if A.ndim != 2 or B.ndim != 2:
            return render_template('index.html', error="Formato esperado: '1,2;3,4'")
        if A.shape != B.shape:
            return render_template('index.html', error=f"Dimensiones distintas: {A.shape} vs {B.shape}")

        filas, cols = A.shape
        C = np.empty((filas, cols), dtype=np.float64)
        A_c, B_c, C_c = np.ascontiguousarray(A), np.ascontiguousarray(B), np.ascontiguousarray(C)

        lib.sumar_matrices(
            A_c.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            B_c.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            C_c.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            ctypes.c_int(filas), ctypes.c_int(cols)
        )

        resultado = np.array2string(C_c, separator=', ', formatter={'float_kind': lambda x: f"{x:.4f}"})
        return render_template('index.html', resultado=resultado)

    except ValueError as e:
        return render_template('index.html', error=f"Formato inválido: {e}")
    except Exception as e:
        return render_template('index.html', error=f"Error interno: {e}")

if __name__ == '__main__':
    app.run(debug=True, host='127.0.0.1', port=5000)