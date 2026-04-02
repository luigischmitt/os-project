# OS-PROJECT

## Descrição
Desenvolvimento do Kernel de um sistema operacional, como projeto final da disciplina de Sistemas Operacionais, seguindo os direcionamentos do livro littleosbook(https://littleosbook.github.io/).
O projeto contempla a construção de um kernel básico com inicialização via bootloader, configuração de IDT e GDT, tratamento de interrupções, Utilização de módulos GRUB, Paginação da memória e um sistema virtual de arquivos em conjunto com um mini-shell.

- Período: 2025.2
- Professor: Davi Henrique dos Santos

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

## Constribuições individuais:

## Referências:

- [The Little OS Book](https://littleosbook.github.io/)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [OSDev Wiki](https://wiki.osdev.org/)

## Licença:

[MIT](LICENSE)
