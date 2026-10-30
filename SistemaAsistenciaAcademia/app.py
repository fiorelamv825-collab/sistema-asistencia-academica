
from flask import Flask, jsonify, request, send_from_directory
import csv
import os

app = Flask(__name__)

CARPETA = os.path.dirname(os.path.abspath(__file__))
ARCHIVO = os.path.join(CARPETA, "matriculados.csv")

CAMPOS = [
    "codigo", "nombre", "asistencias",
    "tardanzas", "faltas", "salon"
]


def leer_estudiantes():
    estudiantes = []

    if not os.path.exists(ARCHIVO):
        return estudiantes

    with open(ARCHIVO, "r", newline="", encoding="utf-8-sig") as archivo:
        lector = csv.DictReader(archivo)

        for fila in lector:
            estudiantes.append({
                "codigo": fila.get("codigo", "").strip(),
                "nombre": fila.get("nombre", "Sin nombre").strip(),
                "asistencias": int(fila.get("asistencias") or 0),
                "tardanzas": int(fila.get("tardanzas") or 0),
                "faltas": int(fila.get("faltas") or 0),
                "salon": fila.get("salon", "").strip()
            })

    return estudiantes


def guardar_estudiantes(estudiantes):
    with open(ARCHIVO, "w", newline="", encoding="utf-8") as archivo:
        escritor = csv.DictWriter(archivo, fieldnames=CAMPOS)
        escritor.writeheader()
        escritor.writerows(estudiantes)


@app.route("/")
def inicio():
    return send_from_directory(CARPETA, "index.html")


@app.route("/<path:archivo>")
def archivos(archivo):
    return send_from_directory(CARPETA, archivo)


@app.route("/api/estudiantes", methods=["GET"])
def obtener_estudiantes():
    return jsonify(leer_estudiantes())


@app.route("/api/estudiantes/<codigo>", methods=["GET"])
def obtener_estudiante(codigo):
    estudiantes = leer_estudiantes()

    for estudiante in estudiantes:
        if estudiante["codigo"].lower() == codigo.lower():
            return jsonify(estudiante)

    return jsonify({"error": "Estudiante no encontrado"}), 404


@app.route("/api/asistencia", methods=["POST"])
def registrar_asistencia():
    datos = request.get_json(silent=True) or {}

    codigo = str(datos.get("codigo", "")).strip()
    tipo = datos.get("tipo")

    if tipo not in [1, 2, 3]:
        return jsonify({"error": "Tipo de asistencia no válido"}), 400

    estudiantes = leer_estudiantes()

    for estudiante in estudiantes:
        if estudiante["codigo"].lower() == codigo.lower():
            if tipo == 1:
                estudiante["asistencias"] += 1
            elif tipo == 2:
                estudiante["tardanzas"] += 1
            elif tipo == 3:
                estudiante["faltas"] += 1

            guardar_estudiantes(estudiantes)
            return jsonify({
                "mensaje": "Registro actualizado",
                "estudiante": estudiante
            })

    return jsonify({"error": "Estudiante no encontrado"}), 404


if __name__ == "__main__":
    print("Servidor de asistencia iniciado.")
    print("Abre http://127.0.0.1:5000")
    app.run(host="127.0.0.1", port=5000, debug=False)
