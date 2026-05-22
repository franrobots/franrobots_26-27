# Robo Rescue Maze 2026 - Resumo 1 Pagina

## 1) Arquitetura em blocos
- **Sensores**: ToF (12), BNO055, encoders, refletancia (C9 + auxiliares), OpenMV.
- **Cerebro**: `RobotControl` (FSM + DFS/BFS + regras de tile).
- **Atuadores**: `Robot/Motor`, `ServoKit`, `LedStrip`.
- **Suporte**: `ToFCalibration`, `ColorCalibration`, `SwitchAvoidance`.

## 2) Fluxo do loop (10 passos)
1. Lê botão de pausa/checkpoint.
2. Lê switches de obstáculo e executa evasão se necessário.
3. Atualiza ToFs (`tof.update` + `snapshot`).
4. Calcula métricas com `Scan360`.
5. Lê cor do piso (`ReflectancePlate`).
6. Lê OpenMV esquerda/direita.
7. Monta `PlannerInput`.
8. Chama `RobotControl.update`.
9. Se vítima válida: para, LED, solta kits (`ServoKit`).
10. Se não: aplica comando de movimento (`Robot.drive`).

## 3) Navegação (mapa + DFS/BFS + FSM)
- **Mapa**: grade com estado por célula (`visited`, `blocked`, `checkpoint`, etc.).
- **DFS**: exploração de áreas novas.
- **BFS**: retorno/reposicionamento por caminhos já conhecidos.
- **FSM de movimento**:
  - `DECIDE` -> escolhe direção.
  - `PRE_ALIGN` -> pré-alinha com ToF/IMU.
  - `TURN_90` -> fecha curva com BNO055.
  - `DRIVE_TILE` -> anda 1 ladrilho por encoder.

## 4) Regras de ladrilho
- `BLACK`: recua e marca bloqueado.
- `BLUE`: para 5s e marca não reentrável.
- `SILVER`: salva checkpoint.
- `RED`: área de risco (navegável).

## 5) Retorno ao início
- Só retorna automaticamente para origem quando **não há mais fronteiras** (`isExplorationComplete()`).
- Antes disso, continua explorando.

## 6) Vítimas e kits (com segurança)
- Só libera kit se:
  - confiança OpenMV >= limiar
  - tipo de vítima exige kit
  - ToF lateral confirma parede (`LC`/`RC`)
- Ação: parar -> LED lado -> `ServoKit.dropForVictim` -> cooldown.

## 7) Botão de round
- Pressionar: pausa, para base, reposiciona no último checkpoint.
- Soltar: retoma.
- Mapa construído é preservado.

## 8) Checklist de calibração (pré-round)
1. `ENCODER_TICKS_PER_TILE`
2. `ToFCalibration` (offsets)
3. `ColorCalibration` (thresholds em RATIO)
4. ângulos/tempo do `ServoKit`
5. limiar de parede para vítima
6. parâmetros de evasão do switch

## 9) Roteiro de explicação em 5 minutos
1. Sensor -> decisão -> ação.
2. Mostrar `PlannerInput` e `PlannerOutput`.
3. Explicar DFS+BFS e FSM (onde ir vs como ir).
4. Explicar regras de tile e vítima.
5. Fechar com calibração + segurança de competição.
