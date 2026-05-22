# Robo Rescue Maze 2026 - Explicacao Completa para Ensino

Este documento explica, em linguagem didatica, como o robo funciona hoje, como as bibliotecas se conectam e como apresentar isso para alunos iniciantes.

---

## 1. Visao geral do sistema

O software do robo foi organizado em modulos (bibliotecas) para separar responsabilidades:

- Sensores de distancia (ToF)
- Sensor de orientacao (IMU/BNO055)
- Encoders de roda
- Sensor de cor do piso (ReflectancePlate)
- Cameras OpenMV (vitimas)
- Atuacao de motores
- Atuacao do servo para kits
- Logica de navegacao (cerebro)
- Calibracoes (ToF e cor)
- Tratamento de obstaculo por switch fisico

A ideia principal para os alunos:

1. **Ler sensores**
2. **Entender o ambiente**
3. **Tomar decisao**
4. **Executar movimento**
5. **Atualizar mapa**
6. **Repetir**

---

## 2. Arquivo principal (`src/main.cpp`)

O `main.cpp` eh o orquestrador. Ele conecta todas as libs.

### 2.1 O que acontece no `setup()`

1. Inicializa serial.
2. Inicializa ToF (`VL53Mux12_FRAN`).
3. Inicializa sensor de cor (`ReflectancePlate`) e seus thresholds.
4. Inicializa encoders, base de motores, LED strip, servo kit e switches.
5. Inicializa BNO055.
6. Inicializa OpenMV esquerda e direita.
7. Se botao pressionado no boot: entra em menu de calibracao (ToF/cor/ambos).
8. Carrega calibracoes salvas da EEPROM.
9. Inicializa o `RobotControl` (pose inicial e config de ticks por ladrilho).

### 2.2 O que acontece no `loop()`

Ordem simplificada:

1. Trata botao de pausa/checkpoint.
2. Trata switch de obstaculo (evasao imediata).
3. Atualiza ToFs e gera snapshot.
4. Le cor do piso.
5. Le cameras OpenMV.
6. Monta `PlannerInput`.
7. Chama `controller.update(in)` para obter `PlannerOutput`.
8. Verifica vitima confirmada (confianca + parede lateral).
9. Se vitima: para, sinaliza LED, solta kits.
10. Se nao vitima: aplica comando de movimento do `RobotControl`.
11. Imprime telemetria no serial.

---

## 3. Como a navegacao funciona (core)

Biblioteca: `lib/RobotControl`

### 3.1 Conceitos principais

- **Mapa em grade** (`MazeMap`): cada celula guarda informacoes (visitado, bloqueado, checkpoint, etc.).
- **Pose discreta**: `(x, y, heading)` do robo no grid.
- **FSM de movimento**:
  - `DECIDE`: escolhe para onde ir.
  - `PRE_ALIGN`: pre-alinha antes do movimento.
  - `TURN_90`: fecha o angulo da curva com IMU.
  - `DRIVE_TILE`: anda 1 ladrilho usando encoder + correcao.

### 3.2 Estrategia de exploracao

- Usa hibrido **DFS + BFS**:
  - DFS para explorar regioes novas.
  - BFS para retornar/reposicionar por caminhos conhecidos.

### 3.3 Regras de ladrilho

- `BLUE`: para 5s e marca como nao reentravel.
- `BLACK`: recua e marca bloqueado.
- `SILVER`: salva checkpoint.
- `RED`: marca area de risco (navegavel).

### 3.4 Retorno ao inicio

Ponto importante:

- O retorno automatico para origem so inicia quando `isExplorationComplete()` = verdadeiro.
- Ou seja: o robo **nao abandona exploracao cedo**.

---

## 4. Papel de cada biblioteca

## 4.1 `VL53Mux12_FRAN`
- Le 12 sensores VL53L0X via 2 mux TCA.
- Faz leitura round-robin.
- Aplica EMA e offsets de calibracao.

## 4.2 `Scan360`
- Converte as 12 distancias em metricas uteis:
  - `minFront`, `minLeft`, `minRight`
  - `corridorError`, `frontSkew`, etc.

## 4.3 `BNO055_FranRobots`
- Fornece yaw para:
  - manter reta
  - fazer curva de 90 graus com precisao

## 4.4 `Encoder`
- Conta pulsos das rodas.
- Mede deslocamento de 1 ladrilho (centro a centro).

## 4.5 `ReflectancePlate`
- Le sensores de refletancia com EMA.
- Usa C9 como principal.
- Modo `RATIO` (`C9/aux`) para maior imunidade a luz ambiente.
- Classifica cor do piso.

## 4.6 `OpenMVCamera`
- Le `vitima` e `confianca` via I2C.
- Uma instancia para cada lado do robo.

## 4.7 `ColorCalibration`
- Rotina guiada para calibrar cor e salvar na EEPROM.

## 4.8 `ToFCalibration`
- Rotina guiada para calibrar offsets dos ToFs e salvar na EEPROM.

## 4.9 `Motor`
- Controle de um motor individual (PWM + direcao + deadband + ganho).

## 4.10 `Robot`
- Controla os 4 motores como base diferencial.
- Recebe `linear` e `turn`.

## 4.11 `ServoKit`
- Encapsula logica de servo para drop de kits:
  - por lado (esquerda/direita)
  - por tipo de vitima (0/1/2 kits)

## 4.12 `LedStrip`
- Feedback visual de estados:
  - pausa
  - vitima detectada
  - etc.

## 4.13 `SwitchAvoidance`
- Le switches fisicos com debounce.
- Gera evento de toque esquerdo/direito/ambos.
- Permite manobra de evasao imediata.

---

## 5. Logica de vitima (seguranca contra falso positivo)

O robo so solta kits quando:

1. OpenMV detecta tipo de vitima com confianca minima.
2. Existe parede lateral confirmada por ToF:
   - esquerda usa `LC`
   - direita usa `RC`

Se confirmar:

- Para o robo.
- Acende LED no lado.
- Executa `ServoKit.dropForVictim(...)`.
- Limpa LED e aplica cooldown.

---

## 6. Botao de round (checkpoint)

Durante o round:

- Apertar botao pausa o robo.
- Robo tenta reposicionar no ultimo checkpoint (`SILVER`) salvo.
- Soltar botao retoma navegacao.
- Mapa ja construido eh mantido.

---

## 7. Fluxo de dados entre modulos (resumo)

Sensores -> `main.cpp` -> `PlannerInput` -> `RobotControl.update` -> `PlannerOutput` -> Atuadores (`Robot`, `LedStrip`, `ServoKit`)

Mais detalhado:

1. ToF/IMU/Encoder/Cor/OpenMV geram dados.
2. `main.cpp` empacota em `PlannerInput`.
3. `RobotControl` decide comando.
4. `main.cpp` aplica:
   - movimento da base
   - comportamento de vitima
   - obstaculo por switch
   - pausa/checkpoint

---

## 8. Roteiro de aula para iniciantes

Sugestao em 6 aulas:

1. **Arquitetura por modulos**
Explicar por que separar em libs.

2. **Leitura de sensores**
ToF, IMU, encoder, refletancia.

3. **Controle de movimento**
Conceito de `linear + turn`, PWM e feedback.

4. **Mapa e estados**
Grid, pose, FSM, regras de tile.

5. **DFS+BFS em robotica**
Explorar vs voltar por caminho conhecido.

6. **Eventos de competicao**
Vitima, kit, switch, pausa/checkpoint, calibracoes.

---

## 9. Pontos de calibracao obrigatorios antes da pista

1. `ENCODER_TICKS_PER_TILE`
2. Thresholds de cor (via `ColorCalibration`)
3. Offsets ToF (via `ToFCalibration`)
4. Angulos/tempos do `ServoKit`
5. Limiar de parede para vitima (`VICTIM_WALL_CONFIRM_MM`)
6. Parametros de manobra do switch (`SWITCH_*`)

---

## 10. Dicas para explicar para alunos

- Evite comecar por todos os arquivos de uma vez.
- Mostre primeiro o ciclo: sensor -> decisao -> atuacao.
- Use serial monitor para ensinar telemetria real.
- Explique que DFS/BFS decide *onde ir* e FSM decide *como ir*.
- Mostre que calibracao eh parte do software, nao so "ajuste manual".

---

## 11. Referencias internas do projeto

- `src/main.cpp`
- `docs/INDEX.md`
- READMEs de cada biblioteca em `lib/*/README.md`

