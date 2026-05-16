const STORAGE_KEY = 'mdp_wizard_state';

document.addEventListener('DOMContentLoaded', () => {
    loadAndDisplayData();
    setupEditLinks();
});

function loadAndDisplayData() {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
        alert('⚠️ No hay datos guardados. Redirigiendo...');
        window.location.href = '/';
        return;
    }

    try {
        const state = JSON.parse(raw);
        
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
    const thead = table.querySelector('thead');
    const tbody = table.querySelector('tbody');
    
    thead.innerHTML = '';
    tbody.innerHTML = '';

    if (!state.costMatrix || !state.K || !state.N) return;

    // Header
    let headerRow = '<tr><th>Estado \\ Decisión</th>';
    for (let k = 0; k < state.K; k++) {
        headerRow += `<th>D${k + 1}</th>`;
    }
    headerRow += '</tr>';
    thead.innerHTML = headerRow;

    // Body
    for (let i = 0; i < state.N; i++) {
        let row = `<tr><td><strong>E${i}</strong></td>`;
        for (let k = 0; k < state.K; k++) {
            const val = state.costMatrix[i][k];
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
    container.innerHTML = '';

    if (!state.policies || state.policies.length === 0) {
        container.innerHTML = '<span class="text-muted">No hay políticas definidas</span>';
        return;
    }

    state.policies.forEach((pol, idx) => {
        const chip = document.createElement('div');
        chip.className = 'policy-chip';
        chip.innerHTML = `
            <strong>${pol.name || `P_${idx + 1}`}</strong>
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
            
            // Solo guardar a dónde ir, NO limpiar nada
            localStorage.setItem('mdp_edit_target', target);
            
            // Redirigir
            window.location.href = '/';
        });
    });

    document.getElementById('btnEditConfig').addEventListener('click', () => {
        localStorage.setItem('mdp_edit_target', 'config');
        window.location.href = '/';
    });
}