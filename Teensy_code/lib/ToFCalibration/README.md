# ToFCalibration

## O que esta lib faz
Realiza calibracao de offset dos 12 ToFs e salva em EEPROM.

## Objetivo
Compensar variacao mecanica/instalacao para melhorar centragem e decisao de parede.

## Fluxo
1. Executar `run(tof, buttonPin)`.
2. Para cada sensor, encostar em referencia e confirmar no botao.
3. Gravar offsets na EEPROM.
4. Em todo boot, chamar `load(tof)`.

## API detalhada
- `run(VL53Mux12_FRAN& tof, uint8_t buttonPin)`
Captura leitura bruta de cada sensor e salva como offset.

- `load(VL53Mux12_FRAN& tof)`
Carrega offsets da EEPROM e aplica em `setOffset(...)`.

## Observacoes
- Esta implementacao salva blob com `magic` para validar dados.
- Execute com robo parado e alvo plano para evitar erro sistematico.

## Exemplo
Veja `lib/ToFCalibration/examples/basic_usage.cpp`.
