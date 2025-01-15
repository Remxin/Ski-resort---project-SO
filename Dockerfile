# Dockerfile
FROM gcc:latest

# Instalacja narzędzi niezbędnych do programowania wielowątkowego
RUN apt-get update && apt-get install -y \
    make \
    gdb \
    valgrind \
    && apt-get clean

# Ustawienie domyślnego katalogu roboczego
WORKDIR /app

# Eksponuj katalog roboczy na zewnątrz
VOLUME ["/app"]

# Domyślny polecenie (można nadpisać przy uruchamianiu kontenera)
CMD ["/bin/bash"]
