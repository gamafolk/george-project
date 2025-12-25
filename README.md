# george-project

Projeto do Cérebro Autônomo do Robô, utilizando Whisper (ASR) e Llama (LLM) para processamento de fala e geração de texto.

## Dependências

- CMake
- Compilador C++ (g++)
- Git (para submódulos)

## Setup Inicial

1. Clone o repositório e inicialize submódulos:

   ```
   git clone <repo>
   git submodule update --init --recursive
   ```

2. Baixe os modelos:
   - Modelo Whisper: `ggml-tiny.en.bin` (já incluído em `models/`)
   - Modelo Llama: `phi-2.Q4_K_M.gguf` (já incluído em `models/`)

## Como Compilar e Executar

### Opção 1: Script Automático (Recomendado)

Execute o script que faz tudo:

```
./build.sh
```

### Opção 2: Manual

```
mkdir -p build
cd build
cmake ..
make
./__app
```

### Opção 3: VS Code

- Pressione `Ctrl+Shift+B` para executar a tarefa "Build and Run C++" configurada.

### Opção 4: Extensão VS Code (Botão na Barra de Status)

Instale a extensão `ControlsIO` (localizada em `vscode-extension/controlsio/controlsio-0.0.1.vsix`) para adicionar um botão na barra de status que executa o build automaticamente.

- Instale a extensão: Abra o VS Code, vá em Extensions > Install from VSIX > selecione o arquivo `.vsix`.
- Após instalar, recarregue o VS Code.
- Um botão "Build & Run" aparecerá na barra de status (esquerda). Clique para executar.

## Estrutura

- `main.cpp`: Código principal
- `models/`: Modelos de IA
- `third_party/`: Submódulos (whisper.cpp, llama.cpp)
- `build/`: Arquivos de build (ignorados pelo Git)
