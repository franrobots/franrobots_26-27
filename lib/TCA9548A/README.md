# TCA9548A

## O que esta lib faz
Controla multiplexador I2C TCA9548A para selecionar canal ativo (0..7).
Antes de selecionar um canal, desliga todos para espelhar o fluxo de debug usado no projeto.

## Quando usar
Quando varios dispositivos I2C compartilham endereco igual ou precisam isolamento de barramento.

## API detalhada
- `TCA9548A(uint8_t addr)`
Define endereco do mux.

- `bool begin(TwoWire& wire = Wire)`
Inicializa ponteiro de barramento e desativa todos os canais.
Retorna `true` se houver ACK.

- `bool select(uint8_t ch)`
Desativa todos os canais, ativa um unico canal e retorna `true` se houve ACK.

- `bool disableAll()`
Desativa todos os canais e retorna `true` se houve ACK.

- `uint8_t addr() const`
Retorna endereco configurado.

## Integracao no projeto
- base para `VL53Mux12_FRAN` (dois mux para 12 ToFs).
- com dois mux ao mesmo tempo, ainda e necessario desativar os dois antes de escolher o sensor; `VL53Mux12_FRAN` faz isso.

## Exemplo
Veja `lib/TCA9548A/examples/basic_usage.cpp`.
