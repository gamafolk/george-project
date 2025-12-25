cmake_minimum_required(VERSION 3.10)
project(RoboAutonomo VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories for third-party libraries
include_directories(third_party/whisper.cpp/include third_party/whisper.cpp/ggml/include third_party/llama.cpp/include third_party/llama.cpp/ggml/include)

# Link directories for prebuilt libraries
link_directories(${CMAKE_SOURCE_DIR}/lib)

# =================================================================
# 2. CONFIGURAÇÃO DO EXECUTÁVEL PRINCIPAL
# =================================================================

# Adiciona o executável principal, chamado __app
add_executable(__app main.cpp)

# Vincula as bibliotecas ao nosso executável
target_link_libraries(__app PRIVATE whisper llama ggml ggml-base ggml-cpu mtmd) 


# =================================================================
# 3. CORREÇÃO DE DLLs E CÓPIA DE ARQUIVOS (POST-BUILD)
# =================================================================

# Define as bibliotecas que devem ser copiadas (incluindo as dependências GGML)
set(DEPENDENCY_DLLS whisper llama ggml ggml-base ggml-cpu mtmd)

# Itera sobre as dependências e cria um comando de cópia para cada DLL/lib
foreach(DEP ${DEPENDENCY_DLLS})
    add_custom_command(TARGET __app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/lib/lib${DEP}.so # Localiza o arquivo da DLL/lib
            $<TARGET_FILE_DIR:__app>/lib${DEP}.so # Copia para a pasta de saída
        COMMAND_EXPAND_LISTS)
endforeach()


# O comando POST_BUILD garante que a pasta models seja copiada para a pasta de saída (build)
add_custom_command(TARGET __app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        "${CMAKE_CURRENT_SOURCE_DIR}/models" 
        "$<TARGET_FILE_DIR:__app>/models"
    COMMAND_EXPAND_LISTS)