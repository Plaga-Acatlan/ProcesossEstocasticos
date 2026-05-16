const STORAGE_KEY = 'mdp_wizard_state';
function saveState() {
    try {
        // Detectar paso activo
        const active = document.querySelector('.step-section[style*="display: block"]');
        const currentStep = active ? active.id : 'step-config';

        const payload = {
            ...state,
            currentStep,
            // Asegurar que no se guarden referencias circulares o funciones
            transitionMatrices: JSON.parse(JSON.stringify(state.transitionMatrices)),
            costMatrix: JSON.parse(JSON.stringify(state.costMatrix)),
            policies: JSON.parse(JSON.stringify(state.policies))
        };
        localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
    } catch (e) {
        console.warn('⚠️ Error guardando en localStorage:', e);
    }
}

function loadState() {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return false;
    
    try {
        const saved = JSON.parse(raw);
        
        // Restaurar propiedades primitivas y arrays
        Object.keys(state).forEach(key => {
            if (saved[key] !== undefined) {
                state[key] = saved[key];
            }
        });

        // Valores por defecto seguros
        state.currentMatrix = state.currentMatrix ?? 0;
        if (!Array.isArray(state.transitionMatrices)) state.transitionMatrices = [];
        if (!Array.isArray(state.costMatrix)) state.costMatrix = [];
        if (!Array.isArray(state.policies)) state.policies = [];

        // Reconstruir UI según el paso guardado
        const step = saved.currentStep || 'step-config';
        showSection(`#${step}`);

        if (step === 'step-transition') buildTransitionMatrixUI();
        else if (step === 'step-costs') buildCostMatrixUI();
        else if (step === 'step-policies') {
            buildPolicyUI();
            renderPoliciesList();
        }
        return true;
    } catch (e) {
        console.warn('⚠️ Estado corrupto en localStorage. Se reiniciará.', e);
        localStorage.removeItem(STORAGE_KEY);
        return false;
    }
}

function clearState() {
    // Solo limpiar cuando se envíe exitosamente, NO al navegar
    localStorage.removeItem(STORAGE_KEY);
    localStorage.removeItem('mdp_edit_target');
    
    // Resetear variables de estado
    state.K = 0;
    state.N = 0;
    state.P = 0;
    state.tipo = '';
    state.currentMatrix = 0;
    state.transitionMatrices = [];
    state.costMatrix = [];
    state.policies = [];
}

const DBL_MAX = 1.7976931348623157e+308;   // DBL_MAX de C
const ROW_SUM_TOLERANCE = 1e-6;
const state = {
    K: 0,
    N: 0,
    P: 0, 
    tipo: '',
    transitionMatrices: [],
    costMatrix: [],
    policies: [],
};


const $ = (sel) => document.querySelector(sel);
const $$ = (sel) => document.querySelectorAll(sel);

function isPositiveInt(v) {
    return Number.isInteger(v) && v > 0;
}

function showSection(id) {
    document.querySelectorAll('.step-section').forEach(s => s.style.display = 'none');
    $(id).style.display = 'block';
}

function showModal(title, body, isError = false) {
    $('#resultModalTitle').textContent = title;
    $('#resultModalBody').innerHTML = body;
    $('#resultModalBody').className = 'modal-body ' + (isError ? 'text-danger' : 'text-success');
    new bootstrap.Modal($('#resultModal')).show();
}

function parseFraction(str) {
    if (str == null || str.toString().trim() === '') return null;
    str = str.toString().trim().replace(/\s/g, '');
    
    if (str.includes('/')) {
        const parts = str.split('/');
        if (parts.length !== 2) return NaN;
        // Validar que numerador y denominador sean números válidos
        if (!/^-?\d+(\.\d+)?$/.test(parts[0]) || !/^-?\d+(\.\d+)?$/.test(parts[1])) {
            return NaN;
        }
        const num = parseFloat(parts[0]);
        const den = parseFloat(parts[1]);
        if (den === 0) return NaN;
        return num / den;
    }
    // Validar formato decimal/entero
    if (!/^-?\d+(\.\d+)?$/.test(str)) return NaN;
    const val = parseFloat(str);
    return isNaN(val) ? NaN : val;
}


function highlightCell(input, isTransition = true) {
    const val = parseFraction(input.value);
    if (val === null) { input.classList.remove('cell-invalid'); return; }
    
    const isBadFormat = isNaN(val);
    const isNegative = isTransition && val < 0;
    
    if (isBadFormat || isNegative) {
        input.classList.add('cell-invalid');
    } else {
        input.classList.remove('cell-invalid');
    }
}

/* ==============================================================
   PASO 1 — Configuración
   ============================================================== */
$('#configForm').addEventListener('submit', function (e) {
    e.preventDefault();

    const K = parseInt($('#numDecisiones').value, 10);
    const N = parseInt($('#numEstados').value, 10);
    const P = parseInt($('#numPoliticas').value, 10);
    const tipo = $('#tipoOptimizacion').value;

    if (!isPositiveInt(K) || !isPositiveInt(N) || !isPositiveInt(P)) {
        alert('Decisiones, Estados y Políticas deben ser enteros positivos.');
        return;
    }
    if (!tipo) {
        alert('Seleccione un tipo de optimización (Min o Max).');
        return;
    }

    state.K = K;
    state.N = N;
    state.P = P;
    state.tipo = tipo;
    state.currentMatrix = 0;

    // Inicializar matrices vacías
    state.transitionMatrices = Array.from({ length: K }, () =>
        Array.from({ length: N }, () => Array(N).fill(''))
    );
    state.costMatrix = Array.from({ length: N }, () => Array(K).fill(''));

    saveState();
    buildTransitionMatrixUI();
    showSection('#step-transition');
});

/* ==============================================================
   PASO 2 — Matrices de Transición
   ============================================================== */
function buildTransitionMatrixUI() {
    // 🛡️ Validación defensiva antes de construir
    if (!state.K || !state.N) {
        console.warn('⚠️ Estado incompleto. Regresando a configuración.');
        showSection('#step-config');
        return;
    }
    if (!Array.isArray(state.transitionMatrices) || state.transitionMatrices.length !== state.K) {
        state.transitionMatrices = Array.from({ length: state.K }, () =>
            Array.from({ length: state.N }, () => Array(state.N).fill(''))
        );
    }
    if (state.currentMatrix === undefined || state.currentMatrix < 0 || state.currentMatrix >= state.K) {
        state.currentMatrix = 0;
    }

    const container = $('#transitionMatricesContainer');
    container.innerHTML = '';

    let html = `<table class="table table-bordered text-center align-middle">
                    <thead><tr><th></th>`;
    for (let j = 0; j < state.N; j++) html += `<th>Estado ${j}</th>`;
    html += `<th>Σ</th></tr></thead><tbody>`;

    for (let i = 0; i < state.N; i++) {
        html += `<tr data-row="${i}"><td><strong>Estado ${i}</strong></td>`;
        for (let j = 0; j < state.N; j++) {
            // Acceso seguro
            const val = state.transitionMatrices[state.currentMatrix]?.[i]?.[j] ?? '';
            html += `<td><input type="text" class="matrix-cell"
                        data-k="${state.currentMatrix}" data-i="${i}" data-j="${j}"
                        value="${val}" placeholder="1/2 o 0.5"></td>`;
        }
        html += `<td class="row-sum fw-bold">—</td></tr>`;
    }
    container.innerHTML = html + `</tbody></table>`;

    // Listeners con guardado automático en cada tecla
    container.querySelectorAll('.matrix-cell').forEach(inp => {
        inp.addEventListener('input', function() {
            const k = parseInt(this.dataset.k);
            const i = parseInt(this.dataset.i);
            const j = parseInt(this.dataset.j);
            state.transitionMatrices[k][i][j] = this.value;
            highlightCell(this, true);
            validateRow(i);
            saveState(); // 💾 Guarda automáticamente al escribir
        });
    });

    updateMatrixLabel();
    updateNavigationButtons();
    validateAllRows();
}

function updateMatrixLabel() {
    $('#transitionLabel').textContent = `(${state.currentMatrix + 1} de ${state.K})`;
    $('#matrixCounter').textContent = `Matriz ${state.currentMatrix + 1} / ${state.K}`;
}

function updateNavigationButtons() {
    $('#btnPrevMatrix').disabled = (state.currentMatrix === 0);

    if (state.currentMatrix === state.K - 1) {
        $('#btnNextMatrix').style.display = 'none';
        $('#btnFinishTransitions').classList.remove('d-none');
    } else {
        $('#btnNextMatrix').style.display = 'inline-block';
        $('#btnFinishTransitions').classList.add('d-none');
    }
}

function attachRowSumListeners() {
    $$('#transitionMatricesContainer input').forEach(input => {
        input.addEventListener('input', function () {
            const k = parseInt(this.dataset.k);
            const i = parseInt(this.dataset.i);
            const j = parseInt(this.dataset.j);
            state.transitionMatrices[k][i][j] = this.value;
            validateRow(i);
        });
    });
}

function validateRow(i) {
    const tr = $(`#transitionMatricesContainer tr[data-row="${i}"]`);
    const inputs = tr.querySelectorAll('.matrix-cell');
    let sum = 0;
    let allFilled = true;

    inputs.forEach(inp => {
        const v = inp.value.trim();
        if (v === '') { allFilled = false; return; }
        const num = parseFraction(v);
        sum += num;
    });

    const sumCell = tr.querySelector('.row-sum');

    if (!allFilled) {
        tr.classList.remove('row-valid', 'row-invalid');
        sumCell.textContent = '—';
        return;
    }

    sumCell.textContent = sum.toFixed(4);

    const allInRange = Array.from(inputs).every(inp => {
        const val = parseFraction(inp.value);
        return val !== null && !isNaN(val) && val >= 0 && val <= 1 + ROW_SUM_TOLERANCE;
    });

    if (Math.abs(sum - 1.0) <= ROW_SUM_TOLERANCE && allInRange) {
        tr.classList.add('row-valid');
        tr.classList.remove('row-invalid');
    } else {
        tr.classList.add('row-invalid');
        tr.classList.remove('row-valid');
    }
}

function validateAllRows() {
    for (let i = 0; i < state.N; i++) {
        validateRow(i);
    }
}

// Botones de navegación
$('#btnPrevMatrix').addEventListener('click', () => {
    if (state.currentMatrix > 0) {
        state.currentMatrix--;
        saveState(); // ← Guardar cambio de matriz
        buildTransitionMatrixUI();
    }
});

$('#btnNextMatrix').addEventListener('click', () => {
    if (state.currentMatrix < state.K - 1) {
        state.currentMatrix++;
        saveState(); // ← Guardar cambio de matriz
        buildTransitionMatrixUI();
    }
});

$('#btnFinishTransitions').addEventListener('click', () => {
    // Validación previa obligatoria
    const errors = [];
    for (let i = 0; i < state.N; i++) {
        const tr = $(`#transitionMatricesContainer tr[data-row="${i}"]`);
        const inputs = tr.querySelectorAll('.matrix-cell');
        let sum = 0, rowHasError = false;
        
        inputs.forEach(inp => {
            const val = parseFraction(inp.value);
            if (val === null) { rowHasError = true; return; }
            if (isNaN(val)) { rowHasError = true; return; }
            if (val < 0 || val > 1) { rowHasError = true; return; }
            sum += val;
        });
        
        if (rowHasError || Math.abs(sum - 1.0) > ROW_SUM_TOLERANCE) {
            errors.push(`Fila ${i}: suma=${sum.toFixed(4)} o valores inválidos`);
            tr.classList.add('row-invalid');
        }
    }
    
    if (errors.length > 0) {
        alert('⚠️ Corrige los errores en las matrices antes de continuar:\n' + errors.join('\n'));
        return; // ← Detiene la navegación
    }
    
    saveState();
    buildCostMatrixUI();
    showSection('#step-costs');
});

/* ==============================================================
   PASO 3 — Matriz de Costos
   ============================================================== */
function buildCostMatrixUI() {
    const table = $('#costTable');
    let html = `<thead><tr><th></th>`;
    for (let k = 0; k < state.K; k++) html += `<th>Decisión ${k + 1}</th>`;
    html += `</tr></thead><tbody>`;

    for (let i = 0; i < state.N; i++) {
        html += `<tr><td><strong>Estado ${i}</strong></td>`;
        for (let k = 0; k < state.K; k++) {
            const val = state.costMatrix[i][k];
            const display = (val === '' || val === null) ? '' : val;
            html += `<td><input type="text" class="cost-cell" data-i="${i}" data-k="${k}"
                        value="${display}" placeholder="-"></td>`;
        }
        html += `</tr>`;
    }
    table.innerHTML = html + `</tbody>`;

    table.querySelectorAll('.cost-cell').forEach(inp => {
        inp.addEventListener('input', function() {
            highlightCell(this, false); 
            const i=parseInt(this.dataset.i), k=parseInt(this.dataset.k);
            state.costMatrix[i][k] = this.value;
            saveState();
        });
    });
}

$('#btnGoPolicies').addEventListener('click', () => {
    // Finalizar costos vacíos con DBL_MAX
    for (let i = 0; i < state.N; i++)
        for (let k = 0; k < state.K; k++)
            if (state.costMatrix[i][k] === '') state.costMatrix[i][k] = DBL_MAX;
    saveState(); // ← NUEVO
    buildPolicyUI();
    showSection('#step-policies');
});

/* ==============================================================
   PASO 4 — Políticas
   ============================================================== */
function buildPolicyUI() {
    $('#policySizeLabel').textContent = state.N;
    const row = $('#policyInputRow'); row.innerHTML = '';
    for (let i = 0; i < state.N; i++) {
        row.innerHTML += `<input type="number" class="form-control policy-input" id="polVec_${i}" min="0" placeholder="E${i}">`;
    }
    renderPoliciesList();
}

$('#btnAddPolicy').addEventListener('click', () => {
    const vector = [];
    for (let i = 0; i < state.N; i++) {
        const inp = $(`#polVec_${i}`), v = parseInt(inp.value, 10);
        if (isNaN(v)) { alert(`⚠️ El valor del Estado ${i} no es un entero válido.`); return; }
        vector.push(v); inp.value = '';
    }
    // Nombre automático 1-indexado: P_1, P_2...
    const name = `P${state.policies.length + 1}`;
    state.policies.push({ name, vector });
    renderPoliciesList();
    saveState();
});


function renderPoliciesList() {
    const list = $('#policiesList');
    list.innerHTML = '';

    if (state.policies.length === 0) {
        list.innerHTML = '<p class="text-muted">No hay políticas agregadas aún.</p>';
        return;
    }

    state.policies.forEach((pol, idx) => {
        list.innerHTML += `
            <div class="card policy-card mb-2">
                <div class="card-body d-flex align-items-center justify-content-between py-2">
                    <div><strong>${pol.name}</strong>: ( ${pol.vector.join(', ')} )</div>
                    <button class="btn btn-outline-danger btn-sm btn-remove" data-idx="${idx}" title="Eliminar">✕</button>
                </div>
            </div>`;
    });

    // Listeners para eliminar
    $$('.btn-remove').forEach(btn => {
        btn.addEventListener('click', function () {
            const idx = parseInt(this.dataset.idx);
            state.policies.splice(idx, 1);
            renderPoliciesList();
        });
    });
}

/* ==============================================================
   VALIDACIÓN FINAL
   ============================================================== */
$('#btnValidate').addEventListener('click', () => {
    const errors = [];

    // 1. Transiciones
    for (let k = 0; k < state.K; k++) {
        for (let i = 0; i < state.N; i++) {
            let sum = 0, hasError = false;
            for (let j = 0; j < state.N; j++) {
                const raw = state.transitionMatrices[k][i][j];
                const val = parseFraction(raw);
                if (val === null) { errors.push(`Transición Dec ${k+1}, Est ${i}→${j}: vacío.`); hasError=true; break; }
                if (isNaN(val))  { errors.push(`Transición Dec ${k+1}, Est ${i}→${j}: formato inválido ("${raw}").`); hasError=true; break; }
                if (val < 0)     { errors.push(`Transición Dec ${k+1}, Est ${i}→${j}: negativa (${val}).`); hasError=true; }
                sum += val;
            }
            if (!hasError && Math.abs(sum - 1.0) > ROW_SUM_TOLERANCE) {
                errors.push(`Transición Dec ${k+1}, fila ${i}: suma = ${sum.toFixed(4)} ≠ 1.`);
            }
        }
    }

    // 2. Costos (solo formato)
    for (let i = 0; i < state.N; i++) {
        for (let k = 0; k < state.K; k++) {
            const raw = state.costMatrix[i][k];
            if (raw !== '' && raw !== null) {
                const val = parseFraction(raw);
                if (isNaN(val)) errors.push(`Costo Dec ${k+1}, Est ${i}: formato inválido ("${raw}").`);
            }
        }
    }

    // 3. Políticas y Tipo
    if (state.policies.length !== state.P) errors.push(`Faltan políticas: se esperan ${state.P}, hay ${state.policies.length}.`);
    if (!['min','max'].includes(state.tipo)) errors.push('Tipo no válido.');
    
    state.policies.forEach((pol, idx) => {
        if (pol.vector.length !== state.N) errors.push(`${pol.name}: tamaño inválido.`);
        pol.vector.forEach((v, si) => {
            if (v < 1 || v > state.K) errors.push(`${pol.name}, Est ${si}: valor ${v} fuera de [1, ${state.K}].`);
        });
    });

    if (errors.length > 0) {
        let msg = '<ul class="mb-0">'; errors.forEach(e => msg += `<li>${e}</li>`); msg += '</ul>';
        showModal('❌ Validación Fallida', msg, true); return;
    }

    // ✅ Payload limpio con floats reales
    const payload = {
        K: state.K, N: state.N, P: state.P, tipo: state.tipo,
        transitionMatrices: state.transitionMatrices.map(mat =>
            mat.map(row => row.map(v => { const p=parseFraction(v); return isNaN(p)?0:p; }))
        ),
        costMatrix: state.costMatrix.map(row => 
            row.map(v => (v === '' || v === null) ? DBL_MAX : parseFraction(v))
        ),
        policies: state.policies,
    };

    fetch('/procesar', { 
        method: 'POST', 
        headers: { 'Content-Type': 'application/json' }, 
        body: JSON.stringify(payload) 
    })
    .then(r => {
        if (!r.ok) throw new Error(`HTTP ${r.status}`);
        return r.json();
    })
    .then(data => {
        // Guardar en localStorage antes de redirigir
        localStorage.setItem('mdp_wizard_state', JSON.stringify(payload));
        // Redirigir a página de procesamiento
        window.location.href = '/procesar';
    })
    .catch(err => {
        console.error('Error:', err);
        showModal('❌ Error al enviar', `<p>${err.message}</p>`, true);
    });
});

$('#btnBackToTransitions').addEventListener('click', () => {
    saveState();
    buildTransitionMatrixUI();
    showSection('#step-transition');
});

// De Políticas → Costos
$('#btnBackToCosts').addEventListener('click', () => {
    saveState();
    buildCostMatrixUI();
    showSection('#step-costs');
});

// De Transiciones → Config (con advertencia si hay datos)
$('#btnBackToConfig').addEventListener('click', () => {
    const hasData = state.transitionMatrices.some(mat => 
        mat.some(row => row.some(v => v !== ''))
    );
    if (hasData && !confirm('⚠️ Regresar a configuración borrará las matrices. ¿Continuar?')) {
        return;
    }
    if (hasData) {
        clearState();
    }
    showSection('#step-config');
});

document.addEventListener('DOMContentLoaded', () => {
    const loaded = loadState();
    if (loaded) {
        // Verificar si viene de "editar" algo específico
        const editTarget = localStorage.getItem('mdp_edit_target');
        if (editTarget) {
            localStorage.removeItem('mdp_edit_target'); // Limpiar solo esta flag
            
            // Navegar a la sección correspondiente
            switch(editTarget) {
                case 'config':
                    showSection('#step-config');
                    break;
                case 'transiciones':
                    buildTransitionMatrixUI();
                    showSection('#step-transition');
                    break;
                case 'costos':
                    buildCostMatrixUI();
                    showSection('#step-costs');
                    break;
                case 'politicas':
                    buildPolicyUI();
                    renderPoliciesList();
                    showSection('#step-policies');
                    break;
            }
        }
    }
});