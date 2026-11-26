# Registro de uso de IA da equipe

O código inicial fornecido pelo docente é identificado pelo primeiro commit.
Registre abaixo o uso de IA nas alterações da equipe; se não houve, registre isso.

| Data e ferramenta | Pedido | Aceito | Rejeitado | Justificativa técnica e validação |
|---|---|---|---|---|


| 11/11/2026 — ChatGPT | Orientação para preparar o ambiente, executar a planta e compreender os arquivos JSON. | Explicações dos comandos, contrato JSON e funcionamento da planta. | Nenhuma sugestão rejeitada nesta etapa. | Validação com `make test ETAPA=00` (12 testes aprovados), execução da planta, inspeção do JSON e confirmação do comando ID 1. Nenhum código autoral implementado. |

| 13/11/2026 — ChatGPT | Auxílio no planejamento da arquitetura, políticas de controle, alarmes, classes e diagrama UML. | Sugestões de organização das classes, Strategy, Command, parâmetros e documentação. | Limites de temperatura propostos inicialmente e terminologia inadequada para a grandeza nível. | Decisões revisadas com base no CONTRATO.md e registradas no ARQUITETURA.md. A arquitetura ainda será verificada na implementação e nos testes autorais. |

### Registro de uso de IA — Etapa 01

Durante o desenvolvimento da Etapa 01, foi utilizado o ChatGPT como ferramenta de apoio para:

- Planejamento e separação das responsabilidades das classes.
- Desenvolvimento e compreensão do código C++.
- Implementação da leitura e validação de arquivos JSON.
- Tratamento de sessões, ciclos e validade temporal.
- Persistência do estado e publicação da telemetria.
- Desenvolvimento dos testes autorais.
- Identificação e correção de erros de compilação e execução.
- Atualização da documentação e dos diagramas UML.

As sugestões e os códigos gerados foram incorporados progressivamente ao projeto, acompanhados de explicações, compilações, testes automatizados e verificações manuais.

A validação resultou na aprovação dos testes oficiais e dos sete executáveis autorais C++, além da geração dos relatórios de cobertura.

A IA foi utilizada como ferramenta de apoio ao desenvolvimento e aprendizado, não substituindo a necessidade de compreender e validar a implementação.

### Registro de uso de IA — Etapa 02

O Codex apoiou a revisão do contrato e a implementação em blocos aprovados
pelo estudante. O trabalho incluiu `Atuacao`, Strategy com duas políticas,
Command, persistência e reconciliação, integração e alarmes. Ao final de cada
bloco, o Codex explicou o código e os testes para revisão do estudante.

Foram aprovados os limites de controle e alarme, o uso da atuação observada
para histerese e a persistência da reserva antes da publicação. A proposta de
alterar diretamente os arquivos de persistência por substituições de texto
foi revista a pedido do estudante. As alterações foram verificadas com
`git apply --check` e aplicadas por patch no WSL.

Em 26/11/2026, a revisão local aprovou as suítes oficiais 00, 01 e 02 e os
13 executáveis autorais C++. Foram gerados relatórios de cobertura C++ e
Python e executados cenários de integração com as duas políticas, reinícios,
falhas e alarmes. O estudante solicitou revisão do relatório antes da mudança
de `ETAPA_ATUAL`.
