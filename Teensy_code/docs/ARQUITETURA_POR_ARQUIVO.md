# Como o Codigo Foi Pensado (Arquivo por Arquivo)

Este documento explica **o raciocinio de engenharia** usado para montar o sistema atual do robô e detalha, **arquivo por arquivo**, qual problema cada parte resolve.

---

## 1. Ideia central de arquitetura

A construção do código seguiu 4 princípios:

1. **Separar responsabilidades**
- leitura de sensores,
- decisão de navegação,
- atuação,
- calibração.

2. **Permitir calibração em campo**
- ToF e cor com EEPROM.

3. **Garantir robustez para competição**
- validação de vítima com parede lateral,
- pausa/checkpoint,
- evasão por switches.

4. **Facilitar ensino e manutenção**
- bibliotecas pequenas e reutilizáveis,
- documentação por lib,
- exemplos por lib.

---

## 2. Fluxo global (de alto nível)

1. Sensores são lidos (`ToF`, `IMU`, `encoder`, `cor`, `OpenMV`, `switch`).
2. Dados são filtrados/normalizados (EMA, ratio de refletância).
3. `RobotControl` transforma percepção em comando.
4. `main.cpp` aplica comando nos atuadores.
5. Eventos prioritários podem interromper (vítima, pausa, obstáculo).
6. Mapa e estado são atualizados continuamente.

---

## 3. Arquivo principal

## 3.1 [`src/main.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\src\main.cpp)
**Papel**
- Orquestrador de todo o sistema.

**Por que foi feito assim**
- Evitar lógica “espalhada” em vários arquivos `.ino`.
- Deixar `setup/loop` como pipeline claro de integração.

**O que concentra**
- inicialização das libs,
- leitura periódica dos sensores,
- montagem de `PlannerInput`,
- aplicação de `PlannerOutput`,
- tratamento de eventos prioritários.

---

## 4. Configuração e parâmetros

## 4.1 [`include/config.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\include\config.h)
**Papel**
- Centralizar constantes de hardware e comportamento.

**Por que existe**
- Evitar “números mágicos” no código.
- Permitir ajuste rápido sem reescrever lógica.

**Tipos de constantes**
- pinos,
- limites de sensor,
- timings de servo,
- limiares de vítima,
- parâmetros de evasão de switch.

---

## 5. Sensores de distância e features

## 5.1 [`lib/VL53Mux12_FRAN/VL53Mux12_FRAN.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\VL53Mux12_FRAN\VL53Mux12_FRAN.h)
## 5.2 [`lib/VL53Mux12_FRAN/VL53Mux12_FRAN.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\VL53Mux12_FRAN\VL53Mux12_FRAN.cpp)
**Papel**
- Driver unificado para 12 ToFs com dois TCA9548A.

**Decisão técnica**
- leitura round-robin (1 sensor por ciclo),
- EMA embutido,
- suporte a offset por sensor.

**Benefício**
- estabilidade e escalabilidade para navegação.

## 5.3 [`lib/Scan360/Scan360.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\Scan360\Scan360.h)
## 5.4 [`lib/Scan360/Scan360.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\Scan360\Scan360.cpp)
**Papel**
- Converter 12 leituras em métricas úteis (`minFront`, `corridorError` etc.).

**Por que separar**
- evitar repetição de cálculos no `main`.

---

## 6. IMU e odometria

## 6.1 [`lib/BNO055_FranRobots/BNO055_FranRobots.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\BNO055_FranRobots\BNO055_FranRobots.h)
## 6.2 [`lib/BNO055_FranRobots/BNO055_FranRobots.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\BNO055_FranRobots\BNO055_FranRobots.cpp)
**Papel**
- Fornecer yaw para reta e curva de 90°.

## 6.3 [`lib/Encoder/Encoder.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\Encoder\Encoder.h)
## 6.4 [`lib/Encoder/Encoder.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\Encoder\Encoder.cpp)
**Papel**
- Contagem de pulsos por interrupção.

**Uso no projeto**
- medir distância de 1 ladrilho (`ticksPerTile`).

---

## 7. Cor do piso

## 7.1 [`lib/ReflectancePlate/ReflectancePlate.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\ReflectancePlate\ReflectancePlate.h)
## 7.2 [`lib/ReflectancePlate/ReflectancePlate.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib\ReflectancePlate\ReflectancePlate.cpp)
**Papel**
- Leitura de refletância com EMA e classificação de cor.

**Decisões importantes**
- C9 explícito,
- modo `RATIO` (`C9/aux`) para imunidade à luz ambiente,
- thresholds por cor.

---

## 8. Visão e vítima

## 8.1 [`lib/OpenMVCamera/OpenMVCamera.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/OpenMVCamera\OpenMVCamera.h)
## 8.2 [`lib/OpenMVCamera/OpenMVCamera.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/OpenMVCamera\OpenMVCamera.cpp)
**Papel**
- Ler código de vítima e confiança da OpenMV por I2C.

**Decisão técnica**
- API com timeout e retry para reduzir falha de comunicação.

---

## 9. Atuadores

## 9.1 [`lib/Motor/Motor.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/Motor/Motor.h)
## 9.2 [`lib/Motor/Motor.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/Motor/Motor.cpp)
**Papel**
- Controle de um motor (PWM + direção + deadband/ganho).

## 9.3 [`lib/Robot/Robot.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/Robot\Robot.h)
## 9.4 [`lib/Robot/Robot.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/Robot\Robot.cpp)
**Papel**
- Combinar 4 motores em base diferencial.

## 9.5 [`lib/LedStrip/LedStrip.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/LedStrip\LedStrip.h)
## 9.6 [`lib/LedStrip/LedStrip.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/LedStrip\LedStrip.cpp)
**Papel**
- Sinalização visual de estados/eventos.

## 9.7 [`lib/ServoKit/ServoKit.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/ServoKit\ServoKit.h)
## 9.8 [`lib/ServoKit/ServoKit.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/ServoKit\ServoKit.cpp)
**Papel**
- Encapsular lógica de drop de kits por lado e tipo de vítima.

---

## 10. Obstáculo físico por switch

## 10.1 [`lib/SwitchAvoidance/SwitchAvoidance.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/SwitchAvoidance\SwitchAvoidance.h)
## 10.2 [`lib/SwitchAvoidance/SwitchAvoidance.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/SwitchAvoidance\SwitchAvoidance.cpp)
**Papel**
- Debounce e evento por borda de toque esquerdo/direito/ambos.

**Por que foi adicionado**
- lidar com colisão real mesmo quando ToF não antecipou.

---

## 11. Navegação e mapa (cérebro)

## 11.1 [`lib/RobotControl/RobotControl.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/RobotControl\RobotControl.h)
## 11.2 [`lib/RobotControl/RobotControl.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/RobotControl\RobotControl.cpp)
**Papel**
- Planejamento de movimento por ladrilho + mapa.

**Decisões técnicas principais**
- FSM (`DECIDE`, `PRE_ALIGN`, `TURN_90`, `DRIVE_TILE`);
- exploração híbrida DFS/BFS;
- regras de tile (`BLACK`, `BLUE`, `SILVER`, `RED`);
- checkpoint de round;
- retorno ao início somente quando mapeamento completo.

---

## 12. Calibração e EEPROM

## 12.1 [`lib/ToFCalibration/ToFCalibration.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/ToFCalibration\ToFCalibration.h)
## 12.2 [`lib/ToFCalibration/ToFCalibration.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/ToFCalibration\ToFCalibration.cpp)
**Papel**
- Calibração de offset de ToFs com EEPROM.

## 12.3 [`lib/ColorCalibration/ColorCalibration.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/ColorCalibration\ColorCalibration.h)
## 12.4 [`lib/ColorCalibration/ColorCalibration.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/ColorCalibration\ColorCalibration.cpp)
**Papel**
- Calibração de cor (black/blue/red/silver) com EEPROM.

---

## 13. Multiplexador I2C

## 13.1 [`lib/TCA9548A/TCA9548A.h`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/TCA9548A\TCA9548A.h)
## 13.2 [`lib/TCA9548A/TCA9548A.cpp`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\lib/TCA9548A\TCA9548A.cpp)
**Papel**
- Selecionar canal I2C no mux.

---

## 14. Documentação de apoio

## 14.1 [`docs/INDEX.md`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\docs\INDEX.md)
Guia de estudo e ordem de integração.

## 14.2 [`docs/ROBO_FUNCIONAMENTO_COMPLETO.md`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\docs\ROBO_FUNCIONAMENTO_COMPLETO.md)
Explicação didática completa.

## 14.3 [`docs/RESUMO_1_PAGINA.md`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\docs\RESUMO_1_PAGINA.md)
Resumo para apresentação rápida.

## 14.4 [`docs/CONCEITOS_EMA_DFS_BFS_FSM.md`](c:\Users\LIE\OneDrive - SESISENAISP - Corporativo\Documentos\PlatformIO\Projects\RescueMaze2026\docs\CONCEITOS_EMA_DFS_BFS_FSM.md)
Base conceitual para ensino.

---

## 15. Como explicar isso em sala (roteiro curto)

1. Mostrar pipeline: sensor -> decisão -> atuação.  
2. Mostrar `main.cpp` como orquestrador.  
3. Mostrar `RobotControl` como cérebro.  
4. Mostrar eventos críticos: vítima, pausa/checkpoint, obstáculo.  
5. Mostrar calibração como parte obrigatória do software.  

---

## 16. Resultado prático da arquitetura

Comparado ao código antigo monolítico:

- menos acoplamento,
- melhor testabilidade,
- manutenção mais fácil,
- didática melhor para iniciantes,
- crescimento mais seguro para novas regras de competição.
