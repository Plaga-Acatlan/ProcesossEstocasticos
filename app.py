import os
import platform
import ctypes
import numpy as np
import traceback
import json
from flask import Flask, request, render_template, jsonify

app = Flask(__name__)

# Auto-detección de librería compilada
try:
    _EXT_MAP = {"Windows": "dll", "Linux": "so", "Darwin": "dylib"}
    _SYSTEM = platform.system()
    _LIB_EXT = _EXT_MAP.get(_SYSTEM, "so")
    _LIB_NAME = f"procesos.{_LIB_EXT}" 
    _LIB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), _LIB_NAME)

    if not os.path.exists(_LIB_PATH):
        raise RuntimeError(f"⚠️ No se encontró '{_LIB_NAME}'.")


    lib = ctypes.CDLL(_LIB_PATH)
    lib.set_configuration.argtypes = [
        ctypes.c_int, 
        ctypes.c_int, 
        ctypes.c_int, 
        ctypes.c_char_p
    ]
    lib.set_configuration.restype = ctypes.c_int

    
except Exception as e:
    print(f"❌ Error cargando librería C: {e}")
    algo_lib = None

def parsear_matriz(texto: str) -> np.ndarray:
    filas = [f.strip() for f in texto.strip().split(';') if f.strip()]
    if not filas: raise ValueError("Matriz vacía")
    return np.array([[float(v) for v in f.split(',')] for f in filas], dtype=np.float64)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/procesar')
def procesar():
    """Página de resumen y selección de método"""
    return render_template('procesar.html')

@app.route('/EEP')
def enumeracion_exhaustiva():
    return render_template('EEP.html')

@app.route('/PL')
def programacion_lineal():
    return render_template('PL.html')

@app.route('/MP')
def mejoramiento_politicas():
    return render_template('MP.html')

@app.route('/MPD')
def mejoramiento_descuento():
    return render_template('MPD.html')

@app.route('/AS')
def aproximaciones_sucesivas():
    return render_template('AS.html')

@app.route('/procesar', methods=['POST'])
def procesar_datos():
    """Valida y guarda datos antes de ir a la página de selección"""
    try:
        data = request.get_json()
        if not data:
            return jsonify({"error": "No se recibieron datos"}), 400
        
        # Aquí puedes validar los datos si es necesario
        return jsonify({"status": "ok", "message": "Datos validados"}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, host='127.0.0.1', port=5000)