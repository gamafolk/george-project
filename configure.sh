#!/bin/bash

# Script para build e execução do projeto George

echo " > Iniciando processo de build"

# Baixa modelos se não existirem
echo "Verificando modelos..."

mkdir -p models

if [ ! -f "models/ggml-tiny.en.bin" ] || [ ! -s "models/ggml-tiny.en.bin" ]; then
    echo "Baixando ggml-tiny.en.bin..."
    wget -O models/ggml-tiny.en.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.en.bin
fi

if [ ! -f "models/phi-2.Q4_K_M.gguf" ] || [ ! -s "models/phi-2.Q4_K_M.gguf" ]; then
    echo "Baixando phi-2.Q4_K_M.gguf..."
    wget -O models/phi-2.Q4_K_M.gguf https://huggingface.co/TheBloke/phi-2-GGUF/resolve/main/phi-2.Q4_K_M.gguf
fi

echo "Modelos prontos."

# Atualiza submódulos git
if [ ! -d "lib" ]; then

    echo " > Atualizando submódulos git"

    mkdir -p third_party

    git submodule update --init --recursive --force
    
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

echo "Configuração finalizada. Para executar o projeto, use: ./__app"