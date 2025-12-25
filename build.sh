#!/bin/bash

# Script para build e execução do projeto George

echo "Iniciando build do projeto..."

# Verifica se bibliotecas pré-compiladas existem; se não, builda submódulos
if [ ! -f "lib/libwhisper.so" ]; then
    echo "Bibliotecas pré-compiladas não encontradas. Preparando..."
    
    # Build Whisper
    echo "Buildando Whisper..."
    cd third_party/whisper.cpp
    mkdir -p build
    cd build
    cmake ..
    make
    cd ../../..
    
    # Build Llama
    echo "Buildando Llama..."
    cd third_party/llama.cpp
    mkdir -p build
    cd build
    cmake .. -DLLAMA_CURL=OFF
    make
    cd ../../..
    
    # Cria lib/ e copia bibliotecas
    echo "Copiando bibliotecas para lib/..."
    mkdir -p lib
    cp third_party/whisper.cpp/build/src/libwhisper.so* lib/
    cp third_party/llama.cpp/build/bin/libllama.so* lib/
    cp third_party/llama.cpp/build/bin/libggml*.so* lib/
    cp third_party/llama.cpp/build/bin/libmtmd.so* lib/
    
    echo "Bibliotecas preparadas."
else
    echo "Bibliotecas pré-compiladas encontradas. Pulando build de submódulos."
fi

# Cria diretório build se não existir
mkdir -p build

# Entra no diretório build
cd build

# Configura CMake
echo "Configurando CMake..."
cmake ..

# Compila
echo "Compilando..."
make

# Executa se compilou com sucesso
if [ $? -eq 0 ]; then
    echo "Executando..."
    ./__app
else
    echo "Erro na compilação!"
    exit 1
fi