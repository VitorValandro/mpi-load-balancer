## Trabalho 2: Programação Paralela - INE 5645

#### Serviço de armazenamento de chave-valor com um mecanismo de balanceamento de carga com replicação

#### Vitor Matheus Valandro da Rosa (22102567)

### Como compilar e executar

#### Pré Requisitos

- GCC;
- MPI;

##### Compilar

Para compilar o programa, basta executar o seguinte comando:

```
mpicc -o key_db main.c load_balancer.c replica.c client.c key_value_db.c utils.c
```

##### Executar

Para executar o programa, basta executar o seguinte comando:

```
mpirun -np 6 ./key_db
```

6 é o número mínimo de processos para executar o programa. Este valor pode ser substituído para qualquer outro valor maior que ele.

#### Extra: executando com nós distribuídos em uma rede Docker

Com o objetivo de executar o programa em uma rede de nós distribuídos, tentei criar uma rede Docker com 6 nós, cada um com um container rodando o programa. Infelizmente, não tive tempo o suficiente para fazer funcionar 100%, atualmente parece que nem todos os containers estão se comunicando corretamente porque ao checar os logs é possível ver que algumas réplicas estão ignorando mensagens enviadas por clientes, o que não acontece ao executar o programa em um único nó. Apesar disso, o programa ainda funciona corretamente na rede docker, apenas algumas operações são perdidas.

Você pode tentar executar o programa em uma rede de nós distribuídos seguindo os passos abaixo.

##### Requisitos

Para executar o programa em uma rede de nós distribuídos, você precisa ter o Docker instalado em sua máquina, juntamente com o Docker Compose.

##### Build da imagem Docker

O arquivo `Dockerfile` contém as instruções para criar uma imagem Docker com o programa. Lá ele faz a configuração do SSH para as máquinas, copia todos os arquivos para dentro da imagem e compila o código. Para criar a imagem, basta executar o seguinte comando no diretório raiz do projeto:

```
docker-compose build
```

ou

```
docker build .
```

##### Criação das redes e dos nós

Para criar os containers e a rede existe o arquivo `docker-compose.yml` que contém as configurações individuais de cada container e da rede. Para criar a rede e os containers, basta executar o seguinte comando no diretório raiz do projeto (após a criação da imagem Docker):

```
docker-compose up -d
```

##### Adicionando mais containers

Se você quiser adicionar mais containers terá que registrá-los individualmente no arquivo `docker-compose.yml`, seguindo o padrão dos outros containers. É importante atribuir um IP único para cada container e depois atualizar o arquivo `hostfile` incluindo os novos containers para que o MPI possa reconhecê-los.

### Relatório Técnico

O objetivo desta seção é apresentar o funcionamento geral dos componentes do sistema e esclarecer as decisões de implementação. Detalhes na implementação de cada função e estrutura de dados estão no código por meio de comentários.

#### Componentes do Sistema Distribuído

O sistema consiste em 1 processo que funciona como balanceador de carga, N processos que são os clientes que fazem as requisições de escrita e leitura e M processos que atuam como réplicas do banco de dados.

O sistema aceita qualquer número de processos igual ou acima de 6, mas a proporção de cada componente é fixa, sendo que N e M tem uma razão 3:2. Portanto, se um processo tem 3 clientes, ele terá 2 réplicas, se tiver 6 clientes, terá 4 réplicas e assim por diante. Sempre haverá apenas 1 balanceador de carga.

Essa proporção foi escolhida para garantir que o sistema tenha um número mínimo de réplicas e clientes e que isso seja suficiente para testar a escalabilidade do sistema e seu caráter distribuído.

#### O Banco de Dados

Para a implementação deste trabalho foi criado um banco de dados em memória que armazena pares chave-valor. A estrutura de dados utilizada foi uma tabela hash encadeada. A tabela hash é um vetor de listas encadeadas com 100 posições, onde cada posição guarda uma lista de pares chave-valor que possuem a mesma chave.

A função hash utilizada é um shift-and-add que utiliza o valor ASCII de cada caractere da chave para calcular o índice da hash table onde o par chave-valor será armazenado.

Todas as réplicas começam vazias e mantém sua própria tabela hash e sua cópia dos dados.

#### O Balanceador de Carga

Conforme especificado no enunciado do trabalho, foi implementado um balanceador de carga para as requisições dos clientes. Independentemente do número de processos, só existe 1 load balancer que é responsável por distribuir as requisições dos clientes entre as réplicas do banco de dados.

Antes de começar a escutar as requisições, o load balancer calcula a quantidade e os ranks das réplicas e dos clientes, guardando-os em vetores que serão usados durante sua execução.

O load balancer entra em um loop infinito e usa o `MPI_Recv` (bloqueante) para esperar por requisições dos clientes. Aqui ele escuta requisições de qualquer fonte e com qualquer tag. Quando recebe uma mensagem, ele analisa a TAG para checar se é uma requisição de leitura ou de escrita.

No caso de leitura, ele escolhe uma réplica usando round-robin e envia a requisição para ela.
No caso de escrita, é usado um laço de repetição para difundir a requisição para todas as réplicas.
Se a mensagem não for nem para leitura nem para escrita, o programa checa se é uma mensagem de finalização. Se for, ele incrementa o contador de clientes finalizados. Quando o contador de clientes finalizados for igual ao número de clientes, o load balancer difunde uma mensagem para todas as réplicas indicando que o programa pode ser encerrado.

#### As Réplicas

As réplicas são responsáveis por manter uma cópia do banco e atender as requisições de escrita e leitura recebidas do load balancer. Existem M réplicas, onde M é um número inteiro maior ou igual a 2 (considerando o mínimo de 6 processos). O número de réplicas é calculado seguindo uma razão 2:3 em relação ao número de clientes.

Cada réplica inicializa seu próprio banco de dados chave-valor e depois entra em um loop infinito que fica escutando as mensagens do load balancer, que podem ter qualquer TAG. O MPI_Recv é bloqueante, então a réplica fica esperando até que uma mensagem seja recebida. Quando uma mensagem é recebida pela réplica o programa checa o tipo da mensagem dentro do conteúdo recebido e verifica se é uma operação de escrita ou leitura.

No caso de operações de leitura o sistema recebe uma mensagem que contém a chave a ser buscada no banco de dados. A réplica tenta encontrar a chave na tabela hash e o valor associado a ela. Caso o valor para aquela chave não seja encontrado, a réplica usará `NULL` como resposta. Com o valor em mãos, a réplica cria uma nova mensagem usando o construtor `new_reply_message` e envia a resposta diretamente para o cliente que fez a requisição, cujo rank está também contido no conteúdo da mensagem.

No caso de operações de escrita todas as réplicas recebem a mesma mensagem que contém a chave e o valor a serem escritos. Cada réplica tenta inserir o par chave-valor na sua tabela hash. Se a chave já existir, o valor é atualizado. Após a inserção ou edição a réplica volta a escutar por novas mensagens, sem enviar resposta para o load balancer ou para o cliente.

Caso a mensagem recebida não seja nem de leitura nem de escrita, a réplica checa se é uma mensagem de encerramento. Se for, a réplica sai do laço de repetição e encerra sua execução.

#### Os Clientes

Os clientes são responsáveis por enviar as requisições de leitura e escrita. Existem N clientes, onde N é um número inteiro maior ou igual a 3 (considerando o mínimo de 6 processos). O número de clientes é calculado seguindo uma razão 3:2 em relação ao número de réplicas.

##### Arquivo de Operações

Clientes podem enviar mensagens de leitura ou de escrita. As operações que o cliente irá fazer são determinadas por um arquivo de texto que contém as requisições linha por linha. O arquivo deve respeitar o padrão `<OPERACAO> <chave> <valor>`, onde `<OPERACAO>` é uma string que pode ser `READ` ou `WRITE`, `<chave>` é uma string de tamanho máximo 100 que representa a chave a ser buscada ou escrita e `<valor>` é uma string de tamanho máximo 100 que representa o valor a ser escrito em caso de operações do tipo `WRITE`.

Dentro do arquivo `utils.c` existe uma função `parse_operations` que recebe o caminho para o arquivo, lê as operações e as armazena em um vetor de operações. Cada operação é representada por uma estrutura de dados `operation_t` que contém o tipo de operação, a chave e o valor. Existem três arquivos pré-criados dentro do diretório `client_operations`, com 3 conjuntos de operações diferentes que podem ser usados para testar o sistema. No arquivo `main.c` na determinação do papel do processo de acordo com seu rank para cada cliente um dos arquivos é escolhido usando round-robin, de maneira que cada cliente tenha um arquivo diferente mas que todos os clientes tenham um arquivo, mesmo que haja mais clientes do que arquivos. Ainda na `main.c`, a função `parse_operations` é chamada para ler as operações do arquivo escolhido e a função do cliente é chamada já com o vetor de operações pronto. Para adicionar mais arquivos de operações, basta criar um arquivo de texto com as operações respeitando o formato especificado e nomeá-lo `operationsN.txt`, onde N é um número inteiro sequencial.

##### Funcionamento

Com o vetor de operações já em mãos cada cliente conta quantas operações são de leitura e guarda esse valor em um contador. Isso será útil para determinar se o cliente já pode encerrar sua execução. O cliente tem duas funções principais, enviar requisições e receber respostas. Como são fluxos de dados independentes, resolvi utilizar o padrão Fork Join para criar duas threads, uma para enviar requisições e outra para receber respostas.

###### Enviando Requisições

Na thread de envio de requisições existe um laço de repetição baseado na contagem de operações lidas do arquivo. Para cada operação o cliente determina se é uma mensagem de escrita ou leitura. No caso de mensagens de leitura uma mensagem é criada usando o construtor `new_read_message` passando como parâmetro o rank do cliente e a chave a ser lida. O construtor retorna uma mensagem do tipo `message_t`, compatível com o tipo de dado MPI customizado `MPI_DB_MESSAGE_TYPE` que foi criado no início do processo. Com a mensagem pronta, o cliente envia a mensagem para o load balancer usando `MPI_Send` (bloqueante).

No caso de mensagens de escrita, o cliente cria uma mensagem usando o construtor `new_write_message` passando como parâmetro o rank do cliente, a chave e o valor a serem escritos. O construtor retorna uma mensagem do tipo `message_t` que é enviada para o load balancer usando `MPI_Send` (bloqueante).

Conforme especificado no enunciado do trabalho, após o envio da mensagem o cliente espera entre 1 a 2 segundos antes de prosseguir com o laço de repetição.

###### Recebendo Respostas

Na thread de recebimento de respostas o cliente inicializa um contador de respostas recebidas e entra em um laço `while` que testa se o contador de respostas recebidas é menor que o número de operações de leitura calculado inicialmente. Enquando ainda houverem requisições de leitura sem resposta, o laço de repetição continua.

Dentro do laço existe um `MPI_Recv` (bloqueante) que espera por uma mensagem de TAG `REPLY_MESSAGE_TAG` que contém a resposta da requisição de leitura. Qualquer outra possível mensagem enviada ao cliente com tags diferentes da tag de resposta serão ignoradas. O cliente recebe a mensagem, imprime a resposta e incrementa o contador de respostas recebidas.

###### Encerrando a Execução

Quando todas as requisições do vetor de operações foram enviadas a thread de envio de requisições se encerra e fica no `pthread_join` esperando a thread de recebimento de respostas terminar. Quando o contador de respostas recebidas for igual ao número de operações de leitura, a thread de recebimento se encerra e também vai para o `pthread_join`.

Com as duas threads encerradas, o cliente cria uma última mensagem usando o construtor `new_terminate_message` e envia a mensagem para o load balancer usando `MPI_Send`. O cliente então encerra sua execução. Quando o load balancer receber a mensagem de encerramento de todos os clientes, ele difunde uma mensagem para todas as réplicas indicando que o programa pode ser encerrado e também encerra sua execução.

#### Exemplos de Execução

#### Conclusão

O trabalho demonstra uma implementação simples para um sistema distribuído que oferece escalabilidade horizontal e alta disponibilidade através da replicação de dados e do balanceamento de carga. O programa funciona com qualquer número de processos sem precisar de nenhuma alteração, e é simples alterar ou adicionar novas operações que serão feitas pelos clientes. O sistema também é capaz de encerrar sua execução de maneira segura e ordenada, garantindo que todos os processos terminem corretamente.

A principal falha do sistema é que todo seu funcionamento depende de um único processo, o balanceador de carga, que se falhar pode comprometer todo o sistema. Uma melhoria possível seria aumentar o número de load balancers ou implementar um sistema de eleição para escolher um novo balanceador de carga caso o atual falhe. Outra melhoria importante seria adicionar um mecanismo de confirmação de escrita e consistência eventual para garantir que todas as réplicas tenham coerência de dados e que operações que falharam possam ser recuperadas.

Durante o desenvolvimento do trabalho foi possível aprender mais sobre programação paralela e distribuída, MPI, implementação de sistemas distribuídos e padrões de projeto para esse tipo de sistema. Por isso concluo que o trabalho foi uma oportunidade para entender todos estes conceitos de maneira prática e por isso o objetivo do trabalho foi alcançado.

```

```

```

```
