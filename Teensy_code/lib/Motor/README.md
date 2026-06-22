# Motor

## O que esta lib faz
Controla um motor DC com ponte H (2 pinos de direcao + 1 PWM).

## Recursos
- PWM com resolucao configuravel.
- Deadband para vencer atrito estatico.
- Ganho individual por motor para equalizacao.
- Funcao de correcao opcional por callback.
- `stop` com freio eletrico ou coast.

## API detalhada
- `motor(uint8_t pinAin1, uint8_t pinAin2, uint16_t pinPWM)`
Mapeia pinos do canal de motor.

- `void begin(uint8_t resolutionBits = 12, uint32_t freqHZ = 20000)`
Configura pinos e PWM.

- `uint16_t applyPWM(int16_t valuePWM)`
Aplica comando assinado (`-max..+max`) e retorna modulo final aplicado.

- `void stopMotor(bool brake = true)`
`true`: freio eletrico; `false`: coast.

- `void setDeadband(uint16_t db)`
Define PWM minimo de partida.

- `void setCorrectionFn(int (*fn)(int))`
Permite ajuste externo do comando antes da aplicacao.

- `void setMotorGain(float gain)`
Escala individual para casar velocidade entre rodas.

## Boas praticas
- Ajustar deadband por roda.
- Usar ganho para corrigir assimetria mecanica.
- Evitar saturacao constante para nao perder controle fino.

## Exemplo
Veja `lib/Motor/examples/basic_usage.cpp`.
