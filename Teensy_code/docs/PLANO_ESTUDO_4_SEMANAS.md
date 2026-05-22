# Plano de Estudo - 4 Semanas (Rescue Maze)

Este plano transforma o projeto em trilha de formação para iniciantes até nível de autonomia técnica.

Formato sugerido:
- 2 encontros por semana
- 1h30 a 2h por encontro
- teoria curta + prática guiada + desafio

---

## Semana 1 - Fundamentos e arquitetura

## Objetivo da semana
Entender a arquitetura do robô e o fluxo completo do código.

## Aula 1 - Mapa do sistema
- Conceitos:
  - sensor -> decisão -> atuação
  - papel do `main.cpp`
  - bibliotecas e responsabilidades
- Leitura guiada:
  - `docs/RESUMO_1_PAGINA.md`
  - `docs/ARQUITETURA_POR_ARQUIVO.md`
- Prática:
  - localizar no código onde cada sensor é lido
  - localizar onde `PlannerInput` é montado

## Aula 2 - Pipeline real no loop
- Conceitos:
  - eventos prioritários (botão, switch, vítima)
  - telemetria de debug
- Prática:
  - acompanhar serial e explicar `State`, `Phase`, `Nav`, `PWM`
  - exercício: adicionar uma linha de log útil

## Entregável da semana
- estudante explica o fluxo do loop sem consultar slides.

---

## Semana 2 - Sensores, filtros e calibração

## Objetivo da semana
Dominar leitura estável de sensores e calibração em campo.

## Aula 3 - EMA e refletância
- Conceitos:
  - ruído de sensor
  - EMA (`alpha`) e trade-off resposta/estabilidade
  - modo `RATIO` para imunidade à luz ambiente
- Leitura guiada:
  - `docs/CONCEITOS_EMA_DFS_BFS_FSM.md`
  - `lib/ReflectancePlate/*`
- Prática:
  - variar `setEmaAlpha`
  - comparar comportamento `ABSOLUTE` vs `RATIO`

## Aula 4 - ToF, IMU, encoder e calibração EEPROM
- Conceitos:
  - `VL53Mux12_FRAN` + `Scan360`
  - BNO055 para yaw
  - encoder para distância por ladrilho
  - `ToFCalibration` e `ColorCalibration`
- Prática:
  - executar rotina de calibração
  - validar carregamento de EEPROM no boot

## Entregável da semana
- estudante calibra ToF e cor sem ajuda.

---

## Semana 3 - Navegação e estratégia

## Objetivo da semana
Dominar decisão de caminho com FSM + DFS/BFS.

## Aula 5 - FSM de movimento
- Conceitos:
  - `DECIDE`, `PRE_ALIGN`, `TURN_90`, `DRIVE_TILE`
  - diferença entre “girar 90” e “centralizar no corredor”
- Leitura guiada:
  - `lib/RobotControl/RobotControl.h/.cpp`
- Prática:
  - acompanhar transições de fase no serial
  - identificar condição de troca de cada fase

## Aula 6 - DFS+BFS e regras de tile
- Conceitos:
  - DFS para explorar
  - BFS para reposicionar/retornar
  - regras: `BLACK`, `BLUE`, `SILVER`, `RED`
  - retorno ao início apenas com exploração completa
- Prática:
  - simular beco e explicar saída
  - simular pausa e retorno via checkpoint

## Entregável da semana
- estudante explica por que DFS+BFS foi escolhido para este robô.

---

## Semana 4 - Eventos de competição e autonomia

## Objetivo da semana
Integrar comportamento de prova com segurança.

## Aula 7 - Vítima, kits, validação e switch
- Conceitos:
  - OpenMV (`vitima`, `confianca`)
  - confirmação por parede lateral (`LC/RC`)
  - `ServoKit` e mapeamento de kits
  - `SwitchAvoidance` para choque físico
- Prática:
  - alterar `setVictimKitMap`
  - ajustar `SWITCH_*` no `config.h`

## Aula 8 - Revisão final + mini-desafio
- Desafio:
  - o robô deve explorar, reagir a obstáculos, detectar vítima e pausar/retomar no checkpoint
- Apresentação:
  - cada aluno explica uma parte do pipeline

## Entregável da semana
- demonstração funcional + explicação técnica consistente.

---

## Avaliação sugerida

Pontuação total: 100

1. Arquitetura e explicação (20)
- identifica papel de cada lib
- explica fluxo do `main.cpp`

2. Sensores e calibração (20)
- calibra corretamente
- interpreta telemetria

3. Navegação (25)
- explica FSM
- explica DFS/BFS
- justifica decisões do robô

4. Eventos de competição (20)
- vítima e kits com validação
- pausa/checkpoint
- switch e evasão

5. Qualidade de prática (15)
- organização
- segurança ao alterar parâmetros
- capacidade de debug

---

## Exercícios progressivos (para casa)

## Nível 1 (iniciante)
1. Trocar cor do LED de pausa.
2. Ajustar intervalo de pisca.
3. Explicar em texto o que é `PlannerInput`.

## Nível 2 (intermediário)
1. Ajustar `ENCODER_TICKS_PER_TILE` e justificar valor.
2. Mudar limiar de vítima e comparar falso positivo.
3. Ajustar `alpha` de EMA para refletância.

## Nível 3 (avançado)
1. Criar novo estado de missão no `RobotControl`.
2. Adicionar novo tipo de telemetria.
3. Propor melhoria de robustez sem quebrar modularidade.

---

## Critérios de “domínio total” do aluno

O aluno está pronto quando consegue:
- explicar o sistema ponta a ponta,
- depurar comportamento em pista,
- recalibrar sem suporte,
- alterar estratégia sem quebrar o projeto,
- ensinar outro colega usando os docs.

---

## Documentos de apoio (ordem recomendada)

1. `docs/RESUMO_1_PAGINA.md`
2. `docs/CONCEITOS_EMA_DFS_BFS_FSM.md`
3. `docs/ARQUITETURA_POR_ARQUIVO.md`
4. `docs/GUIA_DOMINIO_PARA_ESTUDANTE.md`
