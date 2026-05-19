# SwitchAvoidance

## O que esta lib faz
Le dois switches de toque com debounce e gera evento de borda para desvio de obstaculo.

## Comportamento
- Considera estado pressionado por maioria em 3 leituras.
- Debounce simples com delay curto.
- Retorna evento apenas na transicao de inativo->ativo.

## API detalhada
- `SwitchAvoidance(uint8_t leftPin, uint8_t rightPin)`
Define pinos dos bumpers.

- `void begin(bool pullup = true)`
Configura entrada com ou sem pullup.

- `SwitchEvent poll()`
Retorna `NONE|LEFT|RIGHT|BOTH`.

- `leftPressed()` / `rightPressed()`
Retorna estado estavel atual.

## Integracao no projeto
- usado durante `DRIVE_TILE` para micro manobras de escape sem quebrar mapa.
- pode pausar linear, aplicar recuo curto e corrigir heading.

## Exemplo
Veja `lib/SwitchAvoidance/examples/basic_usage.cpp`.
