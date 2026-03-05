# Capítulo 7

- É o capítulo que começa a trilha para a construção de um User Mode.

- Aqui ele fala que o User Mode é o modo em que o usuário pode executar programas, prevenindo programas maliciosos/mal-escritos de cometer erros com o sistema, visto que se executasse em Kernel Mode, teria acesso absoluto ao hardware.

- Apesar disso, neste capítulo iremos executar um programa pequeno diretamente em Kernel Mode, visto que ainda temos um longo caminho até a implementação de um User Mode, que dependeria de criarmos vários drivers e sistemas de arquivos que possibilitariam carregar software diretamente do CD-ROM, HD ou outra forma de armazenamento persistente.

## Módulos no GRUB

- Para tal, usaremos uma ferramenta presente no GRUB chamada `modules`, que permitirá carregar o programa sem a infraestrutura citada anteriormente.

- O GRUB pode carregar arquivos arbitrários para a memória a partir da imagem ISO, sendo estes arquivos chamados de "módulos". Assim, precisaremos editar o `iso/boot/grub/menu.lst` para permitir escolhermos módulos.

- Foi necessário também editar o código que chama a `kmain` para passar a informação de onde encontrar os módulos (em `loader.s` e `kmain.c`).

## Teste de execução de programa curto

- Para efetuar o teste de rodar um programa curto, basta tentar rodar um programa que escreve um valor em um registrador e checar nos logs do Bochs se aparece o número correto para verificar que o programa executou corretamente.

- Para isso, criamos o arquivo `iso/modules/program.s`.

- Como nosso Kernel não consegue processar formatos executáveis avançados, precisamos compilar o programa para um binário com o NASM usando a flag `-f`.

## Multiboot e passagem de argumentos

- Antes de executar, é preciso identificar aonde estão os módulos e passar o endereço para `kmain()` como argumento, que estará no registrador `ebx`.

- Precisamos então fazer o download da multiboot structure, num `multiboot.h` que foi fornecido pelo livro (o link do livro não funcionava mais, tive que procurar na internet), e então alterar os argumentos do `kmain()` e adicionar duas linhas que manipulam este endereço.

- Link usado para conseguir o `multiboot.h`: <https://www.gnu.org/software/grub/manual/multiboot/html_node/multiboot_002eh.html>

- Foi necessário também adicionar checagens de que o endereço armazenado no registrador `ebx` é válido: checando as flags do `struct`, se o `mods_count` do `struct` é igual a `1` e se o intervalo de memória do módulo carregado corresponde ao módulo pretendido, evitando ler código inválido.

## Execução do módulo

- Ao fim, foi preciso criar um ponteiro de função que aponta para o endereço do código do módulo a ser executado, para que a CPU trate aquele trecho da memória como uma função, chamando a variável que foi definida como esse ponteiro de função e que armazena o endereço do início do módulo, ou seja, iniciando o código do programa teste.

## Ajustes extras e observações

- Precisei realizar algumas alterações no `loader.s` para fazer rodar o teste, principalmente no `ALIGN_MODULES`, que segundo o livro deveria ser usado no lugar de `FLAGS`, que causava erro, além de algumas pequenas alterações.

- Também foi necessário adicionar um header `BITS 32` no `program.s`, pois sem isso ele estava sendo montado como 16 bit, e ao ser chamado como 32 bit ele gerava instruções inválidas e ficava em loop reiniciando o boot.

- Adicionei também uma mensagem que aparece no Bochs para termos a certeza de que finalizou a execução do `program.s`, que entra em um loop, escrevendo `MOD LOOP` em branco e azul.

- Aproveitei para automatizar o comando NASM que transforma o `program.s` em binário por meio do `Makefile`. Isso é necessário, pois nosso Kernel ainda não pode fazer o parsing de formatos de executável mais avançados, portanto precisamos do programa executável já em binário.

- OBS: note que o `0xDEADBEEF` aparecerá no `bochslog.txt` em minúscula, isso é uma convenção do formato hexadecimal do Bochs, mas mantém o mesmo valor do original maiúsculo, o que significa que o Bochs tem hexadecimal case-insensitive.
