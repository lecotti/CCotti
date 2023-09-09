# CCotti
Librería de C++ que sirve como API para implementar IPCs System V, signals, sockets y pthreads de C.

## Instalación
Agregar en el `CMakeLists.txt` de tu proyecto:
```
target_include_directories(<target_name> "${path}/CCotti/include")
target_link_libraries(<target_name> "${path}/Ccotti/lib/libipc_lib.a")
```

## Workflow
Para ejemplos de uso, revisar la carpeta "test". Para ver las explicaciones de cada función, revisar los archivos dentro de "src". Para ver los prototipos de las funciones, revisar "include".

## Compilación y testeo

```
$ cd build
$ cmake ..
$ cmake --build .
$ ctest --verbose
$ cmake --install . --prefix "$(pwd)/../install"
```

## Known issues
1. Para Ipv6 "link local addresses" (las locales del router, que empiezan con "fe80:"), getaddrinfo() no completa en la struct sockaddr_in6 el campo "sin6_scope_id", y al querer conectar o bindear devuelve error.

2. Un socket que se haya conectado exitosamente a otro, pero que luego el socket ajeno se cierre, produce una SIGPIPE luego de **DOS** escrituras en el socket. El primer llamado carga el buffer y retorna la cantidad de bytes escritos en el buffer, y el segundo llamado ya se da cuenta que la conexión se rompió.
