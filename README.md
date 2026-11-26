# Planta, controlador e supervisório — starter de POO

Repositório-base público: `rafaelrezo/poo-planta-controle`.
Faça fork para desenvolver seu projeto. A planta de tanque é fornecida pronta;
o controlador C++ implementa aquisição, controle e alarmes. O Streamlit
ainda apresenta a tela inicial. A planta também pode ser usada isoladamente em aulas
de arquivos, JSON, validação e tratamento de erros.

Você começará executando a planta pronta, verá o arquivo criado e só depois
desenvolverá os outros programas. Execute os comandos no terminal, na pasta
que contém este README e o `Makefile`. Não é necessário usar Streamlit para
começar a gerar ou ler os dados.

```text
Planta pronta -> planta.json -> seu controlador C++ -> telemetria.json -> sua tela
     ^                                  |
     +---------- comando_planta.json ----+
```

## 1. O que já funciona

| Parte | Estado no projeto | Responsabilidade |
|---|---|---|
| `planta/simulador.py` | pronta, testada, biblioteca padrão Python | gerar leituras e aplicar comandos |
| `controlador/main.cpp` | aquisição e controle automático implementados | validar snapshots, publicar comandos e gerar telemetria com alarmes |
| `supervisor/app.py` | abre uma página inicial | supervisão a desenvolver |
| `tools/enviar_comando.py` | pronto | teste manual de atuação, sem política automática |
| `docs/CONTRATO.md` | fornecido | formato, autoria dos arquivos e regras de troca |

As políticas do tanque foram implementadas pela equipe.
O enunciado e os critérios de avaliação são distribuídos separadamente pelo docente.

## 2. Fork e requisitos

Use Python 3.12+, compilador C++17 e GNU Make (Linux, WSL ou ambiente equivalente).
Primeiro faça fork pela interface do GitHub. Substitua `SEU_USUARIO` abaixo:

```bash
git clone https://github.com/SEU_USUARIO/poo-planta-controle.git
cd poo-planta-controle
git remote -v
git switch -c projeto/00-ambiente
make test ETAPA=00
```

Mantenha somente `origin`, apontando para seu fork. Abra todas as PRs da branch
para a `main` do próprio fork. Não adicione `upstream` nem abra PR contra a base
do professor. Preserve os commits iniciais e acrescente o trabalho da equipe.

O comando compila o C++ com avisos tratados como erros e roda os testes da planta.
Resultado esperado: `OK`. A etapa 00 confirma a infraestrutura, não o projeto pronto.
Se aparecer `command not found`, confira se Python, g++ e Make estão instalados
no terminal Linux/WSL que você está usando. `git remote -v` deve mostrar somente
`origin`, com o endereço do seu fork nas linhas de leitura e escrita.

## 3. Experimentar arquivos antes de implementar o controle

### 3.1. Gerar uma primeira aquisição

Na raiz do fork, execute este comando. Ele cria a pasta `dados` automaticamente,
simula três ciclos e encerra; você não precisa interrompê-lo:

```bash
python3 -m planta.simulador --dados dados --passos 3 --intervalo 0 --seed 7
```

| Parte do comando | O que você está pedindo |
|---|---|
| `--dados dados` | salvar os arquivos na pasta `dados` |
| `--passos 3` | executar três ciclos e encerrar |
| `--intervalo 0` | executar sem esperar entre os ciclos |
| `--seed 7` | repetir os sorteios ao repetir as mesmas condições |

A última linha começa com `ciclo=3`. Agora leia os dados gerados:

```bash
python3 tools/inspecionar.py --dados dados
```

Em uma execução inicial com esses parâmetros e sem comandos de atuação, a saída é:

```text
Ciclo: 3
LT-01: 47.004 %
TT-01: 24.25 C
Comando confirmado: 0
```

`LT-01` identifica o nível e `TT-01`, a temperatura. O zero indica que nenhum
comando foi aceito nesta sessão. A semente fixa reproduz as medições; o instante
e a identificação da sessão mudam. Para ver a estrutura completa do arquivo:

```bash
python3 -m json.tool dados/planta.json
```

| Campo que você verá | Como usá-lo |
|---|---|
| `sessao` e `ciclo` | saber se é uma aquisição nova ou um arquivo já processado |
| `timestamp` | mostrar quando a aquisição ocorreu e identificar dados antigos |
| `leituras` | obter tag, valor, unidade e estado de cada medição |
| `atuadores` | saber a atuação que a planta realmente aplicou |
| `ultimo_comando` | conferir qual solicitação já foi aceita |

O arquivo é um **snapshot**, uma fotografia do estado mais recente. Cada ciclo
substitui o conteúdo anterior; três ciclos não significam três documentos no
arquivo. O histórico será uma responsabilidade do projeto, em outro armazenamento.

**Confira antes de continuar:** você consegue localizar o valor do nível e sua
unidade no JSON, e explicar por que o arquivo termina no ciclo 3?

### 3.2. Entender como seu código utilizará esses dados

O controlador C++ valida `dados/planta.json`, publica a telemetria e calcula
a atuação quando ambos os sensores estão operando. Para processar uma aquisição
recente e encerrar, após gerar um snapshot da planta:

```bash
make controlador
```

Esse alvo usa a política ON/OFF. Veja a seção da Etapa 02 para executar
o controle em laço contínuo ou selecionar a política por faixas.

### 3.3. Ver uma atuação mudar a planta

Para observar atuação, execute a planta continuamente em um terminal:

```bash
make planta
```

Em outro terminal, na mesma raiz, envie um comando manual e inspecione o arquivo
depois do próximo ciclo. Aguarde cerca de um segundo entre o envio e a inspeção:

```bash
python3 tools/enviar_comando.py --dados dados --bomba 100 --aquecedor 50
python3 tools/inspecionar.py --dados dados
```

Confirme `ultimo_comando` e as novas atuações. Sem comandos novos, a atuação
expira após 5 ciclos. Não execute o comissionador e o controlador automático
simultaneamente: o arquivo de comando deve ter um único escritor. A planta pode
ser encerrada com Ctrl+C. Cada reinício gera outra sessão e invalida comandos antigos.

**O que observar:** a ferramenta informa o id enviado. Depois, `Comando confirmado`
mostra esse id e o nível passa a subir enquanto a bomba está atuando. Se esperar
sem enviar um id novo, a planta zera os atuadores após cinco ciclos. Isso ajuda a
distinguir pedido escrito, pedido aceito e atuação expirada.

## 4. Abrir o esqueleto Streamlit

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements-dev.txt
make supervisor PYTHON=.venv/bin/python
```

A página inicial tem título, diretório e aviso de implementação incompleta.
A equipe desenvolverá a leitura de `telemetria.json`, sem acesso direto à planta.
A variável `POO_DADOS` pode escolher outro diretório para testes e demonstração.
`requirements-dev.txt` instala também as ferramentas de cobertura; a planta
isolada usa apenas a biblioteca padrão. Ao terminar de observar a página,
encerre o comando de Streamlit com Ctrl+C no terminal onde o iniciou.

## 5. Incrementos e testes cumulativos

Integre cada PR antes de abrir a branch seguinte. Parta da main atualizada e
altere `ETAPA_ATUAL` no mesmo incremento, para que a CI da main mantenha as verificações.
`make test` sem `ETAPA` lê esse arquivo; especificar `ETAPA=01`, por exemplo,
permite testar a próxima etapa antes de integrá-la.

| Branch | `ETAPA_ATUAL` | Comando local e da CI | Comportamento verificado |
|---|---|---|---|
| `projeto/00-ambiente` | `00` | `make test ETAPA=00` | planta e build inicial |
| `projeto/01-aquisicao` | `01` | `make test ETAPA=01` | anteriores + aquisição, falhas e testes autorais C++ |
| `projeto/02-controle` | `02` | `make test ETAPA=02` | anteriores + comando e confirmação da planta |
| `projeto/03-supervisor` | `03` | `make test ETAPA=03` | anteriores + tela e testes autorais Python |
| `projeto/04-entrega` | `03` | `make test ETAPA=03` | regressão completa e casos autorais |

Na etapa 03, instale `requirements-dev.txt` e use
`make test ETAPA=03 PYTHON=.venv/bin/python`; o ambiente ativo da CI já tem as
dependências. `AppTest` verifica a tela sem iniciar servidor nem navegador.

O controlador aceita `--dados DIRETORIO --uma-vez`. Esse modo realiza uma
tentativa de aquisição e encerra. O modo contínuo tenta adquirir a cada segundo.

Fluxo de uma etapa, após integrar a anterior:

```bash
git switch main
git pull --ff-only origin main
git switch -c projeto/01-aquisicao
make test ETAPA=01
```

Depois de implementar, atualizar `ETAPA_ATUAL` e validar, faça commits pequenos,
push para `origin` e abra a PR. A PR inclui saída local, link da CI do commit,
explicação técnica, UML quando mudar e registro do uso de IA.

## 6. Testes que a equipe acrescenta

Cada objeto que você criar precisa de testes que mostrem seu comportamento.
Os contratos fornecidos não substituem os testes das suas classes e políticas.
Para cada responsabilidade, escolha um caso comum, uma fronteira e uma falha
quando aplicáveis. Verifique resultado e estado preservado; não apenas que o código executa.

| Código da equipe | Onde escrever os testes | Como serão executados |
|---|---|---|
| classes C++ em `controlador/` | `tests/cpp/autorais_*.cpp`, com um `main` por arquivo | o Makefile compila e o runner executa os binários |
| objetos Python do supervisor/persistência | `tests/autorais_*.py`, usando `unittest` | o runner descobre e executa os casos |

O Makefile encontra arquivos `.cpp` dentro de `controlador/` e liga os objetos
de implementação aos testes, sem incluir o `main.cpp` do aplicativo. Classes
definidas em `.hpp` também podem ser incluídas pelos testes. Separe a lógica de
domínio da função `main`, para testá-la sem iniciar o programa completo.

Desde a etapa 01, o comando exige ao menos um executável de teste autoral C++;
na etapa 03, exige também casos autorais Python. Essa checagem confirma registro
e execução, sem provar que todas as classes foram cobertas. A matriz de testes
e a revisão do código complementam a automação.

Comece pelo [guia de testes de objetos](docs/TESTES.md), com exemplos completos,
saídas esperadas e um exercício de provocar e corrigir uma falha. Depois rode:

```bash
make test ETAPA=03 PYTHON=.venv/bin/python
make cobertura ETAPA=03 PYTHON=.venv/bin/python
```

Na primeira linha, procure os nomes dos seus testes, além dos testes fornecidos.
A segunda recompila C++ com instrumentação, repete os mesmos testes e produz:

- `build/relatorios/cpp/index.html`: linhas e ramificações do controlador C++;
- `build/relatorios/python/index.html`: linhas e ramificações do Python;
- `build/relatorios/LEIA-ME.md`: etapa e resultado da execução medida.

Abra os HTMLs localmente no navegador. Localize suas classes e métodos e encontre
os caminhos não executados. No relatório Python, separe a planta pronta dos
objetos escritos pela equipe: o percentual total da planta não demonstra que
o seu código foi testado. Na base inicial, C++ pode aparecer com 0% e Streamlit
com 0%, pois esses comportamentos ainda não foram implementados nem exercitados.

Na demonstração, rode `make test`, mostre seus casos passando e explique uma
lacuna do relatório e como investigá-la. Uma condição pode executar e produzir
resultado errado; por isso cobertura alta precisa acompanhar verificações de
resultado. Não há percentual mínimo arbitrário nem promessa de qualidade por
uma métrica isolada. A CI também gera os relatórios, disponíveis como artefato
`cobertura` da execução correspondente ao commit.

Mantenha no README da sua entrega uma matriz com **objeto/método, comportamento,
teste, caso comum/fronteira/falha e lacuna conhecida**. Preserve as regressões
anteriores ao acrescentar os casos da nova etapa.

Os testes fornecidos verificam contratos observáveis e integração básica. Eles
não provam a correção do controle, a aplicação de padrões, a qualidade da UML,
a persistência nem o entendimento da equipe. Essas evidências dependem dos
testes autorais, da inspeção do diff e da defesa. Workflows e testes são visíveis
e editáveis no fork; CI verde não substitui a revisão técnica.

## 7. Padrões e organização

O controlador concentra as decisões de controle; serialização, arquivos e tela
ficam em fronteiras próprias. Justifique os padrões a partir do problema.

| Técnica/Padrão | Melhor uso | Esforço | Entregável | Limitação |
|---|---|---|---|---|
| Strategy | trocar a política mantendo a aquisição | médio | interface e políticas testadas | exige justificar as variações |
| Command | representar atuações e seus ids | médio | solicitação verificável | não garante aplicação pela planta |
| Repository | separar histórico e apresentação | médio | persistência testável | exige contrato de consulta |
| Observer | notificar mudanças e alarmes | médio | notificações desacopladas | pode duplicar eventos se mal usado |

Comece com Strategy quando variar a política; use Command se a solicitação de
atuação precisar de identidade e rastreio. Para histórico, considere Repository.
O esqueleto não determina sua hierarquia de classes.

## 8. Diagnóstico

- Arquivo ausente: execute a planta e confirme o diretório de trabalho.
- Comando rejeitado: confira sessão, id, chaves e percentuais no contrato.
- Atuadores voltam a zero: a validade é contada em ciclos; gere comandos novos
  somente após novas aquisições e respeite o intervalo da planta.
- Dados repetidos: sessão/ciclo identificam o snapshot; não duplique o histórico.
- Biblioteca JSON C++ ausente: documente sua escolha e instalação no README e CI.

Consulte [o contrato completo](docs/CONTRATO.md) antes de alterar a comunicação.

## Etapa 01 — Aquisição e telemetria

### Dependência adicional C++

O controlador utiliza a biblioteca nlohmann/json
para leitura, validação e escrita de arquivos JSON.

Instalação no Ubuntu/WSL:

```bash
sudo apt install nlohmann-json3-dev
```

A dependência também foi incluída no GitHub Actions.

### Matriz cumulativa de testes

| Objeto/método | Comportamento verificado | Arquivo de teste | Tipo de caso | Lacuna conhecida |
|---|---|---|---|---|
| Medicao | Validação de valores, unidades e estados | autorais_medicao.cpp | Comum, fronteira e falha | Nem todas as combinações inválidas foram testadas |
| Aquisicao::lerArquivo | Leitura de uma aquisição válida | autorais_aquisicao.cpp | Comum | Diferentes combinações de dados válidos |
| Aquisicao::lerArquivo | Rejeição de entradas inválidas | autorais_aquisicao_falhas.cpp | Fronteira e falha | Nem todas as combinações de campos inválidos |
| EstadoAquisicao::avaliar e registrar | Sessões, ciclos e preservação do estado | autorais_estado_aquisicao.cpp | Comum, fronteira e falha | Alternância entre múltiplas sessões |
| EstadoAquisicao::carregar e salvar | Persistência e recuperação do estado | autorais_estado_persistencia.cpp | Comum e falha | Falha durante a substituição do arquivo |
| ValidadeTemporal::avaliar | Validade, expiração e timestamp futuro | autorais_validade_temporal.cpp | Comum, fronteira e falha | Conversão de diferentes datas válidas |
| Telemetria::publicar | Publicação, preservação dos dados e substituição do JSON | autorais_telemetria.cpp | Comum e falha | Interrupção durante a gravação |
| Atuacao::Atuacao, bomba e aquecedor | Percentuais finitos em [0, 100] e consulta | autorais_atuacao.cpp | Comum, fronteira e falha | Validação de tipos JSON pertence à aquisição |
| PoliticaControle::calcular e PoliticaOnOff::calcular | Chamada pela interface, histerese e potência parcial observada | autorais_politica_onoff.cpp | Comum e fronteira | Evolução longa do tanque fora do teste unitário |
| PoliticaFaixas::calcular | Quatro faixas, transições exatas e comparação polimórfica | autorais_politica_faixas.cpp | Comum e fronteira | Evolução longa do tanque fora do teste unitário |
| Comando::Comando e publicar | Identidade, percentuais, JSON UTF-8, substituição e limpeza do temporário | autorais_comando.cpp | Comum, fronteira e falha | Falha em flush ou fechamento, como disco cheio |
| EstadoAquisicao::reservarComando, pendente, reconciliar e ultimoComandoConfirmado | Recuperação com mesmo ID e potência, confirmação atrasada e troca de sessão | autorais_estado_comandos.cpp | Comum, fronteira e falha | Interrupção nos demais pontos de escrita |
| AvaliadorAlarmes::avaliar | Códigos, mensagens, severidades, simultaneidade e sensores em falha | autorais_avaliador_alarmes.cpp | Comum, fronteira e falha | Teste da avaliação isolada das operações de arquivo |
| Fluxo do controlador e seleção da política | Aplicação, confirmação, repetição, regressão de ciclo, troca de sessão e seleção da política | test_controlador.py e autorais_integracao_controlador.py | Comum e falha | Laço contínuo validado em integração local |
| Recuperação de comando pendente | Restaura mesmo ID e potências com arquivo ausente, inválido ou divergente, preservando o arquivo já correto | autorais_integracao_controlador.py | Comum e falha | ID pendente superado observado apenas no teste de estado |
| Fluxo com falhas e alarmes | Estado inválido, bloqueio de escrita, aquisição expirada ou futura, sensores em falha e alarmes publicados | autorais_integracao_controlador.py | Comum, fronteira e falha | Erros no fechamento de arquivos |

### Resultados históricos da Etapa 01

- 15 testes oficiais aprovados.
- 7 executáveis autorais C++ aprovados.

### Cobertura C++

| Métrica | Resultado |
|---|---:|
| Linhas | 85,9% |
| Funções | 100% |
| Ramificações | 48,5% |

A cobertura de ramificações indica que nem todos
os caminhos condicionais e de exceção foram
exercitados pelos testes.

### Reprodução dos testes

```bash
make test ETAPA=01
```

### Reprodução da cobertura

```bash
make cobertura ETAPA=01 PYTHON=.venv/bin/python
```

### Documentação da arquitetura

Consulte [docs/ARQUITETURA.md](docs/ARQUITETURA.md)
para conhecer as responsabilidades das classes,
o diagrama UML e as decisões de implementação.

## Etapa 02 — Controle e alarmes

Compile com `make build`. Em um terminal, execute a planta com `make planta`.
Em outro terminal, execute uma das opções de controle no mesmo diretório de dados.
Use um único processo controlador por diretório.

```bash
./build/controlador --dados dados --politica on-off
./build/controlador --dados dados --politica faixas
```

`on-off` é a política padrão. Use `--uma-vez` para processar um snapshot recente e
encerrar. Confira `comando_planta.json` para ver o pedido e `planta.json` para
ver `ultimo_comando` e os atuadores aplicados. O campo `alarmes` de
`telemetria.json` contém os alarmes ativos no ciclo publicado.

### Validação local

```bash
make test ETAPA=00 PYTHON=.venv/bin/python
make test ETAPA=01 PYTHON=.venv/bin/python
make test ETAPA=02 PYTHON=.venv/bin/python
make cobertura ETAPA=02 PYTHON=.venv/bin/python
```

Em 26/11/2026 passaram 12, 15 e 17 testes oficiais nas Etapas 00, 01 e 02,
respectivamente. Na rodada após acrescentar os testes de integração, cada suíte
aprovou os 13 executáveis autorais C++ e os 8 novos casos autorais Python.
O runner fornece `POO_CONTROLADOR` a esses casos. A cobertura usa o binário
instrumentado `build/cobertura/controlador`.
Em diretórios temporários, a planta e o controlador foram executados como
processos independentes nas duas políticas. Foram observados comando, confirmação,
expiração e retomada após reinícios. Cenários controlados também verificaram
falha de publicação, recuperação da reserva, JSON inválido, falha de sensor e
alarmes publicados.

| Cobertura da suíte da Etapa 02 | Linhas ou instruções | Funções | Ramificações |
|---|---:|---:|---:|
| Controlador C++ | 85,4% → 92,7% | 97,4% → 100,0% | 43,7% → 51,2% |
| `main.cpp` | 62,1% → 92,4% | 75,0% → 100,0% | 20,7% → 49,1% |
| Python | 88,4% | — | 84,4% |

A comparação usa a suíte da Etapa 02 antes e depois dos testes autorais de
integração. A cobertura Python de instruções e ramificações permaneceu estável.
O relatório combinado exibe 88%, com 95% para `planta/simulador.py` e 0% para
`supervisor/app.py`, cuja implementação pertence à Etapa 03.

O laço contínuo foi exercitado fora da medição automática. Em `main.cpp`,
a mensagem para um pedido superado por um ID maior e opções secundárias da
CLI ainda ficaram sem execução. Também permanecem lacunas de E/S, como a
falha no fechamento de arquivos.
