# Tarea 1 — Implementación de la ALU y Análisis de ILP

## Descripción

Este programa implementa una **ALU (Arithmetic Logic Unit)** de 32 bits en C y analiza el **Instruction-Level Parallelism (ILP)** comparando la ejecución secuencial (sin pipeline) contra una simulación software de un pipeline de 4 etapas.

---

## Estructura del código

```
alu_pipeline.c
├── alu()        → Función ALU (8 operaciones)
├── generar()    → Generación de datos de prueba
├── run_seq()    → Ejecución secuencial (sin pipeline)
├── run_pipe()   → Simulación de pipeline de 4 etapas
└── main()       → Orquestación y reportes
```

---

## 1. Función ALU

La función `alu(op, a, b)` recibe un **código de operación entero** y dos operandos de 32 bits sin signo (`uint32_t`), y devuelve el resultado de la operación correspondiente.

### Tabla de códigos de operación

| Código (int) | Nombre | Operación            | Expresión C         |
|:------------:|--------|----------------------|---------------------|
| `0`          | `ADD`  | Suma                 | `a + b`             |
| `1`          | `SUB`  | Resta                | `a - b`             |
| `2`          | `AND`  | AND bit a bit        | `a & b`             |
| `3`          | `OR`   | OR bit a bit         | `a \| b`            |
| `4`          | `XOR`  | XOR bit a bit        | `a ^ b`             |
| `5`          | `NOT`  | Complemento de `a`   | `~a`                |
| `6`          | `SHL`  | Desplazamiento izq.  | `a << (b & 31)`     |
| `7`          | `SHR`  | Desplazamiento der.  | `a >> (b & 31)`     |

> **Nota sobre SHL/SHR:** el desplazamiento se limita a `b & 31` para evitar comportamiento indefinido en C al desplazar 32 o más bits.

### Ejemplo de uso

```c
uint32_t resultado = alu(ADD, 100, 200); // → 300
uint32_t mascara   = alu(AND, 0xFF00, 0x0FF0); // → 0x0F00
```

---

## 2. Secuencia de prueba

Se genera un arreglo de **1 000 000 de pares** de enteros de 32 bits con semilla fija (`srand(42)`) para reproducibilidad:

```c
#define N 1000000
Pair pairs[N]; // cada Pair contiene { uint32_t a, b }
```

Las **8 operaciones** se aplican secuencialmente sobre todos los pares, dando un total de:

```
Total instrucciones = 1 000 000 × 8 = 8 000 000
```

El tiempo se mide con `clock_gettime(CLOCK_MONOTONIC)` para máxima precisión.

---

## 3. Análisis ILP

### Pipeline de 4 etapas (simulación software)

La función `run_pipe()` simula las siguientes etapas con latencia unitaria (1 ciclo cada una):

```
┌─────────┐   ┌─────────┐   ┌─────────┐   ┌────────────┐
│  Fetch  │ → │ Decode  │ → │ Execute │ → │ Write-back │
│ Stage 0 │   │ Stage 1 │   │ Stage 2 │   │  Stage 3   │
└─────────┘   └─────────┘   └─────────┘   └────────────┘
```

Cada ciclo simulado avanza las instrucciones a través del pipeline. En el estado estacionario, una nueva instrucción es emitida (Fetch) mientras otra se completa (Write-back), logrando **CPI ≈ 1**.

### Valores teóricos vs. medidos

| Métrica          | Sin pipeline (teórico) | Con pipeline (teórico) | Resultado esperado        |
|------------------|:----------------------:|:----------------------:|---------------------------|
| CPI              | ≈ 4                    | ≈ 1                    | Pipeline ~4× más rápido   |
| Throughput       | `Total / (4 × Total)`  | `Total / (1 × Total)`  | Depende del hardware real |
| Speedup          | 1×                     | ~4×                    | `t_seq / t_pipe`          |

### Comentario sobre desviaciones

- **Sin pipeline real:** La ejecución secuencial en hardware moderno no equivale a CPI = 4, ya que el procesador aplica su propio pipeline y optimizaciones (branch prediction, out-of-order execution). El tiempo medido puede ser **mucho menor** al teórico puro.
- **Con pipeline simulado:** La simulación software tiene overhead adicional (ciclo del bucle, variables de estado), por lo que `t_pipe` puede ser **mayor** que `t_seq` en algunas máquinas, reflejando que simular hardware con software tiene un costo propio que no representa el hardware real.
- **Speedup real:** El speedup calculado (`t_seq / t_pipe`) mide la diferencia entre los dos enfoques *en la simulación*, no en silicio real.

---

## Requisitos del sistema

| Componente | Mínimo requerido |
|------------|-----------------|
| SO | Linux, macOS, o WSL2 en Windows |
| Compilador | GCC 7+ o Clang 6+ con soporte C99 |
| Librerías | Ninguna externa — solo la biblioteca estándar de C (`libc`) |
| RAM | ~16 MB (el arreglo de pares ocupa ≈ 8 MB) |

---

## Instalación del compilador

### Linux (Debian / Ubuntu)
```bash
sudo apt update
sudo apt install gcc -y

# Verificar instalación
gcc --version
```

### Linux (Fedora / RHEL)
```bash
sudo dnf install gcc -y
```

### macOS
```bash
# Instala las herramientas de línea de comandos de Xcode (incluye clang)
xcode-select --install

# O instala GCC vía Homebrew
brew install gcc
```

### Windows (WSL2)
```powershell
# En PowerShell como administrador, activar WSL y luego desde Ubuntu:
sudo apt update && sudo apt install gcc -y
```

---

## Obtener el código

### Opción A — Clonar el repositorio con Git
```bash
git clone https://github.com/tu-usuario/tu-repositorio.git
cd tu-repositorio
```

### Opción B — Descargar el archivo directamente
```bash
# Con wget
wget https://raw.githubusercontent.com/tu-usuario/tu-repositorio/main/alu_pipeline.c

# O con curl
curl -O https://raw.githubusercontent.com/tu-usuario/tu-repositorio/main/alu_pipeline.c
```

### Opción C — Crear el archivo manualmente
Si ya tienes el código fuente, crea el archivo con cualquier editor:
```bash
nano alu_pipeline.c    # pegar el código y guardar con Ctrl+O → Ctrl+X
# o
vim alu_pipeline.c
# o
code alu_pipeline.c    # VS Code
```

---

## Compilación

Una vez que tengas `alu_pipeline.c` en tu directorio de trabajo:

```bash
# Opción recomendada (sin optimizaciones, para medición fiel)
gcc -O0 -o alu_pipeline alu_pipeline.c

# Con información de depuración
gcc -O0 -g -o alu_pipeline alu_pipeline.c

# Con Clang en lugar de GCC
clang -O0 -o alu_pipeline alu_pipeline.c
```

> ⚠️ Se recomienda **`-O0`** para desactivar las optimizaciones automáticas del compilador. Con `-O2` o `-O3` el compilador puede vectorizar o reordenar las operaciones, lo que distorsiona la comparación ILP entre los dos modos.

Verificar que el binario fue creado correctamente:
```bash
ls -lh alu_pipeline
```

---

## Ejecución

```bash
./alu_pipeline
```

El programa **no recibe argumentos**; todos los parámetros (`N`, `NUM_OPS`, semilla) están definidos en el código fuente como constantes.

### Salida esperada

```
Total instrucciones: 8000000

Sin pipeline:
  Tiempo: X.XXXX s
  Throughput: XX.XX MIPS

Con pipeline:
  Tiempo: X.XXXX s
  Ciclos: XXXXXXXX
  CPI: X.XXXX
  Throughput: XX.XX MIPS

Speedup: X.XXx
```

### Guardar la salida en un archivo
```bash
./alu_pipeline | tee resultados.txt
```

---

## Modificar parámetros

Los parámetros principales están al inicio de `alu_pipeline.c`:

```c
#define N       1000000   // Cantidad de pares de operandos
#define NUM_OPS 8         // Número de operaciones ALU
```

Para cambiarlos, edita el archivo y recompila:
```bash
nano alu_pipeline.c      # modificar N o NUM_OPS
gcc -O0 -o alu_pipeline alu_pipeline.c
./alu_pipeline
```

---

## Autores

> _Isabella Agudel, Laura Jimenez, Dady Loaiza, Martin Pinzon._

## Fecha

Abril 2026
