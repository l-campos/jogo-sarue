# 🐾 Saruê Saga

**Saruê Saga** é um jogo de plataforma 2D de ação e aventura desenvolvido inteiramente em **C++** utilizando a biblioteca **SDL2**. Sobreviva a pombos kamikazes, gatos implacáveis e escale obstáculos usando as habilidades únicas de um Saruê!

---

## 🛠️ Tecnologias Utilizadas

O jogo foi construído com C++11 e utiliza as seguintes bibliotecas da família SDL2:
* **SDL2** (Core, Janela, Eventos, Renderização)
* **SDL2_image** (Carregamento de texturas JPG, PNG)
* **SDL2_mixer** (Reprodução de Músicas e Efeitos Sonoros)
* **SDL2_ttf** (Renderização de Textos e Fontes)

---

## 🎮 Controles do Jogo

O Saruê possui um leque de movimentos dinâmicos para exploração e combate:

| Ação | Tecla / Botão |
| :--- | :--- |
| **Mover** | `A` / `D` ou `Setas Esquerda/Direita` |
| **Pular** | `Espaço` |
| **Ataque Corpo-a-Corpo** | `Botão Esquerdo do Mouse` |
| **Ataque para Cima** | `W` (ou Seta p/ Cima) + `Botão Esquerdo` |
| **Dash** | `Shift Esquerdo` |
| **Escalar Canos** | `W` / `S` ou `Setas Cima/Baixo` perto de um cano |
| **Descer de Plataformas** | `S` (ou Seta p/ Baixo) + `Espaço` |
| **Fingir de Morto** | Segurar `S` ou `Seta para Baixo` no chão |
| **Alternar Tela Cheia** | `F11` |
| **Sair / Voltar** | `ESC` |

*Dica de Sobrevivência:* Fingir de morto é uma ótima tática para fazer os pombos inimigos abortarem seus mergulhos!

---

## ⚙️ Dependências e Instalação

Para compilar o código-fonte, você precisará ter o compilador **g++** e as bibliotecas **SDL2** instaladas na sua máquina.

### 🐧 Linux (Ubuntu / Debian)
A instalação no Linux é a mais direta via terminal. Basta rodar o comando abaixo para instalar tudo o que é necessário:
```bash
sudo apt-get update
sudo apt-get install g++ make libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
```
## 🪟 Windows (WSL)

Para compilar no Windows, utilize o **Windows Subsystem for Linux (WSL)**.

### Instalação das dependências

Primeiro, atualize os pacotes do sistema:

```bash
sudo apt update
```

Em seguida, instale o compilador, o `make` e as bibliotecas SDL2:

```bash
sudo apt install build-essential make \
libsdl2-dev \
libsdl2-image-dev \
libsdl2-mixer-dev \
libsdl2-ttf-dev
```

---

## 🍎 macOS

No macOS, a forma mais simples de instalar as dependências é utilizando o **Homebrew**:

```bash
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf
```

---

## 🚀 Como Compilar e Executar

O projeto utiliza um **Makefile** que gerencia automaticamente os arquivos objeto (`.o`) e os diretórios de compilação.

Abra um terminal na pasta raiz do projeto.

### Compilar

```bash
make
```

O compilador criará automaticamente as pastas:

- `bin/`
- `dep/`

e gerará o executável do jogo.

### Executar

**Linux / WSL / macOS**

```bash
./jogo
```

---

## 🛠️ Comandos Úteis

### Limpar arquivos gerados

Remove o executável e os arquivos temporários da compilação.

```bash
make clean
```

### Compilar em modo de depuração

Compila o projeto com as flags de depuração (`-ggdb` e `-O0`), facilitando o uso do **GDB**.

```bash
make debug
```
