# RobotControl

## O que esta lib faz
E o cerebro de navegacao do robo.
Combina regras do Rescue Maze com planejamento hibrido DFS/BFS e controle por fases.

## Principais responsabilidades
- manter mapa interno (`MazeMap` + `Cell`);
- atualizar estado por cor de tile;
- escolher proximo movimento (DFS para explorar, BFS para reconectar/retornar);
- executar FSM de movimento por tile;
- registrar checkpoints, vitimas, no-reentry de azul e bloqueio de preto.

## Estruturas importantes
- `TileType`: `UNKNOWN|RED|BLUE|SILVER|BLACK`
- `MissionState`: `NAVIGATING|WAITING_BLUE|ESCAPING_BLACK`
- `MotionPhase`: `DECIDE|PRE_ALIGN|TURN_90|DRIVE_TILE`
- `PlannerInput`: sensores e tempo
- `PlannerOutput`: comando de movimento e estado

## Logica de exploracao
- DFS: tenta vizinho nao visitado na prioridade relativa `left -> front -> right -> back`.
- BFS: usado para voltar a pontos da pilha DFS quando nao ha vizinho novo.
- Retorno ao inicio: so ativa se `isExplorationComplete() == true`.

## Regras de tile
- `BLACK`: marca bloqueado e gera `RETREAT_ONE`.
- `BLUE`: entra em `WAITING_BLUE` por 5s e depois marca `noReentry`.
- `SILVER`: salva checkpoint atual.
- `RED`: marca `danger` no mapa.

## API detalhada
- `begin(startX, startY, startHeading)`
Inicializa pose e estado.

- `setTicksPerTile(int32_t)`
Define ticks para 1 tile.

- `PlannerOutput update(const PlannerInput& in)`
Passo principal do planner/FSM.

- `bool relocateToLastCheckpoint()`
Teleporta estado logico para ultimo checkpoint conhecido.

- `bool isExplorationComplete() const`
Retorna se ainda existe fronteira exploravel.

- `pose()`, `state()`, `phase()`, `map()`
Acesso a estado interno.

- `toString(...)`
Conversao de enums para debug/log.

## Integracao no projeto
- Entrada: ToF, BNO055, encoders, refletancia, OpenMV.
- Saida: `linearPwm`, `turnPwm`, `MotionCommand`.
- Atuacao: `Robot`, `LedStrip`, `ServoKit`.

## Exemplo
Veja `lib/RobotControl/examples/basic_usage.cpp`.
