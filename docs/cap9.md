- Por que não fazer Identity Map com o Kernel?
    Pois quando for fazer o linking do código de um processo 
    em User Mode isso causará problemas, visto que o linker assume que
    o código será alocado na posição 0x00000000 da memória. Isso pode ser corrigido
    usando um script de linker que avisa ao linker para assumir uma posição de começo
    de memória diferente, mas acaba sendo demasiado complicado para os usuários do SO.

    Além disso, dessa forma nós assumimos que queremos o Kernel como parte do Address Space 
    do User Mode, visto que dessa forma quando ocorrer System Calls não será preciso alterar 
    a estrutura de páginas para acessar o código e dados do Kernel, economizando o tempo de troca 
    de contexto. Apesar disso, essas páginas do Kernel precisarão de nível de privilégio 0 (Ring 0)
    para acesso, como modo de previnir um processo de usuário de ler ou escrever em memória de Kernel.

- Onde devemos alocar o Kernel então?
    Para evitar que a alocação de memória dos processos de usuário atinjam o espaço alocado para o
    Kernel, devemos posicioná-lo num espaço de memória bem alto em relação ao início do Address Space.
    Neste caso, podemos usar o espaço a partir dos 3 GB(0xC0000000) de memória, para separar completamente 
    o espaço de usuário do espaço de Kernel.

    Esta técnica é denominada Higher-Half Kernel, e caso os processos de usuário consumam mais de 3 GB,
    algumas páginas precisarão sofrer "swap out" pelo Kernel, mas isto está fora do escopo do livro.

    Mesmo assim, ainda não é o ideal posicionar o Kernel em 0xC0000000, mas em 0xC0100000, pois de começo
    o Kernel já será carregado pelo GRUB em 0x00100000, visto que tudo abaixo de 1 MB foi utilizado pelo GRUB,
    BIOS, bootloader e outros programas de inicialização que podem ter deixado lixo na memória ou estarem reservados 
    quando o Kernel for alocado.

    Dessa forma, conseguimos manter um offset constante de 0xC0000000 para acessar o Kernel, visto que o endereço
    virtual do Kernel torna-se 0xC0000000(deslocamento higher-half) + 0x00100000(endereço físico real), conseguindo 
    mapear toda a memória do Kernel de forma linear a partir deste endereço virtual. Assim, o espaço de memória física baixa 
    será mapeada de (0x00000000, 0x00100000) para o endereço virtual (0xC0000000, 0xC0100000), 
    bem como o Kernel sairá de 0x00100000 para 0xC0100000.

    0x00000000 até 0xBFFFFFFF → espaço de usuário
    0xC0000000 até 0xFFFFFFFF → espaço do kernel

- Explicando o ponto principal de forma simples: cada PDT terá algumas PDEs reservadas para mapear o espaço virtual 
do Kernel (na região a partir de 0xC0000000), onde o Kernel propriamente dito começa em 0xC0100000, permitindo que 
os processos realizem System Calls sem precisar trocar o mapeamento do Kernel.

- Para efetivamente alocar o Kernel em 0xC0100000(evitar erros em caso de não ter pelo menos 3GB de memória), 
usa-se a relocação .=0xC0100000 e a instrução AT no linker script, sendo a relocação responsável por especificar 
que referências não-relativas à memória devem usar o endereço de realocação como base para calcular os endereços, 
enquanto a AT é responsável por especificar aonde o Kernel deve ser carregado na memória, que será feito pelo GRUB.

- Após o Kernel ser carregado fisicamente em 0x00100000 e ser linkado para 0xC0100000, o paging ainda não foi ativado, portanto,
a CPU ainda não sabe fazer a tradução. A ideia da seção "Entering the Higher Half" é a seguinte:
    - Você começa executando “lá embaixo”, perto de 0x00100000, monta uma paginação temporária que mapeia o mesmo código em dois lugares virtuais, ativa paging, e então faz um salto para a versão alta do endereço, fazendo o EIP entrar de vez no higher half.

    O livro diz que, antes do salto para o higher half, nós precisamos usar um código assembly que evite dependência de endereços absolutos e faça três coisas:

    - montar a tabela de páginas;
    - criar identity mapping para os primeiros 4 MB do espaço virtual;
    - criar uma entrada que faça o mapeamento do endereço alto do kernel para o endereço físico onde ele foi carregado.

- Outro cuidado a se tomar quando utilizamos Higher-Half Kernel é ao usar I/O que foi mapeado para endereços específicos
da memória, como o framebuffer, que foi mapeado para 0x000B8000, mas como não há mais entrada para esse endereço na page table,
o endereço 0xC00B8000 deve ser utilizado, visto que o endereço virtual 0xC0000000 mapeia o endereço físico 0x00000000.
Dessa forma, qualquer referência explícita a endereços dentro da multiboot structure precisam ser alteradas para refletir
aos novos endereços virtuais.