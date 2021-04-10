# Web server

## Introducción
Este es el código para la primera verificación de las prácticas de SSTT.
Alumno: Marcos Gil Espinosa
Grupo: 9
Curso: 2020/2021

## Descripción del proyecto

El código se encuentra en la carpeta `src` y está compuesto de un programa
principal y de las carpetas `src/dg` y `src/http` en las que se ofrece,
respectivamente, funciones para facilitar el logging y TDAs para representar
y manejar programáticamente cookies y mensajes http.

Cada fichero [x] de código en el módulo http va acompañado de otro fichero
[x.test.c] con tests unitarios, realizados con la biblioteca Unity (los test
son el único sitio en que se emplea esta biblioteca)

La carpeta build contiene los ficheros objeto y los ejecutables. No se
interactua con esta carpeta directamente, sino a través de `make`.

La carpeta `resources` contiene el fichero de bitácora, ficheros empleados por
los test unitarios y una carpeta `resources/web` con los ficheros que conforman
la web en sí.

## Compilación y ejecución
El `Makefile` del proyecto permite la ejecución de varias recetas:
- `build` para compilar el proyecto.
- `start` para compilar y ejecutar el proyecto.
- `stop` para detener el programa.
- `log` para consultar la bitácora.
- `clean` para limpiar eliminar los ficheros objeto.
- `clean_log` para vaciar la bitácora.
