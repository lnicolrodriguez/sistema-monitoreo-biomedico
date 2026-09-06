# Informe Final - Sistema de Monitoreo Biomédico

Este informe explica las estructuras y las funciones del proyecto, tal como quedaron después del Sprint 4 (arreglos dinámicos).

## 1. Estructuras utilizadas

Cada estructura está definida en su propio archivo dentro de `libs/`.

### `lectura` (`libs/lectura.h`)

Representa una sola lectura de un sensor.

```cpp
struct lectura {
    char tipo;
    double dato1;
    double dato2;
};
```

- `tipo`: el tipo de sensor ('T' temperatura, 'P' presión, 'E' ECG, 'O' oxímetro).
- `dato1`: el valor de la lectura (o la presión sistólica, si `tipo` es 'P').
- `dato2`: solo se usa cuando `tipo` es 'P', para la presión diastólica.

### `medicion` (`libs/medicion.h`)

Representa una medición de un paciente, que puede traer varias lecturas (por ejemplo, temperatura y oxímetro tomados al mismo tiempo).

```cpp
struct medicion {
    char idpac[50];
    char fecha[50];
    int cantlect;
    lectura* datos;
};
```

- `idpac`: el id del paciente al que pertenece la medición.
- `fecha`: la fecha y hora de la medición, en formato "DD/MM/AAAA HH:MM:ss.sss".
- `cantlect`: cuántas lecturas hay en `datos`.
- `datos`: arreglo dinámico (reservado con `new`) de `cantlect` lecturas.

### `maquinauci` (`libs/maquina.h`)

Representa una máquina de la UCI, que toma varias mediciones a lo largo del tiempo.

```cpp
struct maquinauci {
    char idmaq;
    int cantmed;
    medicion* lista;
};
```

- `idmaq`: el id de la máquina.
- `cantmed`: cuántas mediciones hay en `lista`.
- `lista`: arreglo dinámico de `cantmed` mediciones.

### `salauci` (`libs/salauci.h`)

Representa una sala de la UCI, que puede tener varias máquinas.

```cpp
struct salauci {
    int id;
    int cantmaq;
    maquinauci* lista;
};
```

- `id`: el id de la sala.
- `cantmaq`: cuántas máquinas hay en `lista`.
- `lista`: arreglo dinámico de `cantmaq` máquinas.

`salauci` → `maquinauci` → `medicion` → `lectura` es la misma estructura que trae el archivo binario `.bsf`, anidada nivel por nivel.

### `umbralsensor` y `configuracion` (`libs/configuracion.h`)

`umbralsensor` representa un límite de alarma para un tipo de sensor (una línea del archivo `configuracion.txt`).

```cpp
struct umbralsensor {
    char tipo[50];
    double vmin;
    double vmax;
};
```

`configuracion` es el arreglo de todos los umbrales cargados del archivo.

```cpp
struct configuracion {
    umbralsensor* lista;
    int cant;
};
```

- `lista`: arreglo dinámico de `cant` umbrales (uno por cada línea de `configuracion.txt`: T, P_SIS, P_DIA, E, O).
- `cant`: cuántos umbrales hay cargados.

### `paciente` (`libs/paciente.h`)

Representa los datos de un paciente (una línea del archivo `pacientes.csv`).

```cpp
struct paciente {
    char id[50];
    char tipodoc[50];
    char numdoc[50];
    char nombre[50];
    char apellido[50];
    char fechanac[50];
    char tel[50];
    char correo[50];
    char sangre[50];
    char entidad[50];
    char prepagada[50];
};
```

Todos los campos son arreglos de caracteres de tamaño fijo (no se usa `string` para guardar los datos, solo para separarlos en tokens), porque cada campo es un solo dato de texto y no una lista de varias cosas.

### `archivopacientes` (`libs/archivopacientes.h`)

Representa el archivo completo de pacientes.

```cpp
struct archivopacientes {
    int cant;
    paciente* lista;
};
```

- `lista`: arreglo dinámico de `cant` pacientes.

## 2. Descripción de las funciones implementadas

Todas las funciones están en `main.cpp`.

### Separar cadenas y convertir a estructuras (Sprint 1)

- **`recortar(char texto[])`**: quita espacios y tabs al inicio y al final de un texto, moviendo los caracteres dentro del mismo arreglo.
- **`copiartexto(char dest[], const char orig[], int tam)`**: copia texto de forma segura a un arreglo de tamaño fijo, usando `strncpy`.
- **`separartokens(...)`**: separa una línea en tokens usando `stringstream` y `getline`, con el delimitador que se le pida.
- **`separarconfiguracion(...)`** y **`separarpaciente(...)`**: llaman a `separartokens` con `,` y `;` respectivamente.
- **`tokensaumbral(...)`**: convierte los tokens de una línea de configuración en un `umbralsensor`.
- **`tokensapaciente(...)`**: convierte los tokens de una línea de pacientes en un `paciente`.
- **`lineasaconfiguracion(...)`** y **`lineasapacientes(...)`**: reciben un arreglo de líneas ya en memoria y arman la `configuracion`/`archivopacientes` completa (reservando el arreglo con `new` del tamaño exacto). Se usan sobre todo en las pruebas.
- **`compararfechas(...)`**: compara dos fechas con formato "DD/MM/AAAA HH:MM:ss.sss" usando `substr` y `stoi`, y dice si la primera es más reciente.

### Lectura de archivos de texto (Sprint 2)

- **`contarlineasvalidas(...)`**: cuenta cuántas líneas útiles (ni vacías ni comentarios) tiene un archivo, para saber cuánto reservar con `new` antes de leerlo.
- **`cargarconfiguracion(...)`** y **`cargarpacientesarchivo(...)`**: leen el archivo real línea por línea con `fstream`/`getline`, reservan el arreglo del tamaño exacto (con `contarlineasvalidas`) y van guardando cada línea convertida.
- **`buscarumbral(...)`**: busca la posición de un umbral por su etiqueta (T, P_SIS, P_DIA, E, O) dentro de la configuración.
- **`fueradelrango(...)`**: compara un valor contra el mínimo/máximo del umbral de esa etiqueta.
- **`esanomalo(...)`**: usa `fueradelrango` para decidir si una lectura de temperatura, presión u oximetría es anómala. ECG no se evalúa aquí.
- **`contarmedicionespaciente(...)`**: cuenta cuántas mediciones tiene un paciente en toda la sala.
- **`recogermedicionespaciente(...)`**: junta todas las mediciones de un paciente en un arreglo nuevo (reservado con `new` del tamaño exacto).
- **`ordenarmedicionesporfecha(...)`**: ordena las mediciones de la más antigua a la más reciente, con bubble sort.
- **`obtenermedicionesordenadas(...)`**: junta las dos funciones anteriores, porque siempre se usan juntas.
- **`actualizarminmax(...)`**: actualiza un mínimo y un máximo con un valor nuevo (para no repetir esa lógica en cada estadística).
- **`generarreportepaciente(...)`**: escribe `mediciones_paciente_<id>.txt`, agrupado por sensor, marcando ANOMALA/NORMAL (menos en ECG) y con mínimo/máximo/promedio al final de cada grupo.

### Lectura de archivos binarios (Sprint 3)

- **`leerbyte(...)`**, **`leerentero4bytes(...)`**, **`leerdouble(...)`**: leen 1, 4 y 8 bytes de un archivo binario respectivamente, usando `fstream::read` y `reinterpret_cast`.
- **`liberarsala(...)`**: libera toda la memoria dinámica de una `salauci` (lecturas, mediciones y máquinas), de adentro hacia afuera.
- **`leerbsf(...)`**: lee el archivo `.bsf` completo y llena una `salauci`, reservando cada arreglo con `new` justo después de leer su cantidad. Empieza llamando a `liberarsala` para no perder memoria si ya había datos cargados.
- **`generarreporteanomalias(...)`**: recorre todos los pacientes y escribe `anomalias.txt` con las lecturas anómalas (menos ECG), agrupadas por paciente.
- **`valorabsoluto(...)`**: calcula el valor absoluto de un número a mano (con un `if`), porque `fabs` no se vio en clase.
- **`esecganomalo(...)`**: decide si el ECG de un paciente es anómalo, comparando `|mínimo| + |máximo|` de sus lecturas ECG contra los límites de configuración.
- **`exportarpacientesecganomalos(...)`**: genera `pacientes_ecg_anomalos.dat` con los pacientes que tienen ECG anómalo.

### Pruebas

- **`contarocurrencias(...)`**: cuenta cuántas veces aparece una palabra dentro de un texto (usada para verificar los reportes).
- **`armarmedicion(...)`** y **`armarmedicion2(...)`**: arman una `medicion` de prueba con una o dos lecturas, para no repetir el mismo código en cada dato de prueba.
- **`pruebas()`**: corre todas las verificaciones del proyecto (separar, convertir, comparar fechas, detectar anomalías, leer el `.bsf` real y generar los tres reportes), y libera toda la memoria que reservó al final. Se ejecuta desde la opción 6 del menú.

### Menú

- **`mostrarmenu()`**: imprime las opciones del menú.
- **`main()`**: controla el ciclo del menú y llama a la función correspondiente según la opción elegida. Libera la memoria de `conf`/`pac`/`sala` antes de volver a cargarlas y antes de salir del programa.
