# Biblioteca do Projeto - Guia de Estudo e Integracao

Este indice organiza a leitura das libs por ordem recomendada para dominar o stack completo do robô.

## 1) Sensores base
1. `VL53Mux12_FRAN`  
Leitura dos 12 ToFs e filtro principal de distância.

2. `Scan360`  
Features de navegação derivadas dos ToFs (`minFront`, `corridorError`, etc.).

3. `BNO055_FranRobots`  
Orientação yaw para reta e curva de 90°.

4. `Encoder`  
Odometria por ticks para deslocamento por ladrilho.

5. `ReflectancePlate`  
Detecção de cor de ladrilho com EMA e modo `RATIO` para robustez à luz.

6. `OpenMVCamera`  
Detecção de vítima por I2C (`vitima`, `confianca`).

## 2) Calibração
1. `ToFCalibration`  
Offset dos ToFs em EEPROM.

2. `ColorCalibration`  
Thresholds de cor em EEPROM (BLACK/BLUE/RED/SILVER).

## 3) Atuadores
1. `Motor`  
Comando de um motor individual.

2. `Robot`  
Mixagem de quatro motores com comando `linear + turn`.

3. `LedStrip`  
Feedback visual de estados/eventos.

4. `ServoKit`  
Lógica de drop de kits por lado e por tipo de vítima.

5. `SwitchAvoidance`  
Leitura debounced dos switches de toque para desvio de obstáculo.

## 4) Navegação e estratégia
1. `RobotControl`  
Núcleo de decisão: FSM de movimento + exploração híbrida DFS/BFS + regras de tile.

## 5) Ordem recomendada de integração em firmware
1. Inicializar ToF (`VL53Mux12_FRAN`) e confirmar `Scan360`.
2. Integrar IMU (`BNO055`) + encoders para controle fechado por ladrilho.
3. Integrar `ReflectancePlate` e validar classificação de cor.
4. Integrar `OpenMVCamera` e lógica de vítima.
5. Integrar `Motor`/`Robot` para movimento real.
6. Integrar `RobotControl` como cérebro final.
7. Aplicar `ToFCalibration` + `ColorCalibration` em EEPROM.

## 6) Exemplo de navegação completa
Veja o arquivo principal do projeto:
- `src/main.cpp`

Cada lib possui:
- `README.md` com objetivo e métodos
- `examples/basic_usage.cpp` com exemplo mínimo
