# ReflectancePlate

## O que esta lib faz
Detecta cor do ladrilho usando placa de refletancia com filtro EMA e regras configuraveis.

## Canais
- C9: canal principal de decisao (normalmente indice 0).
- AUX: canal auxiliar para combinacao (normalmente indice 3).

## Modos de comparacao
- `ABSOLUTE`: compara faixa de C9 e AUX diretamente.
- `RATIO`: compara `C9/AUX` (mais imune a luz ambiente).

## Fluxo recomendado
1. `begin()`
2. `setDecisionChannels(...)`
3. `setMatchMode(...)`
4. `setEmaAlpha(...)`
5. carregar thresholds (`ColorCalibration::load`) ou setar manualmente
6. chamar `read()` continuamente
7. consumir `detect()`

## API detalhada
- `ReflectancePlate(const uint8_t* sensorPins, uint8_t sensorCount)`
Recebe vetor de pinos analogicos.

- `void begin(uint8_t adcBits = 12)`
Inicializa ADC e pinos.

- `void read(uint8_t numReadings = 1, uint16_t interSampleDelayUs = 200)`
Atualiza EMA dos canais.

- `void setEmaAlpha(float alpha)`
Ajusta suavizacao (`0.01..1.0`).

- `uint16_t value(uint8_t idx) const`, `const uint16_t* values() const`, `uint16_t c9() const`
Acesso as leituras filtradas.

- `setDecisionChannels`, `setMatchMode`, `setRatioThresholdScale`
Config de decisao.

- `setRedThreshold`, `setBlueThreshold`, `setSilverThreshold`, `setBlackThreshold`
Configura janelas por cor.

- `void setTiltFn(float (*fn)())`
Anexa funcao de inclinacao (ex: BNO) para bloquear deteccao em rampa.

- `FloorColor detect() const`
Retorna `UNKNOWN|RED|BLUE|SILVER|BLACK`.

- `const char* toString(FloorColor)`
Conversao para texto.

## Integracao no projeto
- `ColorCalibration`: calibra thresholds reais da pista.
- `RobotControl`: aplica regras de `BLUE`, `BLACK`, `SILVER`, `RED`.

## Exemplo
Veja `lib/ReflectancePlate/examples/basic_usage.cpp`.
