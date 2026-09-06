#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "libs/archivopacientes.h"
#include "libs/configuracion.h"
#include "libs/salauci.h"
using namespace std;

const int tamlinea = 256;
const int tamdato = 50;
const int cantdatosconfig = 3;
const int cantdatospaciente = 11;

   // Sprint 1: cadenas de caracteres y estructuras

void copiartexto(char dest[], const char orig[], int tam) {
    strncpy(dest, orig, tam - 1);
    dest[tam - 1] = '\0';
}

// Quita espacios y tabs al inicio y al final, moviendo los caracteres
// dentro del mismo arreglo
void recortar(char texto[]) {
    if (texto[0] == '\0') {
        return;
    }
    int inicio = 0;
    int final = strlen(texto) - 1;

    while (inicio <= final && (texto[inicio] == ' ' || texto[inicio] == '\t')) {
        inicio++;
    }

    while (final >= inicio && (texto[final] == ' ' || texto[final] == '\t')) {
        final--;
    }

    for (int i = inicio; i <= final; i++) {
        texto[i - inicio] = texto[i];
    }

    texto[final - inicio + 1] = '\0';
}
// Separa una linea en tokens usando stringstream + getline
int separartokens(const char linea[], char separador,
                   char datos[][tamdato], int maxdatos) {
    string txt(linea);
    stringstream flujo(txt);
    string dato;
    int cont = 0;

    while (cont < maxdatos && getline(flujo, dato, separador)) {
        copiartexto(datos[cont], dato.c_str(), tamdato);
        recortar(datos[cont]);
        cont++;
    }

    return cont;
}

int separarconfiguracion(const char linea[], char datos[][tamdato]) {
    return separartokens(linea, ',', datos, cantdatosconfig);
}

int separarpaciente(const char linea[], char datos[][tamdato]) {
    return separartokens(linea, ';', datos, cantdatospaciente);
}

umbralsensor tokensaumbral(char datos[][tamdato]) {
    umbralsensor u = {};
    copiartexto(u.tipo, datos[0], tamdato);
    u.vmin = atof(datos[1]);
    u.vmax = atof(datos[2]);
    return u;
}

// conf.lista se reserva con new del tamano exacto de lineas que llegan
configuracion lineasaconfiguracion(char lineas[][tamlinea], int cantlineas) {
    configuracion conf = {};
    conf.lista = new umbralsensor[cantlineas];

    for (int i = 0; i < cantlineas; i++) {
        char datos[cantdatosconfig][tamdato] = {};
        int cantd = separarconfiguracion(lineas[i], datos);

        if (cantd == cantdatosconfig) {
            conf.lista[conf.cant] = tokensaumbral(datos);
            conf.cant++;
        }
    }
    return conf;
}

paciente tokensapaciente(char datos[][tamdato], int cantd) {
    paciente p = {};
    if (cantd > 0) copiartexto(p.id, datos[0], tamdato);
    if (cantd > 1) copiartexto(p.tipodoc, datos[1], tamdato);
    if (cantd > 2) copiartexto(p.numdoc, datos[2], tamdato);
    if (cantd > 3) copiartexto(p.nombre, datos[3], tamdato);
    if (cantd > 4) copiartexto(p.apellido, datos[4], tamdato);
    if (cantd > 5) copiartexto(p.fechanac, datos[5], tamdato);
    if (cantd > 6) copiartexto(p.tel, datos[6], tamdato);
    if (cantd > 7) copiartexto(p.correo, datos[7], tamdato);
    if (cantd > 8) copiartexto(p.sangre, datos[8], tamdato);
    if (cantd > 9) copiartexto(p.entidad, datos[9], tamdato);
    if (cantd > 10) copiartexto(p.prepagada, datos[10], tamdato);
    return p;
}

// arch.lista se reserva con new del tamano exacto de lineas que llegan
archivopacientes lineasapacientes(char lineas[][tamlinea], int cantlineas) {
    archivopacientes arch = {};
    arch.lista = new paciente[cantlineas];

    for (int i = 0; i < cantlineas; i++) {
        char datos[cantdatospaciente][tamdato] = {};
        int cantd = separarpaciente(lineas[i], datos);

        if (cantd == cantdatospaciente) {
            arch.lista[arch.cant] = tokensapaciente(datos, cantd);
            arch.cant++;
        }
    }
    return arch;
}
// Las fechas vienen en formato "DD/MM/AAAA HH:MM:ss.sss". Retorna true si
// la primera es mas reciente que la segunda
bool compararfechas(const char fecha1[], const char fecha2[]) {
    string f1(fecha1);
    string f2(fecha2);

    int dia1 = stoi(f1.substr(0, 2));
    int mes1 = stoi(f1.substr(3, 2));
    int anio1 = stoi(f1.substr(6, 4));
    int hora1 = stoi(f1.substr(11, 2));
    int min1 = stoi(f1.substr(14, 2));
    int seg1 = stoi(f1.substr(17, 2));
    int mili1 = stoi(f1.substr(20, 3));

    int dia2 = stoi(f2.substr(0, 2));
    int mes2 = stoi(f2.substr(3, 2));
    int anio2 = stoi(f2.substr(6, 4));
    int hora2 = stoi(f2.substr(11, 2));
    int min2 = stoi(f2.substr(14, 2));
    int seg2 = stoi(f2.substr(17, 2));
    int mili2 = stoi(f2.substr(20, 3));

    if (anio1 != anio2) return anio1 > anio2;
    if (mes1 != mes2) return mes1 > mes2;
    if (dia1 != dia2) return dia1 > dia2;
    if (hora1 != hora2) return hora1 > hora2;
    if (min1 != min2) return min1 > min2;
    if (seg1 != seg2) return seg1 > seg2;
    return mili1 > mili2;
}
//Sprint 2: archivos de texto

// Cuenta cuantas lineas validas (ni vacias ni comentarios) tiene un
// archivo de texto, para saber cuanto reservar con new antes de leerlo
int contarlineasvalidas(const char nombrearch[]) {
    fstream arch;
    arch.open(nombrearch, ios::in);
    if (arch.fail()) {
        return 0;
    }

    int total = 0;
    char linea[tamlinea] = {};
    while (arch.getline(linea, tamlinea)) {
        if (linea[0] != '\0' && linea[0] != '#') {
            total++;
        }
    }
    arch.close();

    return total;
}

/* Leo el archivo dos veces: la primera con contarlineasvalidas, para
 saber cuanto reservar con new, y la segunda aqui para separar y guardar
 los datos. Asi el arreglo queda del tamano exacto que necesita el
 archivo, sea chico o grande */
configuracion cargarconfiguracion(const char nombrearch[]) {
    configuracion conf = {};
    int totallineas = contarlineasvalidas(nombrearch);
    conf.lista = new umbralsensor[totallineas];

    fstream arch;
    arch.open(nombrearch, ios::in);
    char linea[tamlinea] = {};
    while (conf.cant < totallineas && arch.getline(linea, tamlinea)) {
        if (linea[0] == '\0' || linea[0] == '#') {
            continue;
        }

        char datos[cantdatosconfig][tamdato] = {};
        int cantd = separarconfiguracion(linea, datos);
        if (cantd == cantdatosconfig) {
            conf.lista[conf.cant] = tokensaumbral(datos);
            conf.cant++;
        }
    }
    arch.close();

    return conf;
}

// Mismo patron que cargarconfiguracion (contarlineasvalidas + segunda pasada), pero para el archivo de pacientes
archivopacientes cargarpacientesarchivo(const char nombrearch[]) {
    archivopacientes pac = {};
    int totallineas = contarlineasvalidas(nombrearch);
    pac.lista = new paciente[totallineas];

    fstream arch;
    arch.open(nombrearch, ios::in);
    char linea[tamlinea] = {};
    while (pac.cant < totallineas && arch.getline(linea, tamlinea)) {
        if (linea[0] == '\0' || linea[0] == '#') {
            continue;
        }

        char datos[cantdatospaciente][tamdato] = {};
        int cantd = separarpaciente(linea, datos);
        if (cantd == cantdatospaciente) {
            pac.lista[pac.cant] = tokensapaciente(datos, cantd);
            pac.cant++;
        }
    }
    arch.close();

    return pac;
}

// Devuelve la posicion del umbral de ese sensor en conf.lista, o -1 si no esta
int buscarumbral(configuracion conf, const char etiqueta[]) {
    for (int i = 0; i < conf.cant; i++) {
        if (strcmp(conf.lista[i].tipo, etiqueta) == 0) {
            return i;
        }
    }
    return -1;
}

// Compara un valor contra el umbral de esa etiqueta. Si la etiqueta no
// esta en la configuracion, se considera que no hay anomalia
bool fueradelrango(configuracion conf, const char etiqueta[], double valor) {
    int i = buscarumbral(conf, etiqueta);
    if (i == -1) {
        return false;
    }
    return valor < conf.lista[i].vmin || valor > conf.lista[i].vmax;
}

// No evalua ECG, ese tiene su propia regla mas adelante (esecganomalo)
bool esanomalo(lectura lect, configuracion conf) {
    if (lect.tipo == 'T') {
        return fueradelrango(conf, "T", lect.dato1);
    }

    if (lect.tipo == 'O') {
        return fueradelrango(conf, "O", lect.dato1);
    }

    if (lect.tipo == 'P') {
        bool sismal = fueradelrango(conf, "P_SIS", lect.dato1);
        bool diamal = fueradelrango(conf, "P_DIA", lect.dato2);

        if (sismal || diamal) {
            return true;
        }
        return false;
    }

    return false;
}

// Cuenta cuantas mediciones tiene el paciente en toda la sala (para saber cuanto reservar)
int contarmedicionespaciente(salauci& sala, const char idpac[]) {
    int cont = 0;
    for (int i = 0; i < sala.cantmaq; i++) {
        for (int j = 0; j < sala.lista[i].cantmed; j++) {
            if (strcmp(sala.lista[i].lista[j].idpac, idpac) == 0) {
                cont++;
            }
        }
    }
    return cont;
}

/* Junta todas las mediciones de un paciente recorriendo todas las maquinas
 de la sala. El arreglo que devuelve se reserva adentro con new, del
 tamano exacto que hace falta; quien la llama debe hacer delete[] cuando
 termine de usarlo. cant queda con la cantidad de mediciones encontradas */
medicion* recogermedicionespaciente(salauci& sala, const char idpac[], int& cant) {
    cant = contarmedicionespaciente(sala, idpac);
    medicion* med = new medicion[cant];

    int cont = 0;
    for (int i = 0; i < sala.cantmaq; i++) {
        for (int j = 0; j < sala.lista[i].cantmed; j++) {
            medicion m = sala.lista[i].lista[j];
            if (strcmp(m.idpac, idpac) == 0) {
                med[cont] = m;
                cont++;
            }
        }
    }

    return med;
}

// Ordena las mediciones de la mas antigua a la mas reciente (bubble sort)
void ordenarmedicionesporfecha(medicion med[], int cant) {
    for (int i = 0; i < cant - 1; i++) {
        for (int j = 0; j < cant - i - 1; j++) {
            if (compararfechas(med[j].fecha, med[j + 1].fecha)) {
                medicion tmp = med[j];
                med[j] = med[j + 1];
                med[j + 1] = tmp;
            }
        }
    }
}

// Junta recogermedicionespaciente + ordenarmedicionesporfecha, porque
// siempre se usan juntas: primero se juntan las mediciones del paciente
// y de una vez se ordenan por fecha
medicion* obtenermedicionesordenadas(salauci& sala, const char idpac[], int& cant) {
    medicion* med = recogermedicionespaciente(sala, idpac, cant);
    ordenarmedicionesporfecha(med, cant);
    return med;
}

// Actualiza minimo y maximo con un valor nuevo, usando & para modificarlos directamente
void actualizarminmax(double valor, bool esprimera, double& minimo, double& maximo) {
    if (esprimera) {
        minimo = valor;
        maximo = valor;
    } else {
        if (valor < minimo) {
            minimo = valor;
        }
        if (valor > maximo) {
            maximo = valor;
        }
    }
}

// Escribe mediciones_paciente_<id>.txt: agrupado por sensor, en orden
// cronologico, marcando anomalias (menos en ECG) y con las estadisticas
// de cada sensor al final de su grupo
bool generarreportepaciente(const char idpac[], salauci& sala, configuracion conf) {
    int cant = 0;
    medicion* med = obtenermedicionesordenadas(sala, idpac, cant);

    char nombrearch[tamlinea] = "mediciones_paciente_";
    strcat(nombrearch, idpac);
    strcat(nombrearch, ".txt");

    fstream rep;
    rep.open(nombrearch, ios::out);
    if (rep.fail()) {
        delete[] med;
        return false;
    }

    rep << "Reporte de mediciones del paciente " << idpac << "\n";

    char tipos[4] = {'T', 'P', 'E', 'O'};
    for (int t = 0; t < 4; t++) {
        char tipo = tipos[t];
        double vmin = 0, vmax = 0, suma = 0;
        double vmind = 0, vmaxd = 0, sumad = 0;
        int cantval = 0;
        bool encontro = false;

        for (int i = 0; i < cant; i++) {
            for (int j = 0; j < med[i].cantlect; j++) {
                lectura lect = med[i].datos[j];
                if (lect.tipo != tipo) {
                    continue;
                }

                if (!encontro) {
                    rep << "\n--- Sensor " << tipo << " ---\n";
                    encontro = true;
                }

                rep << med[i].fecha << " | ";
                if (tipo == 'P') {
                    rep << "sistolica=" << lect.dato1 << " diastolica=" << lect.dato2;
                } else {
                    rep << "valor=" << lect.dato1;
                }

                if (tipo != 'E') {
                    if (esanomalo(lect, conf)) {
                        rep << " | ANOMALA";
                    } else {
                        rep << " | NORMAL";
                    }
                }
                rep << "\n";

                actualizarminmax(lect.dato1, cantval == 0, vmin, vmax);
                suma += lect.dato1;

                if (tipo == 'P') {
                    actualizarminmax(lect.dato2, cantval == 0, vmind, vmaxd);
                    sumad += lect.dato2;
                }

                cantval++;
            }
        }

        if (encontro) {
            if (tipo == 'P') {
                rep << "Estadisticas sistolica: minimo=" << vmin
                    << ", maximo=" << vmax
                    << ", promedio=" << suma / cantval << "\n";
                rep << "Estadisticas diastolica: minimo=" << vmind
                    << ", maximo=" << vmaxd
                    << ", promedio=" << sumad / cantval << "\n";
            } else {
                rep << "Estadisticas: minimo=" << vmin
                    << ", maximo=" << vmax
                    << ", promedio=" << suma / cantval << "\n";
            }
        }
    }

    rep.close();
    delete[] med;
    return true;
}

//Sprint 3: archivos binarios

uint8_t leerbyte(fstream& arch) {
    uint8_t v = 0;
    arch.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

// Lee un numero de 4 bytes (usado para las cantidades: de maquinas, mediciones, lecturas)
int leerentero4bytes(fstream& arch) {
    int v = 0;
    arch.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

// Lee un valor de 8 bytes (usado para T, O y ECG)
double leerdouble(fstream& arch) {
    double v = 0;
    arch.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

/* Libera toda la memoria reservada dentro de una sala (maquinas,
 mediciones y lecturas), de adentro hacia afuera. Hace falta antes de
 volver a llamar leerbsf, para no perder memoria si ya habia datos */
void liberarsala(salauci& sala) {
    for (int i = 0; i < sala.cantmaq; i++) {
        for (int j = 0; j < sala.lista[i].cantmed; j++) {
            delete[] sala.lista[i].lista[j].datos;
        }
        delete[] sala.lista[i].lista;
    }
    delete[] sala.lista;
    sala.lista = nullptr;
    sala.cantmaq = 0;
}

/* Leo el .bsf: id de sala (1 byte) + cantidad de maquinas (1 byte), y por
 cada maquina su id (1 byte) + cantidad de mediciones (4 bytes) + las
 mediciones. Cada medicion trae id paciente (1 byte) + fecha (24 bytes) +
 cantidad de lecturas (4 bytes) + las lecturas (tipo 1 byte + el valor,
 8 bytes o dos enteros de 4 si es presion).
 Cada cantidad se lee antes que su arreglo, asi que se reserva con new
 justo despues de leerla (maquinas, luego mediciones, luego lecturas) */
bool leerbsf(const char nombrearch[], salauci& sala) {
    liberarsala(sala);
    sala.id = 0;

    fstream arch;
    arch.open(nombrearch, ios::in | ios::binary);
    if (arch.fail()) {
        return false;
    }

    sala.id = leerbyte(arch);
    uint8_t cantmaq = leerbyte(arch);
    sala.lista = new maquinauci[cantmaq];

    for (int i = 0; i < cantmaq; i++) {
        maquinauci maq = {};
        maq.idmaq = static_cast<char>(leerbyte(arch));

        int cantmed = leerentero4bytes(arch);
        maq.lista = new medicion[cantmed];

        for (int j = 0; j < cantmed; j++) {
            medicion med = {};

            uint8_t idb = leerbyte(arch);
            char idtxt[tamdato] = {};
            sprintf(idtxt, "%d", idb);
            copiartexto(med.idpac, idtxt, tamdato);

            char fecha[25] = {};
            arch.read(fecha, 24);
            fecha[24] = '\0';
            copiartexto(med.fecha, fecha, tamdato);

            int cantlect = leerentero4bytes(arch);
            med.datos = new lectura[cantlect];

            for (int k = 0; k < cantlect; k++) {
                char tipo = '\0';
                arch.read(&tipo, 1);

                lectura lect = {};
                lect.tipo = tipo;

                if (tipo == 'P') {
                    int sis = leerentero4bytes(arch);
                    int dia = leerentero4bytes(arch);
                    lect.dato1 = sis;
                    lect.dato2 = dia;
                } else {
                    lect.dato1 = leerdouble(arch);
                }

                med.datos[med.cantlect] = lect;
                med.cantlect++;
            }

            maq.lista[maq.cantmed] = med;
            maq.cantmed++;
        }

        sala.lista[sala.cantmaq] = maq;
        sala.cantmaq++;
    }

    arch.close();
    return true;
}

// Escribe anomalias.txt con las anomalias de todos los pacientes (menos ECG), agrupadas por paciente
bool generarreporteanomalias(archivopacientes pac, salauci& sala, configuracion conf) {
    fstream rep;
    rep.open("anomalias.txt", ios::out);
    if (rep.fail()) {
        return false;
    }

    rep << "Reporte de anomalias\n";

    for (int p = 0; p < pac.cant; p++) {
        int cant = 0;
        medicion* med = obtenermedicionesordenadas(sala, pac.lista[p].id, cant);

        bool encontro = false;
        for (int i = 0; i < cant; i++) {
            for (int j = 0; j < med[i].cantlect; j++) {
                lectura lect = med[i].datos[j];
                if (lect.tipo == 'E' || !esanomalo(lect, conf)) {
                    continue;
                }

                if (!encontro) {
                    rep << "\n--- Paciente " << pac.lista[p].id << " ---\n";
                    encontro = true;
                }

                rep << med[i].fecha << " | Sensor " << lect.tipo << " | ";
                if (lect.tipo == 'P') {
                    rep << "sistolica=" << lect.dato1 << " diastolica=" << lect.dato2;
                } else {
                    rep << "valor=" << lect.dato1;
                }
                rep << "\n";
            }
        }

        delete[] med;
    }

    rep.close();
    return true;
}

double valorabsoluto(double x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

// Es anomalo si |minimo| + |maximo| de las lecturas ECG supera |limite minimo| + |limite maximo| de E
bool esecganomalo(medicion med[], int cant, configuracion conf) {
    double vmin = 0;
    double vmax = 0;
    bool encontro = false;

    for (int i = 0; i < cant; i++) {
        for (int j = 0; j < med[i].cantlect; j++) {
            lectura lect = med[i].datos[j];
            if (lect.tipo != 'E') {
                continue;
            }

            actualizarminmax(lect.dato1, !encontro, vmin, vmax);
            encontro = true;
        }
    }

    if (!encontro) {
        return false;
    }

    int i = buscarumbral(conf, "E");
    if (i == -1) {
        return false;
    }

    double sumapac = valorabsoluto(vmin) + valorabsoluto(vmax);
    double sumalim = valorabsoluto(conf.lista[i].vmin) + valorabsoluto(conf.lista[i].vmax);

    return sumapac > sumalim;
}

// Exportacion de pacientes con ECG anomalo, en el formato binario que se pide: ID del paciente (11 bytes), cantidad de mediciones ECG
// (4 bytes) y por cada una fecha/hora (24 bytes) y valor (8 bytes).
bool exportarpacientesecganomalos(archivopacientes pac, salauci& sala, configuracion conf) {
    fstream arch;
    arch.open("pacientes_ecg_anomalos.dat", ios::out | ios::binary);
    if (arch.fail()) {
        return false;
    }

    for (int p = 0; p < pac.cant; p++) {
        int cant = 0;
        medicion* med = obtenermedicionesordenadas(sala, pac.lista[p].id, cant);

        if (!esecganomalo(med, cant, conf)) {
            delete[] med;
            continue;
        }

        int cantecg = 0;
        for (int i = 0; i < cant; i++) {
            for (int j = 0; j < med[i].cantlect; j++) {
                if (med[i].datos[j].tipo == 'E') cantecg++;
            }
        }

        char idtxt[11] = {};
        copiartexto(idtxt, pac.lista[p].id, 11);
        arch.write(idtxt, 11);

        arch.write(reinterpret_cast<const char*>(&cantecg), sizeof(cantecg));

        for (int i = 0; i < cant; i++) {
            for (int j = 0; j < med[i].cantlect; j++) {
                if (med[i].datos[j].tipo != 'E') {
                    continue;
                }

                char fecha[24] = {};
                copiartexto(fecha, med[i].fecha, 24);
                arch.write(fecha, 24);

                double v = med[i].datos[j].dato1;
                arch.write(reinterpret_cast<const char*>(&v), sizeof(v));
            }
        }

        delete[] med;
    }

    arch.close();
    return true;
}

//Pruebas

int contarocurrencias(const string& texto, const string& sub) {
    int cont = 0;
    size_t pos = texto.find(sub);
    while (pos != string::npos) {
        cont++;
        pos = texto.find(sub, pos + sub.size());
    }
    return cont;
}

// Arma una medicion con una sola lectura, para no repetir estas 5 lineas por cada dato de prueba
medicion armarmedicion(const char idpac[], const char fecha[], lectura lect) {
    medicion med = {};
    copiartexto(med.idpac, idpac, tamdato);
    copiartexto(med.fecha, fecha, tamdato);
    med.cantlect = 1;
    med.datos = new lectura[1];
    med.datos[0] = lect;
    return med;
}

// Igual que armarmedicion, pero con dos lecturas (para los casos de ECG)
medicion armarmedicion2(const char idpac[], const char fecha[], lectura lect1, lectura lect2) {
    medicion med = {};
    copiartexto(med.idpac, idpac, tamdato);
    copiartexto(med.fecha, fecha, tamdato);
    med.cantlect = 2;
    med.datos = new lectura[2];
    med.datos[0] = lect1;
    med.datos[1] = lect2;
    return med;
}

bool pruebas() {
    char datosconfig[cantdatosconfig][tamdato] = {};
    int cantd = separarconfiguracion("T,36.0,38.0", datosconfig);
    if (cantd != 3 || strcmp(datosconfig[0], "T") != 0) {
        cout << "Algo fallo separando la configuracion\n";
        return false;
    }

    char datospac[cantdatospaciente][tamdato] = {};
    cantd = separarpaciente(
        "1; CC; 123456789; Juan; Perez; 01/01/1980; 300; juan@mail.com; A+; EPS; Sura",
        datospac);
    if (cantd != 11 || strcmp(datospac[3], "Juan") != 0) {
        cout << "Algo fallo separando el paciente\n";
        return false;
    }

    char lineasconf[5][tamlinea] = {
        "T,36.0,38.0", "P_SIS,90,140", "P_DIA,60,90",
        "E,-3.858747,1.228621", "O,95,100"
    };
    configuracion conf = lineasaconfiguracion(lineasconf, 5);
    if (conf.cant != 5) {
        cout << "No se armo bien la configuracion\n";
        return false;
    }

    char lineaspac[2][tamlinea] = {
        "1; TI; 123; Juan; Perez; 01/01/1980; 300; juan@mail.com; A+; EPS; Sura",
        "2; CC; 456; Maria; Gomez; 02/02/1990; 301; maria@mail.com; O+; EPS; "
    };
    archivopacientes pac = lineasapacientes(lineaspac, 2);
    if (pac.cant != 2) {
        cout << "No se armaron bien los pacientes\n";
        return false;
    }

    if (!compararfechas("02/01/2024 00:00:00.000", "01/12/2023 23:59:59.999")) {
        cout << "Fallo comparar fechas (anio/mes)\n";
        return false;
    }
    if (!compararfechas("01/01/2024 10:00:00.500", "01/01/2024 10:00:00.100")) {
        cout << "Fallo comparar fechas (milisegundos)\n";
        return false;
    }

    lectura tempok = {'T', 37, 0};
    lectura tempmal = {'T', 39, 0};
    lectura presok = {'P', 120, 80};
    lectura oximal = {'O', 92, 0};

    if (esanomalo(tempok, conf)) {
        cout << "Marco anomala una temperatura normal\n";
        return false;
    }
    if (!esanomalo(tempmal, conf)) {
        cout << "No detecto una temperatura anomala\n";
        return false;
    }
    if (esanomalo(presok, conf)) {
        cout << "Marco anomala una presion normal\n";
        return false;
    }
    if (!esanomalo(oximal, conf)) {
        cout << "No detecto una oximetria anomala\n";
        return false;
    }

    configuracion confarch = cargarconfiguracion("data/configuracion.txt");
    if (confarch.cant == 0) {
        cout << "No cargo el archivo de configuracion\n";
        return false;
    }

    archivopacientes pacarch = cargarpacientesarchivo("data/pacientes_small.csv");
    if (pacarch.cant == 0) {
        cout << "No cargo el archivo de pacientes\n";
        return false;
    }

    // sala ya no pesa 1 MB (solo tiene un puntero adentro), asi que puede
    // ser una variable local normal, sin necesidad de static ni global
    salauci sala = {};
    sala.cantmaq = 1;
    sala.lista = new maquinauci[1];
    sala.lista[0].cantmed = 9;
    sala.lista[0].lista = new medicion[9];

    sala.lista[0].lista[0] = armarmedicion("1", "02/01/2024 10:00:00.000", tempmal);

    lectura ecgok = {'E', 0.5, 0};
    sala.lista[0].lista[1] = armarmedicion("1", "01/01/2024 10:00:00.000", ecgok);

    /* Segunda temperatura, con fecha anterior a la primera, para comprobar
     que dentro del grupo del sensor T las lecturas quedan en orden
     cronologico (no solo por sensor) */
    lectura tempbaja = {'T', 36.5, 0};
    sala.lista[0].lista[2] = armarmedicion("1", "01/01/2024 09:00:00.000", tempbaja);

    // Presion arterial: sistolica anomala (150 > 140), diastolica normal (70)
    lectura presmal = {'P', 150, 70};
    sala.lista[0].lista[3] = armarmedicion("1", "03/01/2024 08:00:00.000", presmal);

    // Oximetria anomala (92 < 95)
    sala.lista[0].lista[4] = armarmedicion("1", "03/01/2024 09:00:00.000", oximal);

    // Medicion de OTRO paciente, para comprobar que no se mezcla con el reporte del paciente "1"
    lectura oxiotro = {'O', 77.7, 0};
    sala.lista[0].lista[5] = armarmedicion("2", "03/01/2024 09:30:00.000", oxiotro);

    // Segunda lectura de ECG, para que sus estadisticas se calculen sobre mas de un dato (ECG no necesita deteccion de anomalia)
    lectura ecgok2 = {'E', -0.3, 0};
    sala.lista[0].lista[6] = armarmedicion("1", "04/01/2024 10:00:00.000", ecgok2);

    // Presion arterial normal (sistolica y diastolica dentro de rango)
    lectura presok2 = {'P', 120, 80};
    sala.lista[0].lista[7] = armarmedicion("1", "04/01/2024 08:00:00.000", presok2);

    // Oximetria normal
    lectura oxiok = {'O', 98, 0};
    sala.lista[0].lista[8] = armarmedicion("1", "04/01/2024 09:00:00.000", oxiok);

    if (!generarreportepaciente("1", sala, confarch)) {
        cout << "No se genero el reporte del paciente\n";
        return false;
    }

    fstream rep;
    rep.open("mediciones_paciente_1.txt", ios::in);
    if (rep.fail()) {
        cout << "No se pudo abrir el reporte generado\n";
        return false;
    }

    string contenido;
    char linea[tamlinea] = {};
    bool huboanomalia = false;
    bool huboestadistica = false;
    while (rep.getline(linea, tamlinea)) {
        if (strstr(linea, "ANOMALA")) huboanomalia = true;
        if (strstr(linea, "Estadisticas")) huboestadistica = true;
        contenido += linea;
        contenido += "\n";
    }
    rep.close();

    if (contenido.find("77.7") != string::npos) {
        cout << "El reporte del paciente 1 tiene datos de otro paciente\n";
        return false;
    }
    if (contenido.find("Sensor P") == string::npos ||
        contenido.find("Sensor O") == string::npos) {
        cout << "Al reporte le falto presion u oximetria\n";
        return false;
    }

    // Casos anomalos: T=39, P=150/70, O=92 -> 3, casos normales: T=36.5, P=120/80, O=98 -> 3, ECG no debe sumar a ninguno de los dos conteos.

    if (contarocurrencias(contenido, "ANOMALA") != 3) {
        cout << "No se detectaron las 3 lecturas anomalas esperadas (T, P, O)\n";
        return false;
    }
    if (contarocurrencias(contenido, "NORMAL") != 3) {
        cout << "No se detectaron las 3 lecturas normales esperadas (T, P, O)\n";
        return false;
    }

    if (!huboanomalia || !huboestadistica) {
        cout << "El reporte no tiene el contenido esperado\n";
        return false;
    }

    // La lectura del 01/01 (36.5) debe aparecer antes que la del 02/01 (39) dentro del grupo del sensor T, es decir, ordenadas por fecha.

    size_t posbaja = contenido.find("valor=36.5");
    size_t posalta = contenido.find("valor=39");
    if (posbaja == string::npos || posalta == string::npos ||
        posbaja > posalta) {
        cout << "Las temperaturas no quedaron en orden por fecha\n";
        return false;
    }

    // Casos armados a mano, no dependen de ningun archivo .bsf

    lectura ecgbajook = {'E', -1.0, 0};
    lectura ecgaltook = {'E', 0.8, 0};
    medicion medecgok = armarmedicion2("9", "01/01/2024 00:00:00.000", ecgbajook, ecgaltook);
    medicion medsecgok[1] = {medecgok};
    if (esecganomalo(medsecgok, 1, confarch)) {
        cout << "Marco anomalo un ECG normal\n";
        return false;
    }

    lectura ecgbajomal = {'E', -4.0, 0};
    lectura ecgaltomal = {'E', 3.0, 0};
    medicion medecgmal = armarmedicion2("9", "01/01/2024 00:00:00.000", ecgbajomal, ecgaltomal);
    medicion medsecgmal[1] = {medecgmal};
    if (!esecganomalo(medsecgmal, 1, confarch)) {
        cout << "No detecto un ECG anomalo\n";
        return false;
    }

    // De aqui en adelante, contra el archivo .bsf real
    salauci salabsf = {};
    if (!leerbsf("patient_readings_simulation_small.bsf", salabsf)) {
        cout << "Fallo leerbsf (recuerde generar el .bsf, vea el README)\n";
        return false;
    }
    if (salabsf.cantmaq != 2 ||
        salabsf.lista[0].cantmed != 163 ||
        salabsf.lista[1].cantmed != 163) {
        cout << "leerbsf no cargo la cantidad esperada de maquinas/mediciones\n";
        return false;
    }

    /* Verificacion de contenido, no solo de cantidades: la primera medicion
     de la primera maquina debe coincidir exactamente con la primera
     medicion de patient_readings_simulation_small.txt (paciente 1,
     25/06/2025 01:00:00.000, T=39.8, P=107-88, O=98.0). Si leerbsf
     estuviera desplazado en algun byte, estos valores saldrian mal aunque
     las cantidades coincidan */

    medicion primermed = salabsf.lista[0].lista[0];
    if (strcmp(primermed.idpac, "1") != 0 ||
        strcmp(primermed.fecha, "25/06/2025 01:00:00.000") != 0 ||
        primermed.cantlect != 3) {
        cout << "leerbsf no leyo bien la primera medicion (paciente/fecha/cantidad)\n";
        return false;
    }
    if (primermed.datos[0].tipo != 'T' || primermed.datos[0].dato1 != 39.8 ||
        primermed.datos[1].tipo != 'P' || primermed.datos[1].dato1 != 107 ||
        primermed.datos[1].dato2 != 88 ||
        primermed.datos[2].tipo != 'O' || primermed.datos[2].dato1 != 98.0) {
        cout << "leerbsf no leyo bien las lecturas de la primera medicion\n";
        return false;
    }

    // generarreporteanomalias: agrupado por paciente, sin ECG

    if (!generarreporteanomalias(pacarch, salabsf, confarch)) {
        cout << "Fallo generarreporteanomalias\n";
        return false;
    }

    fstream archanom("anomalias.txt", ios::in);
    if (archanom.fail()) {
        cout << "No se genero anomalias.txt\n";
        return false;
    }
    string contanom;
    char lineaanom[tamlinea] = {};
    while (archanom.getline(lineaanom, tamlinea)) {
        contanom += lineaanom;
        contanom += "\n";
    }
    archanom.close();

    if (contanom.find("Sensor E") != string::npos) {
        cout << "anomalias.txt no debe traer lecturas de ECG\n";
        return false;
    }
    if (contanom.find("--- Paciente") == string::npos) {
        cout << "anomalias.txt no agrupo por paciente\n";
        return false;
    }

    // con este .bsf los dos pacientes de prueba tienen ECG anomalo, 140 mediciones ECG cada uno
    if (!exportarpacientesecganomalos(pacarch, salabsf, confarch)) {
        cout << "Fallo exportarpacientesecganomalos\n";
        return false;
    }

    fstream archecg("pacientes_ecg_anomalos.dat", ios::in | ios::binary);
    if (archecg.fail()) {
        cout << "No se genero pacientes_ecg_anomalos.dat\n";
        return false;
    }

    const char* idsok[2] = {"1", "2"};
    for (int p = 0; p < 2; p++) {
        char idleido[12] = {};
        archecg.read(idleido, 11);
        idleido[11] = '\0';

        int cantecgleida = 0;
        archecg.read(reinterpret_cast<char*>(&cantecgleida), sizeof(cantecgleida));

        if (strcmp(idleido, idsok[p]) != 0 || cantecgleida != 140) {
            cout << "pacientes_ecg_anomalos.dat no tiene el contenido esperado\n";
            archecg.close();
            return false;
        }

        for (int m = 0; m < cantecgleida; m++) {
            char fecha[24] = {};
            archecg.read(fecha, 24);
            double v = 0;
            archecg.read(reinterpret_cast<char*>(&v), sizeof(v));
        }
    }
    archecg.close();

    // Se acabaron las pruebas: libero todo lo que se reservo con new
    delete[] medecgok.datos;
    delete[] medecgmal.datos;
    delete[] conf.lista;
    delete[] pac.lista;
    delete[] confarch.lista;
    delete[] pacarch.lista;
    liberarsala(sala);
    liberarsala(salabsf);

    cout << "Todas las pruebas pasaron bien.\n";
    return true;
}

//Menu

void mostrarmenu() {
    cout << "\n===== Sistema de Monitoreo Biomedico =====\n";
    cout << "1. Cargar archivo de configuracion y datos de pacientes\n";
    cout << "2. Leer archivo .bsf\n";
    cout << "3. Generar reporte de anomalias\n";
    cout << "4. Calcular estadisticas\n";
    cout << "5. Exportar datos procesados\n";
    cout << "6. Ejecutar pruebas\n";
    cout << "7. Salir\n";
    cout << "Que quieres hacer? ";
}

int main() {
    configuracion conf = {};
    archivopacientes pac = {};
    salauci sala = {};
    bool cargobsf = false;
    int op = 0;

    while (op != 7) {
        mostrarmenu();
        cin >> op;

        switch (op) {
            case 1:
                delete[] conf.lista;
                delete[] pac.lista;
                conf = cargarconfiguracion("data/configuracion.txt");
                pac = cargarpacientesarchivo("data/pacientes.csv");
                if (conf.cant > 0 && pac.cant > 0) {
                    cout << "Se cargaron " << conf.cant << " umbrales y "
                         << pac.cant << " pacientes.\n";
                } else {
                    cout << "No se pudo cargar nada, revisa los archivos.\n";
                }
                break;

            case 2: {
                cout << "Nombre del archivo .bsf (por ejemplo patient_readings_simulation_small.bsf): ";
                char nombrebsf[tamdato] = {};
                cin >> nombrebsf;
                leerbsf(nombrebsf, sala);
                cargobsf = sala.cantmaq > 0;
                if (cargobsf) {
                    cout << "Se cargaron " << sala.cantmaq << " maquinas del archivo .bsf.\n";
                } else {
                    cout << "No se pudo leer el archivo .bsf (revise el nombre, o generelo primero, vea el README).\n";
                }
                break;
            }

            case 3:
                if (conf.cant == 0 || pac.cant == 0 || !cargobsf) {
                    cout << "Primero usa las opciones 1 y 2 para cargar los datos.\n";
                } else if (generarreporteanomalias(pac, sala, conf)) {
                    cout << "Se genero el archivo anomalias.txt.\n";
                } else {
                    cout << "No se pudo generar anomalias.txt.\n";
                }
                break;

            case 4:
                if (conf.cant == 0 || !cargobsf) {
                    cout << "Primero usa las opciones 1 y 2 para cargar los datos.\n";
                } else {
                    cout << "Cual es el id del paciente? ";
                    char idpac[tamdato] = {};
                    cin >> idpac;
                    if (generarreportepaciente(idpac, sala, conf)) {
                        cout << "Se genero mediciones_paciente_" << idpac << ".txt.\n";
                    } else {
                        cout << "No se pudo generar el reporte de ese paciente.\n";
                    }
                }
                break;

            case 5:
                if (conf.cant == 0 || pac.cant == 0 || !cargobsf) {
                    cout << "Primero usa las opciones 1 y 2 para cargar los datos.\n";
                } else if (exportarpacientesecganomalos(pac, sala, conf)) {
                    cout << "Se genero pacientes_ecg_anomalos.dat.\n";
                } else {
                    cout << "No se pudo generar pacientes_ecg_anomalos.dat.\n";
                }
                break;

            case 6:
                pruebas();
                break;

            case 7:
                break;

            default:
                cout << "Esa opcion no existe.\n";
                break;
        }
    }

    delete[] conf.lista;
    delete[] pac.lista;
    liberarsala(sala);

    return 0;
}
