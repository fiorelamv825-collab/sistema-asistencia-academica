
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <cctype>

using namespace std;

struct Estudiante {
    string codigo;
    string nombre;
    string salon;
    int asistencias = 0;
    int tardanzas = 0;
    int faltas = 0;
};

queue<Estudiante> estudiantes;
queue<string> colaAsistencia;

const string ARCHIVO = "matriculados.csv";

// Quitar espacios al inicio y al final
string limpiar(string texto) {
    while (!texto.empty() && isspace((unsigned char)texto.front()))
        texto.erase(texto.begin());

    while (!texto.empty() && isspace((unsigned char)texto.back()))
        texto.pop_back();

    return texto;
}

// Convertir texto a minusculas
string minusculas(string texto) {
    for (char &c : texto)
        c = (char)tolower((unsigned char)c);

    return texto;
}

// Separar una fila CSV por comas
queue<string> separarCSV(string linea) {
    queue<string> datos;
    string dato;
    stringstream ss(linea);

    while (getline(ss, dato, ',')) {
        datos.push(limpiar(dato));
    }

    return datos;
}

// Obtener un campo de una fila
string campo(queue<string> datos, int posicion) {
    for (int i = 0; i <= posicion && !datos.empty(); i++) {
        string valor = datos.front();
        datos.pop();

        if (i == posicion)
            return valor;
    }

    return "";
}

// Comprobar que el codigo tenga formato B004-12345
bool codigoValido(string codigo) {
    codigo = limpiar(codigo);

    if (codigo.size() != 10)
        return false;

    if (codigo.substr(0, 5) != "B004-")
        return false;

    for (int i = 5; i < 10; i++) {
        if (!isdigit((unsigned char)codigo[i]))
            return false;
    }

    return true;
}

// Buscar estudiante sin perder el orden de la cola
bool buscarEstudiante(string codigo, Estudiante &resultado) {
    bool encontrado = false;
    int cantidad = estudiantes.size();

    for (int i = 0; i < cantidad; i++) {
        Estudiante e = estudiantes.front();
        estudiantes.pop();

        if (e.codigo == codigo) {
            resultado = e;
            encontrado = true;
        }

        estudiantes.push(e);
    }

    return encontrado;
}

// Agregar o actualizar estudiante
void agregarOActualizar(Estudiante nuevo) {
    Estudiante existente;

    if (buscarEstudiante(nuevo.codigo, existente)) {
        queue<Estudiante> auxiliar;

        while (!estudiantes.empty()) {
            Estudiante e = estudiantes.front();
            estudiantes.pop();

            if (e.codigo == nuevo.codigo) {
                if (!nuevo.salon.empty())
                    e.salon = nuevo.salon;

                if (e.nombre.empty() || e.nombre == "Estudiante 1" ||
                    e.nombre == "Estudiante 2" ||
                    e.nombre == "Estudiante 3") {
                    e.nombre = "Sin nombre";
                }
            }

            auxiliar.push(e);
        }

        estudiantes = auxiliar;
    } else {
        nuevo.nombre = "Sin nombre";
        estudiantes.push(nuevo);
    }
}

// Leer archivos de examenes
void cargarExamen(string nombreArchivo) {
    ifstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
        cout << "No se pudo abrir: " << nombreArchivo << "\n";
        return;
    }

    string linea;
    int columnaCodigo = -1;
    int columnaSalon = -1;
    bool encabezadoEncontrado = false;
    int cargados = 0;

    while (getline(archivo, linea)) {
        queue<string> datos = separarCSV(linea);

        if (!encabezadoEncontrado) {
            int posicion = 0;

            while (!datos.empty()) {
                string encabezado = minusculas(datos.front());
                datos.pop();

                if (encabezado == "codigo" ||
                    encabezado == "codigos") {
                    columnaCodigo = posicion;
                }

                if (encabezado == "salon" ||
                    encabezado == "inicio") {
                    columnaSalon = posicion;
                }

                posicion++;
            }

            if (columnaCodigo != -1) {
                encabezadoEncontrado = true;
            }

            continue;
        }

        if (columnaCodigo < 0)
            continue;

        string codigo = campo(datos, columnaCodigo);
        string salon = campo(datos, columnaSalon);

        codigo = limpiar(codigo);
        salon = limpiar(salon);

        if (!codigoValido(codigo))
            continue;

        Estudiante e;
        e.codigo = codigo;
        e.nombre = "Sin nombre";
        e.salon = salon;

        agregarOActualizar(e);
        cargados++;
    }

    archivo.close();

    if (!encabezadoEncontrado) {
        cout << "No se encontro el encabezado en "
             << nombreArchivo << "\n";
    } else {
        cout << nombreArchivo << ": registros revisados "
             << cargados << "\n";
    }
}

// Cargar asistencia guardada anteriormente
void cargarMatriculados() {
    ifstream archivo(ARCHIVO);

    if (!archivo.is_open()) {
        cout << "No existe matriculados.csv. Se creara uno nuevo.\n";
        return;
    }

    string linea;
    getline(archivo, linea);

    while (getline(archivo, linea)) {
        queue<string> datos = separarCSV(linea);

        Estudiante e;
        e.codigo = campo(datos, 0);
        e.nombre = campo(datos, 1);
        string valor;

        valor = campo(datos, 2);
        if (!valor.empty()) e.asistencias = stoi(valor);

        valor = campo(datos, 3);
        if (!valor.empty()) e.tardanzas = stoi(valor);

        valor = campo(datos, 4);
        if (!valor.empty()) e.faltas = stoi(valor);

        e.salon = campo(datos, 5);

        if (codigoValido(e.codigo)) {
            agregarOActualizar(e);
        }
    }

    archivo.close();
}

// Guardar estudiantes y asistencia
void guardarEstudiantes() {
    ofstream archivo(ARCHIVO);

    archivo << "codigo,nombre,asistencias,tardanzas,faltas,salon\n";

    queue<Estudiante> auxiliar;

    while (!estudiantes.empty()) {
        Estudiante e = estudiantes.front();
        estudiantes.pop();

        archivo << e.codigo << ","
                << e.nombre << ","
                << e.asistencias << ","
                << e.tardanzas << ","
                << e.faltas << ","
                << e.salon << "\n";

        auxiliar.push(e);
    }

    estudiantes = auxiliar;
    archivo.close();
}

// Actualizar asistencia
void actualizarEstudiante(string codigo, int tipo) {
    queue<Estudiante> auxiliar;

    while (!estudiantes.empty()) {
        Estudiante e = estudiantes.front();
        estudiantes.pop();

        if (e.codigo == codigo) {
            if (tipo == 1) e.asistencias++;
            if (tipo == 2) e.tardanzas++;
            if (tipo == 3) e.faltas++;
        }

        auxiliar.push(e);
    }

    estudiantes = auxiliar;
    guardarEstudiantes();
}

// Consultar salon
void consultarSalon() {
    string codigo;
    cout << "\nIngrese codigo: ";
    cin >> codigo;

    Estudiante e;

    if (buscarEstudiante(codigo, e)) {
        cout << "\nCodigo: " << e.codigo;
        cout << "\nNombre: " << e.nombre;
        cout << "\nSalon: " << e.salon << "\n";
    } else {
        cout << "\nEstudiante no encontrado.\n";
    }
}

// Consultar asistencia
void consultarAsistencia() {
    string codigo;
    cout << "\nIngrese codigo: ";
    cin >> codigo;

    Estudiante e;

    if (buscarEstudiante(codigo, e)) {
        cout << "\nNombre: " << e.nombre;
        cout << "\nAsistencias: " << e.asistencias;
        cout << "\nTardanzas: " << e.tardanzas;
        cout << "\nFaltas: " << e.faltas << "\n";
    } else {
        cout << "\nEstudiante no encontrado.\n";
    }
}

// Agregar a la cola de espera
void agregarCola() {
    string codigo;
    cout << "\nCodigo del estudiante: ";
    cin >> codigo;

    Estudiante e;

    if (buscarEstudiante(codigo, e)) {
        colaAsistencia.push(codigo);

        cout << "\nEstudiante agregado a la cola.\n";
        cout << "Pendientes: " << colaAsistencia.size() << "\n";
    } else {
        cout << "\nCodigo no encontrado.\n";
    }
}

// Atender al primer estudiante
void atenderEstudiante() {
    if (colaAsistencia.empty()) {
        cout << "\nNo hay estudiantes en espera.\n";
        return;
    }

    string codigo = colaAsistencia.front();
    colaAsistencia.pop();

    Estudiante e;

    if (!buscarEstudiante(codigo, e)) {
        cout << "\nEstudiante no encontrado.\n";
        return;
    }

    cout << "\nAtendiendo a: " << e.codigo << "\n";
    cout << "Salon: " << e.salon << "\n";
    cout << "1. Asistencia\n";
    cout << "2. Tardanza\n";
    cout << "3. Falta\n";
    cout << "Seleccione: ";

    int opcion;
    cin >> opcion;

    if (opcion >= 1 && opcion <= 3) {
        actualizarEstudiante(codigo, opcion);
        cout << "\nRegistro actualizado correctamente.\n";
    } else {
        cout << "\nOpcion incorrecta.\n";
    }
}

// Menu principal
int main() {
    cout << "Cargando matriculados...\n";

    cargarMatriculados();

    cargarExamen("examen_18052024.csv");
    cargarExamen("examen_01062024.csv");
    cargarExamen("examen_09112024.csv");
    cargarExamen("examen_07122024.csv");

    guardarEstudiantes();

    cout << "\nTotal de estudiantes unicos: "
         << estudiantes.size() << "\n";

    int opcion;

    do {
        cout << "\n================================\n";
        cout << "   ACADEMIA PREUNIVERSITARIA\n";
        cout << "   CONTROL DE ASISTENCIA\n";
        cout << "================================\n";

        cout << "1. Consultar salon\n";
        cout << "2. Consultar asistencia\n";
        cout << "3. Agregar estudiante a la cola\n";
        cout << "4. Atender siguiente estudiante\n";
        cout << "5. Ver cantidad de estudiantes\n";
        cout << "0. Salir\n";

        cout << "\nSeleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
            case 1:
                consultarSalon();
                break;

            case 2:
                consultarAsistencia();
                break;

            case 3:
                agregarCola();
                break;

            case 4:
                atenderEstudiante();
                break;

            case 5:
                cout << "\n========== CANTIDADES ==========\n";
                cout << "Total de matriculados: "
                << estudiantes.size() << "\n";
                cout << "Estudiantes en espera: "
                << colaAsistencia.size() << "\n";
                cout << "================================\n";
                break;

            case 0:
                guardarEstudiantes();
                cout << "\nSistema finalizado.\n";
                break;

            default:
                cout << "\nOpcion no valida.\n";
        }

    } while (opcion != 0);

    return 0;
}
