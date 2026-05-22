# Encoder

## O que esta lib faz
Conta pulsos de encoder por interrupcao para odometria por tile.
No projeto atual, dois encoders (FL e RR) sao usados para distancia media.

## Caracteristicas
- Leitura por ISR em borda de subida.
- Metodos atomicos para `read`, `reset` e `readAndReset`.
- Suporte simples para duas instancias simultaneas.

## API detalhada
- `Encoder(uint8_t pin)`
Define pino de entrada do encoder.

- `void begin(bool pullup = true)`
Configura pino e anexa ISR.

- `void reset()`
Zera contador de ticks com secao critica.

- `int32_t read() const`
Le contador atual sem reset.

- `int32_t readAndReset()`
Le e zera em operacao atomica.

## Boas praticas
- Nao chamar `begin()` mais de duas vezes (limite de instancias ISR desta implementacao).
- Calibrar `ticksPerTile` em pista real.
- Fazer media de dois encoders para reduzir erro por derrapagem.

## Integracao no projeto
- `RobotControl::traveledTicks(...)` usa FL e RR para fechar 28 cm.

## Exemplo
Veja `lib/Encoder/examples/basic_usage.cpp`.
