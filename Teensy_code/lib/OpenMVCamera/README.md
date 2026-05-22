# OpenMVCamera

## O que esta lib faz
Le via I2C os dados da OpenMV: `vitima` e `confianca`.

## Protocolo esperado
A camera responde 2 bytes:
- byte 0: codigo da vitima
- byte 1: confianca (0..100 ou escala definida pelo firmware da camera)

## API detalhada
- `OpenMVCamera(uint8_t i2c_address)`
Usa `Wire` padrao.

- `OpenMVCamera(TwoWire& bus, uint8_t i2c_address)`
Permite barramento alternativo.

- `bool begin(uint32_t i2c_clock_hz = 400000)`
Inicializa barramento e testa `ping()`.

- `CameraData ler()`
API legada compatvel: tentativa unica com timeout curto.

- `bool read(CameraData& out, uint32_t timeout_ms = 20, uint8_t retries = 1)`
Leitura robusta com timeout e retentativas.

- `bool ping()`
Verifica ACK do dispositivo.

- `uint8_t address() const` / `void setAddress(uint8_t)`
Consulta/atualiza endereco.

## Integracao no projeto
- `RobotControl`: salva stream de vitima no `Cell` atual.
- `ServoKit`: converte tipo de vitima em quantidade de kits.
- `LedStrip`: feedback de deteccao.

## Exemplo
Veja `lib/OpenMVCamera/examples/basic_usage.cpp`.
