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
            <span>[${pol.vector.join(', ')}]</span>
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
    
    title.textContent = `Parámetros: ${getMethodName(method)}`;
    body.innerHTML = buildFormHTML(method);
    
    modal.classList.remove('hidden');
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

    // Listener del formulario DENTRO del modal (usando delegación de eventos)
    document.getElementById('modal-overlay').addEventListener('submit', (e) => {
        e.preventDefault();
        const form = e.target;
        const method = form.dataset.method; // Guardamos el método en el form
        const params = {};
        for (let [key, value] of new FormData(form)) {
            // Convierte a número si es válido
            params[key] = (!isNaN(value) && value.trim() !== '') ? Number(value) : value;
        }
        
        closeModal(); // Cierra el pop-up
        executeMethod(method, params); // Ejecuta y muestra resultado
    });
}

function buildFormHTML(method) {
    const state = JSON.parse(localStorage.getItem(STORAGE_KEY));
    let html = `<form data-method="${method}">`;

    // Selector de Política Inicial (MP, MPD)
    if (['MP', 'MPD'].includes(method)) {
        html += `<div class="form-group"><label>Política Inicial</label><select name="politica_inicial">`;
        state.politicas.forEach((p, i) => {
            html += `<option value="${i}">P<sub>${i}</sub> = (${p.vector.join(', ')})</option>`;
        });
        html += `</select></div>`;
    }

    // Factor Descuento (MPD)
    if (method === 'MPD') {
        html += `<div class="form-group"><label>Factor (0-1)</label><input type="number" name="factor" value="0.95" step="0.01" min="0" max="1" required></div>`;
    }

    // Parámetros AS
    if (method === 'AS') {
        html += `<div class="form-group"><label>Tolerancia</label><input type="number" name="tolerancia" value="1e-6" step="any" required></div>`;
        html += `<div class="form-group"><label>Max Iteraciones</label><input type="number" name="max_iteraciones" value="100" required></div>`;
        html += `<div class="form-group"><label>Alpha</label><input type="number" name="alpha" value="1.0" step="0.1" required></div>`;
    }

    html += `<button type="submit" class="btn-submit">Calcular y Ver Resultado</button></form>`;
    return html;
}


function getMethodName(key) {
    return { 'EEP':'Enumeración Exhaustiva', 'PL':'Programación Lineal', 'MP':'Mejoramiento de Políticas', 'MPD':'Mejoramiento con Descuento', 'AS':'Aproximaciones Sucesivas' }[key];
}

function renderResults(method, data) {
    const container = document.getElementById('results-panel');
    container.innerHTML = `<h3 style="margin-bottom:1rem; border-bottom:1px solid; padding-bottom:0.5rem;">Resultados: ${getMethodName(method)}</h3>`;

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
        container.innerHTML += `<div style="text-align:center;"><p>El modelo generado es:</p><pre style="background:#0f172a; padding:1rem; border-radius:8px; text-align:left; overflow:auto;">${data.model}</pre><div style="margin-top:1rem; padding:0.5rem 1rem; background:#065f46; border-radius:20px; display:inline-block;">📍 CDMX: ${data.weather}</div></div>`;
    } else {
        data.matrix.data.forEach((row, i) => {
            container.innerHTML += `<div style="padding:0.8rem; border-bottom:1px dashed #334155;"><strong>Iteración ${i}:</strong> [${row.map(v=>v.toFixed(4)).join(', ')}]</div>`;
        });
    }
    
}
