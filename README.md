# Stow Programming Language

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)

**Stow** es un lenguaje de programación interpretado moderno, diseñado para ser simple, expresivo y fácil de aprender. Combina sintaxis clara con características poderosas.

## 🚀 Características

- ✨ **Sintaxis Clara**: Fácil de leer y escribir
- 🔧 **Funciones con Parámetros**: Define funciones reutilizables
- 🔄 **Bucles While**: Con soporte para `break` y `continue`
- 📦 **Sistema de Imports**: Modulariza tu código
- 🎯 **Tipado Explícito**: `Int`, `Str`, `Float`, `Bool`, `List`
- 💬 **Comentarios**: Soporta `//` y `/* */`
- 🔢 **Operadores Completos**: Matemáticos y lógicos
- 📍 **Errores con Línea**: Debugging más fácil
- 🖥️ **REPL Interactivo**: Prueba código al instante

## 📋 Requisitos

- GCC (GNU Compiler Collection)
- WSL (Windows Subsystem for Linux) o Linux/macOS

## 🛠️ Instalación

```bash
# Clonar el repositorio
git clone https://github.com/tu-usuario/stow.git
cd stow

# Compilar
gcc -Iinclude src/main.c src/lexer.c src/parser.c src/interpreter.c -o stow

# Ejecutar un archivo
./stow examples/demo.stow

# O iniciar el REPL
./stow
```

## 📖 Sintaxis Básica

### Variables

```stow
val nombre: Str = "Paolo";      // Constante
var edad: Int = 25;             // Variable mutable
val pi: Float = 3.14159;
var activo: Bool = "true";
```

### Funciones

```stow
func saludar(nombre: Str): Void {
    print("Hola " + nombre + "!");
}

func sumar(a: Int, b: Int): Int {
    return a + b;
}

saludar("Mundo");
val resultado: Int = sumar(10, 20);
```

### Condicionales

```stow
if (edad > 18) {
    print("Mayor de edad");
} else {
    print("Menor de edad");
}
```

### Bucles

```stow
var i: Int = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}

// Con break y continue
while (i < 10) {
    if (i == 3) {
        continue;  // Saltar iteración
    }
    if (i == 7) {
        break;     // Salir del bucle
    }
    i = i + 1;
}
```

### Imports

```stow
import "libreria.stow";
```

### Entrada del Usuario

```stow
val nombre: Str = input("¿Cómo te llamas? ");
print("Hola " + nombre);
```

### Listas

```stow
val numeros: List = [1, 2, 3, 4, 5];
val nombres: List = ["Ana", "Juan", "Pedro"];
```

## 📂 Estructura del Proyecto

```
Stow/
├── src/              # Código fuente
│   ├── main.c
│   ├── lexer.c
│   ├── parser.c
│   └── interpreter.c
├── include/          # Headers
│   └── stow.h
├── examples/         # Ejemplos
│   ├── demo.stow
│   └── debug.stow
├── Makefile          # Script de compilación
├── errors.json       # Errores
├── README.md
└── .gitignore
```

## 🎯 Ejemplos

### Hello World

```stow
print("¡Hola, Mundo!");
```

### Fibonacci

```stow
func fibonacci(n: Int): Int {
    if (n < 2) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

var i: Int = 0;
while (i < 10) {
    print(fibonacci(i));
    i = i + 1;
}
```

### Calculadora Interactiva

```stow
val a: Str = input("Primer número: ");
val b: Str = input("Segundo número: ");

print("Suma: " + (a + b));
print("Resta: " + (a - b));
print("Multiplicación: " + (a * b));
print("División: " + (a / b));
```

## 🐛 Códigos de Error

| Código | Descripción |
|--------|-------------|
| E001   | Se esperaba '(' |
| E002   | Se esperaba ')' |
| E003   | Se esperaba ';' |
| E004   | Se esperaba ':' después del nombre de variable |
| E005   | Se esperaba '=' para asignación |
| E006   | Tipo de dato no válido |
| E007   | Variable no definida |
| E008   | Se esperaba '{' |
| E009   | Se esperaba '}' |
| E010   | Función no definida |
| E011   | Número incorrecto de argumentos |

## 🤝 Contribuir

Las contribuciones son bienvenidas. Por favor:

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## 📝 Licencia

Este proyecto está bajo la Licencia MIT. Ver el archivo `LICENSE` para más detalles.

## 👨‍💻 Autor

**Paolo** - Creador de Stow

## 🙏 Agradecimientos

- Inspirado en lenguajes modernos como Kotlin, Swift y Rust
- Gracias a la comunidad de desarrolladores de lenguajes

---

**¿Preguntas o sugerencias?** Abre un issue en GitHub o contacta al autor.
