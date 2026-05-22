# ColorCalibration

## O que esta lib faz
Executa calibracao assistida das cores dos ladrilhos e salva em EEPROM.
Ela trabalha junto com `ReflectancePlate` para ajustar limiares reais do ambiente.

## Quando usar
- Antes da competicao (setup de pista).
- Sempre que mudar iluminacao, altura da placa ou sensor.

## Fluxo de calibracao
1. Chamar `run(...)`.
2. Posicionar robo em `BLACK`, `BLUE`, `RED`, `SILVER`.
3. Confirmar cada cor pelo botao.
4. Salvar thresholds na EEPROM.
5. Em boots seguintes, usar `load(...)`.

## API detalhada
- `run(ReflectancePlate& plate, uint8_t buttonPin, uint8_t c9Idx = 0, uint8_t auxIdx = 3, bool ratioMode = true, float ratioScale = 1000.0f)`
Roda captura de amostras por cor, aplica margem de seguranca e grava blob de calibracao.

- `bool load(ReflectancePlate& plate)`
Carrega thresholds salvos e aplica no objeto `ReflectancePlate`. Retorna `false` se EEPROM invalida.

## Detalhes internos importantes
- Usa assinatura (magic) e versao para validar dados.
- Pode operar em `ABSOLUTE` ou `RATIO`.
- `RATIO` costuma ser mais robusto a variacao de luz ambiente.

## Integracao no projeto
- `ReflectancePlate`: recebe thresholds calibrados.
- `RobotControl`: consome cor detectada para regras de tile.

## Exemplo
Veja `lib/ColorCalibration/examples/basic_usage.cpp`.
