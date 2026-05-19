# Guia de Dominio do Codigo (Versao Estudante)

Objetivo: este guia foi feito para o estudante conseguir **entender, modificar e explicar** o robô com segurança.

Se você seguir este documento por etapas, vai dominar:
- arquitetura do projeto,
- fluxo de execução,
- integração entre sensores e atuadores,
- lógica de navegação,
- calibração e debug de competição.

---

## 1) O que este robô faz

O robô participa do Rescue Maze e precisa:
- explorar um mapa desconhecido,
- identificar tipo de ladrilho (preto, azul, prata, vermelho),
- detectar vítimas com OpenMV,
- soltar kits de resgate,
- manter mapa atualizado para decidir o próximo movimento,
- lidar com pausa de round e obstáculos físicos.

---

## 2) Mapa mental do projeto

Pense no software em 4 camadas:

1. **Percepção (sensores)**
- ToF: distância
- BNO055: orientação
- Encoder: deslocamento
- Reflectance: cor do piso
- OpenMV: vítima
- Switch: toque/obstáculo

2. **Entendimento**
- `Scan360`: extrai métricas dos ToFs
- filtros EMA: estabilizam leituras

3. **Decisão**
- `RobotControl`: FSM + DFS/BFS + regras de tile

4. **Ação**
- `Robot/Motor`: movimentação
- `ServoKit`: kits
- `LedStrip`: feedback visual

---

## 3) Fluxo real do `main.cpp`

No loop, a ordem geral é:

1. Verifica botão de pausa/checkpoint.
2. Verifica switch de obstáculo.
3. Atualiza ToF e snapshot.
4. Lê cor do piso.
5. Lê OpenMV esquerda/direita.
6. Monta `PlannerInput`.
7. Chama `RobotControl.update`.
8. Se vítima válida: para + LED + servo.
9. Caso contrário: executa comando de movimento.
10. Publica telemetria no serial.

Regra de ouro:
- `main.cpp` integra tudo.
- `RobotControl` decide o que fazer.

---

## 4) Entendendo cada biblioteca (com foco de estudo)

## 4.1 Sensores e percepção
- `VL53Mux12_FRAN`: driver de 12 ToFs (com EMA e offsets)
- `Scan360`: métricas prontas de navegação
- `BNO055_FranRobots`: yaw para reta/curva
- `Encoder`: ticks para distância por ladrilho
- `ReflectancePlate`: detecção de cor com EMA + ratio (C9/aux)
- `OpenMVCamera`: leitura de vítima por I2C
- `SwitchAvoidance`: eventos de toque debounced

## 4.2 Calibração
- `ToFCalibration`: offsets dos ToFs em EEPROM
- `ColorCalibration`: thresholds de cor em EEPROM

## 4.3 Atuação
- `Motor`: controle de um motor
- `Robot`: mixagem 4 motores (linear + turn)
- `ServoKit`: drop de kits por lado/tipo
- `LedStrip`: feedback visual

## 4.4 Navegação
- `RobotControl`: núcleo do comportamento inteligente

---

## 5) Conceitos que o estudante precisa dominar

## 5.1 EMA (filtro)
Por que usar:
- reduzir ruído sem perder dinâmica.

Onde está:
- ToF e Reflectance.

## 5.2 DFS + BFS
- DFS: explorar área nova.
- BFS: voltar/reposicionar por caminho conhecido.

## 5.3 FSM
Estados de movimento:
- `DECIDE`, `PRE_ALIGN`, `TURN_90`, `DRIVE_TILE`.

Estados de missão:
- `NAVIGATING`, `WAITING_BLUE`, `ESCAPING_BLACK`.

---

## 6) Regras de competição no código

- **BLACK**: recuo imediato e marca bloqueio.
- **BLUE**: para 5 segundos e marca não reentrada.
- **SILVER**: salva checkpoint.
- **RED**: marca área de risco.

Vítima:
- só libera kit com confiança mínima + parede lateral confirmada por ToF.

---

## 7) Checkpoints de domínio (autoavaliação)

O estudante domina o projeto quando consegue:

1. Explicar o fluxo do loop sem consultar código.
2. Dizer quem decide movimento (`RobotControl`) e quem executa (`Robot`).
3. Ajustar `ENCODER_TICKS_PER_TILE` e explicar o efeito.
4. Recalibrar ToF e cor no robô real.
5. Alterar mapeamento de kits por tipo de vítima no `ServoKit`.
6. Interpretar telemetria (`State`, `Phase`, `Nav`, `PWM`).
7. Explicar por que retorno ao início só ocorre com exploração completa.

---

## 8) Plano de estudo em 7 etapas

1. Ler `docs/RESUMO_1_PAGINA.md`.
2. Ler `docs/CONCEITOS_EMA_DFS_BFS_FSM.md`.
3. Ler `src/main.cpp` de cima para baixo.
4. Estudar `RobotControl.h/.cpp`.
5. Estudar libs de sensor (`VL53`, `Reflectance`, `OpenMV`, `BNO`, `Encoder`).
6. Estudar libs de atuação (`Motor`, `Robot`, `ServoKit`, `LedStrip`).
7. Rodar exercícios práticos abaixo.

---

## 9) Exercícios práticos (obrigatórios)

## Exercício 1 - Telemetria
- Adicione no serial o valor de `scan.minFront()`.
- Explique como isso afeta `frontFree`.

## Exercício 2 - Cor
- Troque temporariamente o modo de `RATIO` para `ABSOLUTE`.
- Compare estabilidade em iluminação diferente.

## Exercício 3 - ServoKit
- Mude mapeamento de vítima para `1->2 kits` e `2->1 kit`.
- Teste e depois retorne ao padrão.

## Exercício 4 - Switch
- Aumente `SWITCH_BACK_MS` e veja efeito na evasão.

## Exercício 5 - Navegação
- Forçar `TOF_WALL_CLEAR_MM` mais alto e observar impacto no caminho.

## Exercício 6 - Checkpoint
- Simule pausa no botão e confirme reposição no último `SILVER`.

---

## 10) Debug: sintomas e causas comuns

1. Oscila muito no piso  
Possível causa: `emaAlpha` alto demais ou thresholds ruins.

2. Não detecta vítima confiável  
Possível causa: `VICTIM_CONFIDENCE_MIN` alto ou OpenMV sem ACK.

3. Curva errada de 90  
Possível causa: IMU não zerada corretamente ou ganho de controle inadequado.

4. Distância por ladrilho errada  
Possível causa: `ENCODER_TICKS_PER_TILE` mal calibrado.

5. Falso positivo de vítima  
Possível causa: limiar de parede lateral (`VICTIM_WALL_CONFIRM_MM`) alto demais.

---

## 11) O que NÃO fazer

- Não misturar regra de competição direto dentro das libs de baixo nível.
- Não colocar números mágicos no meio do `loop`.
- Não calibrar “no olho” sem salvar EEPROM.
- Não alterar várias constantes de uma vez sem teste incremental.

---

## 12) Resultado esperado de um aluno com domínio total

Um aluno com domínio total consegue:
- explicar arquitetura em 5 minutos,
- rastrear uma decisão de movimento do sensor até o motor,
- recalibrar sistema completo para nova pista,
- ajustar estratégia sem quebrar módulos,
- evoluir o código com segurança.

---

## 13) Leitura complementar do próprio projeto

- `docs/ARQUITETURA_POR_ARQUIVO.md`
- `docs/ROBO_FUNCIONAMENTO_COMPLETO.md`
- `docs/INDEX.md`
- READMEs de cada pasta em `lib/*/README.md`

