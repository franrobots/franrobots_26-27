# Conceitos Fundamentais: EMA, DFS/BFS e FSM

Este documento explica, de forma didática e completa, os três pilares usados no software do robô:

1. **EMA** (filtro de leitura de sensores)  
2. **DFS/BFS** (estratégia de exploração e rota)  
3. **FSM** (máquina de estados do movimento)

---

## 1) EMA (Exponential Moving Average)

## 1.1 O problema que o EMA resolve
Sensores reais têm ruído. Exemplo:
- refletância muda com luz ambiente,
- ToF oscila alguns milímetros,
- leituras instantâneas podem “pular”.

Se você usar leitura bruta para decisão, o robô pode:
- classificar cor errada,
- corrigir direção em excesso,
- oscilar.

## 1.2 Fórmula do EMA

Para cada nova amostra:

`EMA_n = EMA_(n-1) + alpha * (amostra_n - EMA_(n-1))`

Onde:
- `alpha` entre `0` e `1`
- `alpha` alto: responde rápido, filtra menos
- `alpha` baixo: responde mais lento, filtra mais

No projeto:
- `ReflectancePlate` usa EMA por canal (`setEmaAlpha`)
- `VL53Mux12_FRAN` também usa EMA (`setEmaAlpha`)

## 1.3 Intuição rápida
- `alpha = 1.0` -> sem filtro (valor atual puro)
- `alpha = 0.5` -> meio termo
- `alpha = 0.2` -> mais estabilidade

## 1.4 EMA no contexto de cor
No robô, o sensor de piso usa:
- canal C9 (principal)
- canal auxiliar
- modo `RATIO` (`C9/aux`) para reduzir influência de luz ambiente
- EMA para estabilizar ambos os canais antes da classificação.

---

## 2) DFS e BFS na navegação

## 2.1 Representação do ambiente
O robô usa um mapa em grade (`MazeMap`), cada célula guarda:
- se foi visitada,
- se é bloqueada,
- se é checkpoint,
- se não pode reentrar,
- informações de vítima.

Cada célula funciona como um **nó** de grafo.

## 2.2 DFS (Depth-First Search)
Objetivo: **explorar área nova**.

Comportamento:
1. tenta vizinho não visitado (prioridade local definida),
2. avança,
3. se não houver opção, volta (backtracking).

Vantagem:
- cobre cenário desconhecido com lógica simples.

Limitação:
- pode fazer voltas longas sem otimizar caminho.

## 2.3 BFS (Breadth-First Search)
Objetivo: **achar caminho curto em mapa já conhecido**.

No projeto, BFS é usado para:
- voltar para pontos da pilha de exploração (backtracking eficiente),
- retorno ao início (quando exploração completa),
- deslocamento em regiões já mapeadas.

Vantagem:
- menor número de passos em grafo não ponderado (grade).

Limitação:
- precisa de mapa conhecido/passável.

## 2.4 Por que usar DFS + BFS juntos
Esse híbrido combina:
- DFS para descobrir,
- BFS para reposicionar com eficiência.

Resumo:
- DFS responde “onde explorar agora?”
- BFS responde “qual melhor caminho até um alvo conhecido?”

---

## 3) FSM (Finite State Machine)

## 3.1 O que é
FSM é uma máquina de estados finitos:
- o robô fica em um estado por vez,
- cada estado define comportamento,
- transições acontecem por condições.

Isso evita “if-else gigante” e torna o sistema previsível.

## 3.2 FSM de movimento no projeto

Estados principais:
- `DECIDE`  
Escolhe próximo heading (com DFS/BFS).

- `PRE_ALIGN`  
Pré-alinha com ToF lateral + yaw.

- `TURN_90`  
Fecha rotação para heading desejado.

- `DRIVE_TILE`  
Anda 1 ladrilho usando encoder, com correção de yaw/centro.

## 3.3 FSM de missão (regras)

Estados de missão:
- `NAVIGATING`
- `WAITING_BLUE` (parada de 5s em tile azul)
- `ESCAPING_BLACK` (recuo em tile preto)

## 3.4 Vantagens da FSM
- comportamento reproduzível,
- debug fácil (telemetria por estado),
- expansão organizada (novos estados sem quebrar tudo).

---

## 4) Como EMA + DFS/BFS + FSM se conectam

Fluxo real:
1. Sensores geram dados ruidosos.
2. EMA estabiliza leituras.
3. `RobotControl` usa dados filtrados para decidir.
4. DFS/BFS define direção macro.
5. FSM executa microcontrole do movimento.
6. Mapa é atualizado e ciclo se repete.

Sem EMA:
- decisões erráticas.

Sem DFS/BFS:
- exploração ineficiente ou presa.

Sem FSM:
- controle confuso e difícil de manter.

---

## 5) Pseudocódigo didático

```text
loop:
  ler sensores
  filtrar (EMA)
  atualizar mapa

  if evento especial (vitima/switch/pausa):
    tratar evento
  else:
    if fase == DECIDE:
      heading <- DFS ou BFS
    if fase == PRE_ALIGN:
      alinhar
    if fase == TURN_90:
      girar para heading
    if fase == DRIVE_TILE:
      andar 1 ladrilho

  repetir
```

---

## 6) Erros comuns de iniciantes

1. `alpha` muito baixo e achar que sensor “travou”.
2. thresholds calibrados em absoluto quando sistema está em modo `RATIO`.
3. misturar lógica de decisão com atuação no mesmo bloco gigante.
4. não registrar estado/fase no serial para debug.
5. esquecer que BFS precisa de mapa válido para funcionar bem.

---

## 7) Valores práticos para começar

- EMA cor: `alpha` entre `0.25` e `0.40`
- EMA ToF: `alpha` entre `0.20` e `0.35`
- DFS padrão + BFS para retorno/reposição
- FSM com telemetria ativa:
  - `State`
  - `Phase`
  - `Nav` (`DFS/BFS`)

---

## 8) Conclusão

No seu robô:
- **EMA** dá estabilidade de leitura,
- **DFS/BFS** dá estratégia de exploração e rota,
- **FSM** dá execução controlada do movimento.

Esse trio é o núcleo que transforma sensores em comportamento robusto de competição.
