# Robot

## O que esta lib faz
Abstrai a base diferencial de 4 motores (FL, RL, FR, RR).
Converte comando global (`linear`, `turn`) para cada roda.

## Modelo de controle
- `left = linear + turn`
- `right = linear - turn`

## API detalhada
- `Robot(motor& mFL, motor& mRL, motor& mFR, motor& mRR)`
Recebe referencias dos 4 motores.

- `void begin(uint8_t resolutionBits = 12, uint32_t freqHZ = 20000)`
Inicializa todos os motores.

- `void move_tank(int16_t linear, int16_t turn)`
Comando principal.

- `void setLeftRight(int16_t left, int16_t right)`
Comando direto por lado.

- `void stop(bool brake = true)`
Para todos os motores.

- `void turnLeft(int16_t speed)` / `void turnRight(int16_t speed)`
Giro no lugar.

- `invertFL/RL/FR/RR(bool)`
Inverte sentido logico de cada roda.

## Integracao no projeto
- `RobotControl` gera `linearPwm` e `turnPwm`.
- `main.cpp` chama `robot.move_tank(linear, turn)`.

## Exemplo
Veja `lib/Robot/examples/basic_usage.cpp`.
