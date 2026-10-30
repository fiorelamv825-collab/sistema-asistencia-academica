
// ======================================
// SISTEMA DE ASISTENCIA - ACADEMIA
// Conectado con Flask y matriculados.csv
// ======================================

let estudiantes = [];
let cola = [];
let estudianteActual = null;

// ======================================
// CONEXION CON EL SERVIDOR
// ======================================

async function cargarEstudiantes() {
    try {
        const respuesta = await fetch("/api/estudiantes");

        if (!respuesta.ok) {
            throw new Error("No se pudieron cargar los estudiantes");
        }

        estudiantes = await respuesta.json();
        actualizar();

    } catch (error) {
        alert("No se pudo conectar con el servidor. Verifica que Flask esté ejecutándose.");
        console.error(error);
    }
}

// ======================================
// NAVEGACION
// ======================================

function mostrar(id, boton) {
    document.querySelectorAll(".pagina").forEach(pagina => {
        pagina.classList.remove("activa");
    });

    document.getElementById(id).classList.add("activa");

    document.querySelectorAll(".nav").forEach(b => {
        b.classList.remove("active");
    });

    if (boton) {
        boton.classList.add("active");
    } else {
        document.querySelectorAll(".nav").forEach(b => {
            if (b.getAttribute("onclick")?.includes(`'${id}'`)) {
                b.classList.add("active");
            }
        });
    }

    const titulos = {
        inicio: "Panel principal",
        salones: "Consultar salón",
        asistencia: "Consultar asistencia",
        registro: "Registrar asistencia",
        estudiantes: "Estudiantes matriculados"
    };

    document.getElementById("titulo").textContent = titulos[id];

    actualizar();
}

// ======================================
// BUSCAR ESTUDIANTE
// ======================================

function buscarEstudiante(codigo) {
    return estudiantes.find(e =>
        e.codigo.toLowerCase() === codigo.trim().toLowerCase()
    );
}

// ======================================
// CONSULTAR SALON
// ======================================

async function consultarSalon() {
    const codigo = document.getElementById("codigoSalon").value.trim();
    const resultado = document.getElementById("resultadoSalon");

    resultado.classList.remove("oculto");

    if (!codigo) {
        resultado.textContent = "Ingrese el código del estudiante.";
        return;
    }

    try {
        const respuesta = await fetch(
            `/api/estudiantes/${encodeURIComponent(codigo)}`
        );

        const estudiante = await respuesta.json();

        if (!respuesta.ok) {
            resultado.textContent = "No se encontró al estudiante.";
            return;
        }

        resultado.innerHTML = `
            <h3>Estudiante encontrado</h3>
            <p><strong>Código:</strong> ${estudiante.codigo}</p>
            <p><strong>Nombre:</strong> ${estudiante.nombre}</p>
            <p><strong>Salón:</strong> ${estudiante.salon || "No registrado"}</p>
        `;

    } catch (error) {
        resultado.textContent = "Error de conexión con el servidor.";
    }
}

// ======================================
// CONSULTAR ASISTENCIA
// ======================================

async function consultarAsistencia() {
    const codigo = document.getElementById("codigoAsistencia").value.trim();
    const resultado = document.getElementById("resultadoAsistencia");

    resultado.classList.remove("oculto");

    if (!codigo) {
        resultado.textContent = "Ingrese el código del estudiante.";
        return;
    }

    try {
        const respuesta = await fetch(
            `/api/estudiantes/${encodeURIComponent(codigo)}`
        );

        const estudiante = await respuesta.json();

        if (!respuesta.ok) {
            resultado.textContent = "No se encontró al estudiante.";
            return;
        }

        resultado.innerHTML = `
            <h3>Historial de asistencia</h3>
            <p><strong>Nombre:</strong> ${estudiante.nombre}</p>
            <p><strong>Código:</strong> ${estudiante.codigo}</p>
            <p><strong>Salón:</strong> ${estudiante.salon || "No registrado"}</p>
            <p><strong>Asistencias:</strong> ${estudiante.asistencias}</p>
            <p><strong>Tardanzas:</strong> ${estudiante.tardanzas}</p>
            <p><strong>Faltas:</strong> ${estudiante.faltas}</p>
        `;

    } catch (error) {
        resultado.textContent = "Error de conexión con el servidor.";
    }
}

// ======================================
// AGREGAR A LA COLA
// ======================================

function agregarCola() {
    const campo = document.getElementById("codigoRegistro");
    const codigo = campo.value.trim();

    if (!codigo) {
        alert("Ingrese el código del estudiante.");
        return;
    }

    const estudiante = buscarEstudiante(codigo);

    if (!estudiante) {
        alert("El estudiante no está registrado.");
        return;
    }

    if (cola.includes(estudiante.codigo)) {
        alert("El estudiante ya está en la cola.");
        return;
    }

    if (estudianteActual?.codigo === estudiante.codigo) {
        alert("El estudiante ya está siendo atendido.");
        return;
    }

    cola.push(estudiante.codigo);
    campo.value = "";

    actualizar();
    alert("Estudiante agregado a la cola correctamente.");
}

// ======================================
// ATENDER SIGUIENTE ESTUDIANTE
// ======================================

function atender() {
    if (estudianteActual) {
        alert("Primero registra al estudiante actual.");
        return;
    }

    if (cola.length === 0) {
        alert("No hay estudiantes pendientes.");
        return;
    }

    // FIFO: primero en entrar, primero en salir
    const codigo = cola.shift();

    estudianteActual = buscarEstudiante(codigo);

    document.getElementById("nombreAtencion").textContent =
        estudianteActual.nombre + " - " + estudianteActual.codigo;

    document.getElementById("formAtencion").classList.remove("oculto");

    actualizar();
}

// ======================================
// REGISTRAR ESTADO EN EL CSV
// ======================================

async function registrarEstado(tipo) {
    if (!estudianteActual) return;

    try {
        const respuesta = await fetch("/api/asistencia", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                codigo: estudianteActual.codigo,
                tipo: tipo
            })
        });

        const resultado = await respuesta.json();

        if (!respuesta.ok) {
            alert(resultado.error || "No se pudo registrar la asistencia.");
            return;
        }

        alert("Registro actualizado correctamente.");

        estudianteActual = null;

        document.getElementById("formAtencion").classList.add("oculto");

        await cargarEstudiantes();

    } catch (error) {
        alert("No se pudo conectar con el servidor.");
        console.error(error);
    }
}

// ======================================
// MOSTRAR TABLA DE ESTUDIANTES
// ======================================

function mostrarTabla() {
    const campoFiltro = document.getElementById("filtro");
    const filtro = campoFiltro ? campoFiltro.value.toLowerCase() : "";

    const filtrados = estudiantes.filter(e =>
        e.codigo.toLowerCase().includes(filtro) ||
        e.nombre.toLowerCase().includes(filtro)
    );

    document.getElementById("tablaEstudiantes").innerHTML =
        filtrados.map(e => `
            <tr>
                <td>${e.codigo}</td>
                <td>${e.nombre}</td>
                <td>${e.salon}</td>
                <td>${e.asistencias}</td>
                <td>${e.tardanzas}</td>
                <td>${e.faltas}</td>
            </tr>
        `).join("");
}

// ======================================
// ACTUALIZAR PANEL
// ======================================

function actualizar() {
    document.getElementById("total").textContent =
        estudiantes.length;

    document.getElementById("totalAsistencias").textContent =
        estudiantes.reduce((s, e) => s + e.asistencias, 0);

    document.getElementById("totalTardanzas").textContent =
        estudiantes.reduce((s, e) => s + e.tardanzas, 0);

    document.getElementById("totalFaltas").textContent =
        estudiantes.reduce((s, e) => s + e.faltas, 0);

    document.getElementById("pendientes").textContent =
        cola.length + " pendientes";

    document.getElementById("listaCola").innerHTML =
        cola.length > 0
        ? cola.map((codigo, i) => {
            const e = buscarEstudiante(codigo);

            return `
                <div class="item-cola">
                    <span>${i + 1}. ${e.nombre}</span>
                    <strong>${e.codigo}</strong>
                </div>
            `;
        }).join("")
        : '<p class="vacio">No hay estudiantes en espera.</p>';

    document.getElementById("tablaInicio").innerHTML =
        estudiantes.slice(0, 5).map(e => `
            <tr>
                <td>${e.codigo}</td>
                <td>${e.nombre}</td>
                <td>${e.salon}</td>
                <td>${e.asistencias}</td>
                <td>${e.faltas}</td>
            </tr>
        `).join("");

    mostrarTabla();
}

// ======================================
// INICIAR SISTEMA
// ======================================

cargarEstudiantes();
