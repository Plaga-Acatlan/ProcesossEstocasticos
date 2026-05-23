from flask import Flask, request, jsonify, render_template
import ctypes
import ctypes.util
import os
from scipy.optimize import linprog
import re
import requests
import platform

app = Flask(__name__)

# Configuración y Carga de C
lib = None
try:
    lib = ctypes.CDLL(ctypes.util.find_library("c"))
    lib.free.argtypes = [ctypes.c_void_p]

    _EXT_MAP = {"Windows": "dll", "Linux": "so", "Darwin": "dylib"}
    _SYSTEM = platform.system()
    _LIB_EXT = _EXT_MAP.get(_SYSTEM, "so")
    _LIB_NAME = f"procesos.{_LIB_EXT}" 
    _LIB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), _LIB_NAME)

    if not os.path.exists(_LIB_PATH):
        raise RuntimeError(f"⚠️ No se encontró '{_LIB_NAME}'.")


    lib = ctypes.CDLL(_LIB_PATH)

    # ED
    class CMatrix(ctypes.Structure):
        _fields_ = [
            ("rows", ctypes.c_int),
            ("cols", ctypes.c_int),
            ("data", ctypes.POINTER(ctypes.POINTER(ctypes.c_double)))
        ]
        
    # Inicio
    lib.set_configuration.argtypes = [
        ctypes.c_int, 
        ctypes.c_int, 
        ctypes.c_int, 
        ctypes.c_char_p
    ]
    lib.set_configuration.restype = ctypes.c_int

    lib.init_with_custom_data.argtypes = [
        ctypes.POINTER(ctypes.c_double),
        ctypes.POINTER(ctypes.c_double),
        ctypes.POINTER(ctypes.c_int)
    ]
    lib.init_with_custom_data.restype = None

    # Métodos
    lib.solve_eep.restype = ctypes.POINTER(CMatrix)
    lib.return_optimal.argtypes = [ctypes.POINTER(CMatrix)]
    lib.return_optimal.restype = ctypes.c_int

    lib.FO.argtypes = []
    lib.FO.restype = ctypes.c_void_p
    lib.CondicionNormalizacion.argtypes = []
    lib.CondicionNormalizacion.restype = ctypes.c_void_p
    lib.Make_Restrictions.argtypes = []
    lib.Make_Restrictions.restype = ctypes.c_void_p
    lib.Create_MPL.argtypes = []
    lib.Create_MPL.restype = ctypes.c_void_p

    lib.free_string.argtypes = [ctypes.c_void_p]
    lib.free_string.restype = None

    lib.Mejoramiento_Politicas.argtypes = [ctypes.c_int]
    lib.Mejoramiento_Politicas.restype = ctypes.POINTER(CMatrix)

    lib.MPD.argtypes = [ctypes.c_int, ctypes.c_double]
    lib.MPD.restype = ctypes.POINTER(CMatrix)

    lib.AS.argtypes = [ctypes.c_double, ctypes.c_int, ctypes.c_double]
    lib.AS.restype = ctypes.POINTER(CMatrix)

    # Liberar Memoria
    lib.free_matrix.argtypes = [ctypes.POINTER(CMatrix)]
    lib.free_matrix.restype = None
    lib.free_global_matrices.restype = None
    lib.free.argtypes = [ctypes.c_char_p]
    lib.free.restype = None
    
except Exception as e:
    print(f"❌ Error cargando librería C: {e}")
    lib = None

# Funciones
def cargar_datos(data):
    # Aplanar JSON y subirlo en globales.

    if not lib:
        raise RuntimeError("Librería C no inicializada")
    N, K, P = data['N'], data['K'], data['P']
    tipo = data['tipo']

    # 1. Set Matrices
    transiciones = [float(v) for m in data['transiciones'] for r in m for v in r]
    costos  = [float(v) for r in data['costos'] for v in r]
    politicas   = [int(v) for p in data['politicas'] for v in p['vector']]

    # 2. Matrices buffers
    transicionesB = (ctypes.c_double * len(transiciones))(*transiciones)
    costosB  = (ctypes.c_double * len(costos))(*costos)
    politicasB   = (ctypes.c_int * len(politicas))(*politicas)

    # 3. Configuración en C
    status = lib.set_configuration(N, P, K, tipo.encode('utf-8'))
    if status != 0:
        raise RuntimeError(f"set_configuration retornó error {status}")
    
    lib.init_with_custom_data(transicionesB, costosB, politicasB)

def matrixC_to_py(ptr):
    # Recibe un Matrix* de C, lo convierte a listas en Python.
    if not ptr:
        return None
    
    matriz = ptr.contents
    rows, cols = matriz.rows, matriz.cols
    data_ptr = matriz.data 

    py_matrix = []
    for i in range(rows):
        row_ptr = data_ptr[i]
        py_matrix.append([row_ptr[j] for j in range(cols)])
    
    lib.free_matrix(ptr)
    return {"rows": rows, "cols": cols, "data": py_matrix}

def stringC_to_py(ptr):
    c_string = ctypes.cast(ptr, ctypes.c_char_p).value
    modelo = c_string.decode("utf-8")
    lib.free_string(ptr)

    if not ptr:
        return ""
    return modelo

# Rutas del sitio
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/procesar', methods=['GET', 'POST'])
def procesar():
    if request.method == 'POST':
        try:
            data = request.get_json()
            if not data:
                return jsonify({"error": "No se recibieron datos JSON"}), 400
            
            # Validación mínima de estructura (opcional pero recomendada)
            required = ['N', 'K', 'P', 'tipo', 'transiciones', 'costos', 'politicas']
            if not all(k in data for k in required):
                return jsonify({"error": "Faltan campos requeridos en el payload"}), 400
             
            return jsonify({"status": "ok", "message": "Datos validados correctamente"}), 200
            
        except Exception as e:
            return jsonify({"error": f"Error en validación: {str(e)}"}), 500
    
    else:
        # ── Lógica GET: Renderizar página de selección ──
        return render_template('procesar.html')

@app.route('/EEP', methods=['POST'])
def enumeracion_exhaustiva():
    try:
        data = request.get_json()
        if not data:
            return jsonify({"status": "error", "message": "No se recibieron datos"}), 400
        cargar_datos(data)
        N, P = data['N'], data['P']

        # Ejecutar C
        ptr = lib.solve_eep()
        mat = ptr.contents
        optimal_idx = lib.return_optimal(ptr)

        matrix = matrixC_to_py(ptr) 
        policies_result = []
        for i in range(P):
            row = matrix['data'][i]  # Lista de N+1 valores
            policies_result.append({
                "index": i,
                "vector": row[:N],      # Primeros N valores: π_0 ... π_{N-1}
                "cost": row[N],         # Último valor: costo de la política
                "original": data['politicas'][i]['vector']  # Para mostrar en UI
            })
        lib.free_global_matrices()

        policies_list = data.get('politicas', [])
        if 0 <= optimal_idx-1 < len(policies_list):
            opt_pol = policies_list[optimal_idx-1]
            optimal_payload = {
                "index": optimal_idx,
                "vector": opt_pol.get('vector', [])
            }
        else:
            optimal_payload = {"index": optimal_idx, "name": "Desconocida", "vector": []}

        return jsonify({"status": "ok", "result": policies_result, "optimal_policy": optimal_payload, "N": N})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/PL', methods=['POST'])
def programacion_lineal():
    try:
        data = request.get_json()
        cargar_datos(data)

        # Ejecutar C
        ptr = lib.Create_MPL()

        if not ptr:
            return jsonify({"status": "error", "message": "FO() returned NULL"}), 500

        modelo = stringC_to_py(ptr)

        niveles = modelo.split("\n")
        variables = {}
        index = 0

        '''
        Min z = cT * x
        s.a.
        A_eq * x == b_eq
        '''
        patron = re.compile(r'([+-]?\s*\d+(?:\.\d+)?)\s*y_{\d+,\d+}')
        patron_variable = re.compile(r'y_{\d+,\d+}')
        patron_restricciones = re.compile(r'([+-]?\s*\d+(?:\.\d+)?(?:e[+-]?\d+)?)\s*\*?\s*y_{(\d+),(\d+)}')

        # Calcular todas variables en mapa
        n = data["N"]
        b = [.0] * n

        for match in patron_variable.finditer(niveles[1]):
            var = match.group(0)
            if var not in variables:
                variables[var] = index
                index += 1

        # Calcular FO
        C = [.0]*index
        A = [[.0] * index for _ in range(n)]

        ecuacion = niveles[0]
        l, right = ecuacion.split("=",1)
        right_norm = re.sub(r'([+\-])\s*y_', r'\1 1y_', right)
        right_norm = re.sub(r'^\s*y_', '1y_', right_norm)
        for match in patron_restricciones.finditer(right_norm):
                coef_str = match.group(1)
                row_i = match.group(2)
                col_i = match.group(3)

                var_key = f"y_{{{row_i},{col_i}}}"

                if var_key in variables:
                    columna = variables[var_key] 
                    coef = float(coef_str.replace(' ', ''))
                    C[columna] = coef
                else:
                    print(f"⚠️ [Simplex] {var_key} no aparece en la primera restricción. Se omite.")
        
        # Restricciones
        for i in range(1,n+1):
            ecuacion = niveles[i]
            left, right = ecuacion.split('=', 1)
            try:
                b[i-1] = float(right.strip())
            except ValueError:
                b[i-1] = 0.0
            
                
            left_norm = re.sub(r'([+\-])\s*y_', r'\1 1y_', left)
            left_norm = re.sub(r'^\s*y_', '1y_', left_norm)
            for match in patron_restricciones.finditer(left_norm):
                coef_str = match.group(1)
                row_i = match.group(2)
                col_i = match.group(3)

                var_key = f"y_{{{row_i},{col_i}}}"

                if var_key in variables:
                    columna = variables[var_key] 
                    coef = float(coef_str.replace(' ', ''))
                    A[i-1][columna] = coef
                else:
                    print(f"⚠️ [Simplex] {var_key} no aparece en la primera restricción. Se omite.")
                
        resultado = linprog(C, A_eq=A, b_eq=b, method='highs')

        if not resultado.success:
            print("No es posible calcular Simplex.")
            return jsonify({"status": "error", "message": "No es posible calcular el método Simplex"}), 500
        
        X = [float(valor) for valor in resultado.x]
        D = []
        sumaEstados = [.0]*(n)

        llaves = []
        for key, value in variables.items():
            fila, col = map(int, key[3:-1].split(","))
            llaves.append(key)
            sumaEstados[fila]+=X[value]
        
        for i in range(len(X)):
            key = llaves[i]
            fila, col = map(int, key[3:-1].split(","))
            D.append(int(X[i] / sumaEstados[fila]))
        P = [0]*(n)
        i = 0
        for d in D:
            if d:
                key = llaves[i]
                l,r = map(int, key[3:-1].split(","))
                P[l]=r
            i+=1
        lib.free_global_matrices()
        # Faltan resultados imprimir yik y convertir a Dik  
        return jsonify({"status": "ok", "model": modelo, "yi": X, "Di": D, "P":P})
    except Exception as e:
        print(f"💥 CRASH EN /PL: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

def general_solver(c_func, data, *args):
    cargar_datos(data)
    print("Ok, aqui:", *args)
    ptr = c_func(*args)
    print("Pase lo critico")
    matrix = matrixC_to_py(ptr) 
    lib.free_global_matrices()   
    return jsonify({"status": "ok", "matrix": matrix})

@app.route('/MP', methods=['POST'])
def mejoramiento_politicas():
    try:
        data = request.get_json()
        pol_idx = int(data.get('politica_inicial', 0))
        return general_solver(lib.Mejoramiento_Politicas, data, pol_idx+1)
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/MPD', methods=['POST'])
def mejoramiento_descuento():
    try:
        data = request.get_json()
        pol_idx = int(data.get('politica_inicial', 0))
        factor = float(data.get('factor', 1.0))
        return general_solver(lib.MPD, data, pol_idx+1, factor)
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/AS', methods=['POST'])
def aproximaciones_sucesivas():
    try:
        data = request.get_json()
        tol = float(data.get('tolerancia', 1e-6))
        max_iter = int(data.get('max_iteraciones', 1000))
        alpha = float(data.get('alpha', 1.0))
        return general_solver(lib.AS, data, tol, max_iter, alpha)
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, host='127.0.0.1', port=5000)