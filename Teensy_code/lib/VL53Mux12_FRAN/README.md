# VL53Mux12_FRAN

## O que esta lib faz
Driver de 12 sensores VL53L0X usando dois mux TCA9548A.
Usa o mesmo fluxo do sketch de validacao do projeto:
- desliga os dois mux;
- seleciona um unico canal;
- inicializa sensor por sensor;
- le sensor por sensor;
- aplica offset e filtro EMA.

## Mapa de sensores
`ToFId`: `FL FC FR LF LC LB BL BC BR RF RC RB`.

## Pipeline de leitura
1. desliga os dois mux;
2. seleciona canal no mux correto;
3. le distancia bruta;
4. aplica offset de calibracao;
5. aplica EMA;
6. salva timestamp e flag de validade.

## API detalhada
- `VL53Mux12_FRAN(uint8_t addrA, uint8_t addrB)`
Define enderecos dos dois mux.

- `bool begin(TwoWire& wire = Wire, uint32_t i2cClock = 400000, uint16_t periodMs = 40)`
Inicializa barramento, desliga os dois mux e tenta inicializar todos os 12 sensores sem abortar no primeiro erro.

- `uint16_t readSensor(ToFId id)`
Le um sensor especifico e retorna a distancia em mm ou `0` em caso de falha.

- `void readAll()`
Le os 12 sensores em sequencia, no mesmo estilo do sketch de teste.

- `bool update()`
Atualiza 1 sensor por chamada (round-robin) usando a mesma selecao exclusiva de canal.

- `void snapshot(ScanToF12& out) const`
Copia estado completo dos 12 sensores (`mm`, `ok`, `age_ms`).

- `void getSensorOk() const`
Imprime o status de inicializacao de cada sensor.

- `setOffset(ToFId id, int16_t off)`
Define offset por sensor.

- `setEmaAlpha(float alpha)`
Define suavizacao EMA (`0.05..1.0`).

- `getRaw/getCal/get/valid`
Leitura por estagio do pipeline.

- `sensorInitOk`
Indica se o sensor foi inicializado com sucesso no `begin()`.

## Integracao no projeto
- `Scan360`: extrai features (`minFront`, `corridorError`).
- `RobotControl`: usa para decisao e alinhamento lateral.
- `ToFCalibration`: injeta offsets persistentes.

## Exemplo
Veja `lib/VL53Mux12_FRAN/examples/basic_usage.cpp`.
