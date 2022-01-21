# CCotti
Personal C++ library for multi process and multi threading.

## Known issues
1. Para Ipv6 "link local addresses" (las locales del router, que empiezan con "fe80:"), getaddrinfo() no completa en la struct sockaddr_in6 el campo "sin6_scope_id", y al querer conectar o bindear devuelve error.
