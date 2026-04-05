# OS-PROJECT

## Descrição
Desenvolvimento do Kernel de um sistema operacional, como projeto final da disciplina de Sistemas Operacionais, seguindo os direcionamentos do livro littleosbook(https://littleosbook.github.io/).

O projeto contempla a construção de um kernel básico com inicialização via bootloader, configuração de IDT e GDT, tratamento de interrupções, Utilização de módulos GRUB, Paginação da memória e um sistema virtual de arquivos em conjunto com um mini-shell.

- Período: 2025.2
- Professor: Davi Henrique dos Santos

## Tecnologias Utilizadas:
- C
- Assembly x86
- GCC 
- NASM
- Make
- GRUB
- Bochs

## Requisitos e Dependências:

### No linux (Ubuntu):
```bash
#Instalação dos pacotes necessários
sudo apt update
sudo apt install build-essential libsdl1.2-dev wget tar
sudo apt-get install build-essential nasm genisoimage bochs bochs-sdl make bochsbios vgabios
#Instalação do bochs 2.7
wget https://downloads.sourceforge.net/project/bochs/bochs/2.7/bochs-2.7.tar.gz
tar xvf bochs-2.7.tar.gz
cd bochs-2.7
#Configuração do bochs, forçando a utilização do sdl
sudo ./configure --enable-x86-64 --with-sdl --enable-gdb-stub --enable-all-optimizations
make -j$(nproc)
sudo make install
```

## Instruções de compilação e execução:
Para compilar e rodar o projeto:
```bash
make run
```
Para limpar os arquivos compilados e os arquivos gerados pelo SO:
```bash
make clean
```

## Funcionalidades

- Inicialização do kernel via bootloader GRUB (Multiboot)
- Geração de imagem ISO bootável via genisoimage
- Execução em ambiente virtualizado (Bochs)
- Driver de vídeo em modo texto via framebuffer VGA (80×25)
- Driver de porta serial COM1 para log/depuração
- Captura de entrada via teclado (scan code → ASCII)
- Configuração da GDT (Global Descriptor Table)
- Configuração da IDT (Interrupt Descriptor Table) para tratamento de interrupções
- Remapeamento do PIC (Programmable Interrupt Controller)
- Paginação com modelo Higher-Half Kernel (0xC0100000)
- Gerenciamento de memória física via bitmap (PMM)
- Gerenciamento de memória virtual com mapeamento dinâmico de páginas (VMM)
- Heap do kernel com kmalloc/kfree (lista encadeada de blocos livres)
- Sistema virtual de arquivos em RAM (VFS + RamFS com inodes)
- Mini-shell interativo com comandos: ls, cd, mkdir, touch, rm, read, write, pwd, clear, stop


## Fluxo De execução:

```
GRUB (Multiboot)
  │
  ▼
loader.s ─── Paginação inicial + Higher-Half Jump
  │
  ▼
kmain() ─── Inicialização dos subsistemas do kernel
  │
  ├── serial_init()        Porta serial COM1
  ├── gdt_init()           Segmentação (GDT)
  ├── pic_remap()          Remapeamento do PIC
  ├── idt_init()           Tabela de interrupções (IDT)
  ├── sti                  Habilita interrupções
  ├── pmm_init()           Gerenciador de memória física
  ├── kheap_init()         Heap do kernel
  ├── vfs_init()           Sistema virtual de arquivos
  ├── shell_init()         Mini-shell interativo
  │
  ▼
idle_forever() ─── Loop com interrupções habilitadas (sti; hlt)
  │
  ▼ (a cada tecla pressionada)
IRQ1 → interrupt_handler_33 → read_letter() → shell_handle_key()
```

## Estrutura do projeto:

```
include             Diretório que contém os headers do projeto
source              Diretório que mantém o código c/assembly do projeto
linker              Diretório que possui o link.ld

source/boot         Contém o código responsável por realizar a inicialização do kernel
source/io           Possui o código responsável pelo framebuffer e pelo serial port
source/kernel       Mantém o kmain.c, o código principal do kernel
source/segmentation Contém o código responsável por segmentar a memória ram
source/interrupt    Contém o código que lida com interrupts do kernel
source/paging       Contém o código que é responsável por realizar a paginação dentro do C
source/file         Contém o código que inicializa o sistema virtual de arquivos
source/shell        Contém o código que inicializa o mini-shell que é utilizado para demonstar o sistema virtual de arquivos

Makefile            Compila e roda os códigos do kernel
```

## Contribuições individuais:
- Cap 1-3: Luigi, Kevin, Luís
- 

## Referências:

- [The Little OS Book](https://littleosbook.github.io/)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [OSDev Wiki](https://wiki.osdev.org/)

## Licença:

[MIT](LICENSE)
