const STORAGE_KEY = 'mdp_wizard_state';
const METODOS_ESPECIALES = ['MP', 'MPD', 'AS'];

document.addEventListener('DOMContentLoaded', () => {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
        window.location.href = '/';
        return;
    }

    loadAndDisplayData();
    setupMethodButtons();
    setupModalListeners();
    setupEditLinks();
});

function loadAndDisplayData() {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
        window.location.href = '/';
        return;
    }

    try {
        const state = JSON.parse(raw);
        console.log('📋 [DATA] Estado:', { N: state.N, K: state.K, P: state.P, policies: state.politicas.length });
        // Config básica
        document.getElementById('summaryN').textContent = state.N || '-';
        document.getElementById('summaryK').textContent = state.K || '-';
        document.getElementById('summaryP').textContent = state.P || '-';
        document.getElementById('summaryTipo').textContent = 
            state.tipo ? (state.tipo === 'min' ? 'Minimizar' : 'Maximizar') : '-';

        // Matriz de costos
        renderCostMatrix(state);
        // Políticas
        renderPolicies(state);
        // Resumen de transiciones
        document.getElementById('transitionSummary').textContent = 
            `${state.K || 0} matriz${(state.K || 0) !== 1 ? 'es' : 's'} de ${state.N || 0}x${state.N || 0}`;

    } catch (e) {
        console.error('Error cargando datos:', e);
        alert('❌ Error al cargar los datos guardados');
    }
}

function renderCostMatrix(state) {
    const table = document.getElementById('costMatrixTable');
    if (!table) { console.warn('⚠️ [DOM] #costMatrixTable no existe'); return; }
    const thead = table.querySelector('thead');
    const tbody = table.querySelector('tbody');
    
    thead.innerHTML = '';
    tbody.innerHTML = '';

    if (!state.costos || !state.K || !state.N) return;

    // Header
    let headerRow = '<tr><th>Edo. \\ Dec.</th>';
    for (let k = 0; k < state.K; k++) {
        headerRow += `<th>D${k + 1}</th>`;
    }
    headerRow += '</tr>';
    thead.innerHTML = headerRow;

    // Body
    for (let i = 0; i < state.N; i++) {
        let row = `<tr><td><strong>E${i}</strong></td>`;
        for (let k = 0; k < state.K; k++) {
            const val = state.costos[i][k];
            const display = (val === '' || val === null || val === undefined) ? '-' : 
                           (typeof val === 'number' && val > 1e100) ? '∞' : val;
            row += `<td>${display}</td>`;
        }
        row += '</tr>';
        tbody.innerHTML += row;
    }
}

function renderPolicies(state) {
    const container = document.getElementById('policiesList');
    if (!container) { console.warn('⚠️ [DOM] #policiesList no existe'); return; }
    container.innerHTML = '';

    if (!state.politicas || state.politicas.length === 0) {
        container.innerHTML = '<span class="text-muted">No hay políticas definidas</span>';
        return;
    }

    state.politicas.forEach((pol, idx) => {
        const chip = document.createElement('div');
        chip.className = 'policy-chip';
        chip.innerHTML = `
            <strong>${pol.name || `P<sub>${idx + 1}</sub>`}</strong>
            <span>(${pol.vector.join(', ')})</span>
        `;
        container.appendChild(chip);
    });
}

function setupEditLinks() {
    document.querySelectorAll('.edit-link').forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const target = this.dataset.target;
            
            localStorage.setItem('mdp_edit_target', target);
            
            window.location.href = '/';
        });
    });

    document.getElementById('btnEditConfig').addEventListener('click', () => {
        localStorage.setItem('mdp_edit_target', 'config');
        window.location.href = '/';
    });
}

function setupMethodButtons() {
    document.querySelectorAll('.method-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const method = btn.dataset.method;
            console.log(`🖱️ [CLICK] Método: ${method} | ¿Requiere params?: ${METODOS_ESPECIALES.includes(method) ? 'SÍ → modal' : 'NO → auto'}`);
            if (!METODOS_ESPECIALES.includes(method)) {
                executeMethod(method, {}); 
            } 
            else {
                openModal(method);
            }
        });
    });
}

function setupFactorInterestSync() {
    const alphaInput = document.getElementById('input-alpha');
    const interestInput = document.getElementById('input-interest');
    
    if (!alphaInput || !interestInput) return;

    const PRECISION = 4;

    // α → i:  i = (1/α) - 1
    const alphaToInterest = (alpha) => {
        const a = parseFloat(alpha);
        if (isNaN(a) || a <= 0.001 || a > 1) return '';
        return ((1 / a) - 1).toFixed(PRECISION);
    };

    // i → α:  α = 1 / (1 + i)
    const interestToAlpha = (interest) => {
        const i = parseFloat(interest);
        if (isNaN(i) || i < 0) return '';
        const alpha = 1 / (1 + i);
        return Math.min(Math.max(alpha, 0.001), 1).toFixed(PRECISION);
    };

    let isUpdating = false;
    
    alphaInput.addEventListener('input', () => {
        if (isUpdating) return;
        isUpdating = true;
        
        const alpha = alphaInput.value.trim();
        if (alpha === '' || alpha.endsWith('.')) {
            isUpdating = false;
            return;
        }
        
        const interest = alphaToInterest(alpha);
        if (interest && interest !== '') {
            interestInput.value = interest;
        }
        
        setTimeout(() => { isUpdating = false; }, 30);
    });

    interestInput.addEventListener('input', () => {
        if (isUpdating) return;
        isUpdating = true;
        
        const interest = interestInput.value.trim();
        if (interest === '' || interest.endsWith('.')) {
            isUpdating = false;
            return;
        }
        
        const alpha = interestToAlpha(interest);
        if (alpha && alpha !== '') {
            alphaInput.value = Number(alpha).toFixed(3);
        }
        
        setTimeout(() => { isUpdating = false; }, 30);
    });
    
    if (alphaInput.value) {
        interestInput.value = alphaToInterest(alphaInput.value);
    }
}

function executeMethod(method, userParams) {
    const state = JSON.parse(localStorage.getItem(STORAGE_KEY));
    const payload = { ...state, ...userParams }; // Combina datos guardados + inputs del modal

    console.log(`📡 [FETCH] POST /${method} | Payload keys:`, Object.keys(payload), '| Params extra:', Object.keys(userParams));
    
    const resultsPanel = document.getElementById('results-panel');
    resultsPanel.classList.remove('hidden');
    resultsPanel.innerHTML = `<div class="text-center py-4">
        <div class="spinner-border text-primary"></div>
        <p class="mt-2">Procesando en C...</p>
    </div>`;

    // Scroll suave hacia los resultados
    resultsPanel.scrollIntoView({ behavior: 'smooth' });

    fetch(`/${method}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    })
    .then(res => res.json())
    .then(data => {
        if (data.status === 'error') throw new Error(data.message);
        renderResults(method, data);
    })
    .catch(err => {
        console.error(err);
        resultsPanel.innerHTML = `<div class="alert alert-danger" style="border:1px solid #ef4444; padding:1rem; border-radius:8px;">❌ ${err.message}</div>`;
    });
}

function openModal(method) {
    const modal = document.getElementById('modal-overlay');
    const title = document.getElementById('modal-title');
    const body = document.getElementById('modal-body');
    
    title.textContent = `Parámetros requeridos:`;
    body.innerHTML = buildFormHTML(method);
    
    modal.classList.remove('hidden');

    if (method === 'MPD') {
        setTimeout(setupFactorInterestSync, 100); // Pequeño delay para que el DOM esté listo
    }
}

function closeModal() {
    document.getElementById('modal-overlay').classList.add('hidden');
}

function setupModalListeners() {
    // Botón X para cerrar
    document.getElementById('btn-close-modal').addEventListener('click', closeModal);
    
    // Clic fuera del modal para cerrar
    document.getElementById('modal-overlay').addEventListener('click', (e) => {
        if (e.target.id === 'modal-overlay') closeModal();
    });

    // Listener del formulario DENTRO del modal
    document.getElementById('modal-overlay').addEventListener('submit', (e) => {
        e.preventDefault();
        const form = e.target;
        const method = form.dataset.method; 
        const params = {};
        
        for (let [key, value] of new FormData(form)) {
            if (method === 'MPD') {
                if (key === 'alpha') {
                    const num = parseFloat(value);
                    params.alpha = isNaN(num) ? 1 : num;
                }
                continue;
            }
            params[key] = (!isNaN(value) && value.trim() !== '') ? Number(value) : value;
        }
        
        // Validación final de rango para alpha
        if (method === 'MPD' && (!params.alpha || params.alpha < 0.001 || params.alpha > 1)) {
            alert('⚠️ El factor α debe estar entre 0.001 y 1');
            return;
        }
        closeModal();
        executeMethod(method, params); 
    });
}

const toSubscript = (num) => {
    const map = {'0':'₀','1':'₁','2':'₂','3':'₃','4':'₄','5':'₅','6':'₆','7':'₇','8':'₈','9':'₉'};
    return num.toString().split('').map(c => map[c] || c).join('');
};

function buildFormHTML(method) {
    const state = JSON.parse(localStorage.getItem(STORAGE_KEY));
    let html = `<form data-method="${method}" novalidate>`;

    // Selector de Política Inicial (MP, MPD)
    if (['MP', 'MPD'].includes(method)) {
        html += `<div class="form-group"><label>Política Inicial</label><select name="politica_inicial">`;
        state.politicas.forEach((p, i) => {
            html += `<option value="${i}">P${toSubscript(i+1)} = (${p.vector.join(', ')})</option>`;
        });
        html += `</select></div>`;
    }

    // Factor Descuento (MPD)
    if (method === 'MPD') {
        html += `
        <div class="form-row">
            <div class="form-col">
                <label for="input-alpha">Factor α (descuento)</label>
                <input type="number" id="input-alpha" name="alpha" value="1" step="any" min="0" max="10">
                <small class="hint">Sugerido: α ∈ (0, 1]</small>
            </div>
            <div class="form-col">
                <label for="input-interest">Tasa de interés i</label>
                <input type="number" id="input-interest" value="0" step="any" min="-0.999">
                <small class="hint">i = (1/α) - 1</small>
            </div>
        </div>
        `;
    }

    // Parámetros AS
    if (method === 'AS') {
        html += `<div class="form-group"><label>Tolerancia</label><input type="number" name="tolerancia" value="0.001" step="any" required></div>`;
        html += `<div class="form-group"><label>Max Iteraciones</label><input type="number" name="max_iteraciones" value="10" required></div>`;
        html += `<div class="form-group"><label>Alpha</label><input type="number" name="alpha" value="1" step="0.1" required></div>`;
    }

    html += `<button type="submit" class="btn-submit">Calcular y Ver Resultado</button></form>`;
    return html;
}


function getMethodName(key) {
    return { 'EEP':'Enumeración Exhaustiva de Políticas', 'PL':'Programación Lineal', 'MP':'Mejoramiento de Políticas', 'MPD':'Mejoramiento de Políticas con Descuento', 'AS':'Aproximaciones Sucesivas' }[key];
}

function renderResults(method, data) {
    const container = document.getElementById('results-panel');
    container.innerHTML = `<h3 style="margin-bottom:1rem; border-bottom:1px solid; padding-bottom:0.5rem;">Método: ${getMethodName(method)}</h3>`;

    if (method === 'EEP' && data.optimal_policy) {
        const vecStr = data.optimal_policy.vector.join(', ');
        data.result.forEach((pol, i) => {
            const piStr = pol.vector.map((v, j) => `π<sub>${j}</sub> = ${v.toFixed(4)}`).join(' &nbsp;|&nbsp; ');
            container.innerHTML += `<div style="padding:1rem; border-radius:8px; margin-bottom:1rem; border-left:3px solid var(--accent);">
                <strong>Política ${i+1}:</strong> (${pol.original.join(', ')})<br>
                <strong>Costo:</strong> ${pol.cost.toFixed(4)}<br>
                <div style="font-family:monospace; margin-top:0.5rem;">${piStr}</div>
            </div>`;
        });

        container.innerHTML += `
                <div style="margin-top: 1.5rem; padding: 1rem; background: #065f46; border-radius: 8px; color: white; text-align: center; border: 1px solid #10b981;">
                    <h4 style="margin:0;">La política óptima es: P<sub>${data.optimal_policy.index}</sub> = (${vecStr})</h4>
                </div>
            `;
    } else if (method === 'PL') {
        const vecStr = data.optimal_policy.join(', ');
        const formatKey = (key, letra) => key.replace(/([a-zA-Z])_\{(\d+)\s*,\s*(\d+)\}/g, (_, l, i, k) => `${letra}<sub>${i}${k}</sub>`);
        let variablesHTML = `<div style="padding: 0.6rem 0.8rem; border-radius: 6px; font-family: monospace; border-left: 3px solid #3b82f6;">`;
        
        data.yi.forEach((val, i) => {
            const rawKey = data.llaves?.[i] || `y_${i},?`;
            const displayVal = typeof val === 'number' ? val.toFixed(4) : val;
            variablesHTML += `<span style="margin-right: 2em;">${formatKey(rawKey, "y")} = ${displayVal}</span>`;
        })
        variablesHTML += `</div>
        <div style="padding: 0.6rem 0.8rem; border-radius: 6px; font-family: monospace; border-left: 3px solid #f59e0b;">
        `
        data.Di.forEach((val, i) => {
            const rawKey = data.llaves?.[i] || `D_${i},?`;
            const displayVal = typeof val === 'number' ? val.toFixed(4) : val;
            variablesHTML += `<span style="margin-right: 2em;"> ${formatKey(rawKey, "D")} = ${displayVal}</span>`
        })
        variablesHTML += `</div>`

        container.innerHTML += `<div style="text-align:center;">
        <h4>El modelo generado es:</h4>
        <pre style="font-size: 1.1rem; border-radius:8px; text-align:center; overflow:auto; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;">
            ${data.model.replace(/y_\{(\d+)\s*,\s*(\d+)\}/g, (m, row, col) => `y<sub>${row}${col}</sub>`).replace("y_{i,k}>=",`y<sub>ik</sub>≥` )}
        </pre>
        </div>

        ${variablesHTML}
        
        <div style="margin-top: 1.5rem; padding: 1rem; background: #065f46; border-radius: 8px; color: white; text-align: center; border: 1px solid #10b981;">
            <h4 style="margin:0;">La política óptima es: P = (${vecStr})</h4>
        </div>
        `;

        
    } else if (method === "AS") {
        let vecStr = "";
        const rowCount = data.matrix.rows;
        const rawData = data.matrix.data || [];
        let j = 1;
        for (let i = 0; i < rowCount-2; i += 2) {
            const iterIndex = j++;
            const Vi = rawData[i] || [];
            const Policy = rawData[i + 1] || [];

            const viStr = Vi.map(v => typeof v === 'number' ? (Number.isInteger(v) ? v : v.toFixed(3)) : v).join(', ');
            const polStr = Policy.join(', ');
            vecStr = viStr

            container.innerHTML += `
            <div style="
                display: flex; flex-wrap: wrap; align-items: center; gap: 0.5rem;
                padding: 0.35rem 0.75rem; border-bottom: 1px dashed #e2e8f0;
                font-family: 'SF Mono', 'Consolas', monospace; font-size: 0.85rem;
            ">
                <span style="
                    background: #3b82f6; color: white; padding: 0.1rem 0.4rem;
                    border-radius: 4px; font-weight: 600; font-size: 0.75rem;
                    min-width: 3.5rem; text-align: center;
                ">Iter ${iterIndex}</span>
                <span style="color: #64748b;">V:</span>
                <span style="color: #1e293b;">[${polStr}]</span>
                <span style="color: #cbd5e1; font-weight: 300;">│</span>
                <span style="color: #64748b;">D:</span>
                <span style="color: #059669; font-weight: 500;">(${viStr})</span>
            </div>`;
        }

        container.innerHTML += `<div style="margin-top: 1.5rem; padding: 1rem; background: #065f46; border-radius: 8px; color: white; text-align: center; border: 1px solid #10b981;">
                <h4 style="margin:0;">La política aproximada es: P = (${vecStr})</h4>
            </div>`
    } else {
        const vecStr = data.optimal_policy.join(', ');
        data.matrix.data.forEach((row, i) => {
            container.innerHTML += `
            <div style="padding:0.8rem; border-bottom:1px dashed #334155;">
            <strong>Iteración ${i}:</strong> 
            (${row.join(', ')})
            </div>`;
        });
        container.innerHTML += `<div style="margin-top: 1.5rem; padding: 1rem; background: #065f46; border-radius: 8px; color: white; text-align: center; border: 1px solid #10b981;">
                <h4 style="margin:0;">La política óptima es: P = (${vecStr})</h4>
            </div>`
    }
    
}
