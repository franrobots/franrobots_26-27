# BNO055_FranRobots

## O que esta lib faz
Esta biblioteca encapsula o IMU BNO055 para fornecer yaw estavel para navegacao por ladrilhos.
No robo Rescue Maze, ela e usada para:
- manter reta durante deslocamento de 1 tile;
- fechar curvas de 90 graus;
- zerar referencia angular no inicio do round.

## Dependencias
- `Wire`
- `Adafruit_BNO055`
- `utility/imumaths`

## Fluxo recomendado
1. Criar objeto com endereco I2C correto.
2. Chamar `begin()` no `setup()`.
3. Chamar `zeroYaw()` na pose inicial.
4. Usar `getYaw360()` no controle fechado.

## API detalhada
- `BNO055_FranRobots(uint8_t address = 0x29)`
Configura endereco do sensor.

- `bool begin(TwoWire& wire = Wire, uint32_t i2cClock = 400000)`
Inicializa barramento e sensor. Retorna `false` se o IMU nao responder.

- `float getYaw()`
Retorna yaw em faixa `-180..+180`. Bom para erro angular minimo.

- `float getYaw360()`
Retorna yaw em faixa `0..360`. Bom para debug e monitoramento.

- `void zeroYaw()`
Define o heading atual como zero logico.

## Boas praticas
- Rezerar yaw em superficie plana antes de iniciar.
- Nao usar curva por tempo. Sempre fechar curva por erro angular.
- Se houver drift, verificar alimentacao e fixacao mecanica do IMU.

## Integracao no projeto
- `RobotControl`: usa yaw para `PRE_ALIGN`, `TURN_90` e `DRIVE_TILE`.
- `main.cpp`: converte erro angular em `turnPwm`.

## Exemplo
Veja `lib/BNO055_FranRobots/examples/basic_usage.cpp`.
