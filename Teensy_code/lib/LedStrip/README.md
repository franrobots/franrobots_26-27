# LedStrip

## O que esta lib faz
Fornece sinalizacao visual de estado/eventos do robo via NeoPixel.

## Uso no Rescue Maze
- vitima detectada;
- pause/manual intervention;
- status de cor de tile;
- feedback de erro.

## Conceitos
- `LedColor`: GREEN, YELLOW, RED, BLUE, WHITE, OFF.
- `LedSide`: LEFT, RIGHT, ALL.
- Blink nao bloqueante: `blink(...)` agenda, `update()` executa.

## API detalhada
- `LedStrip(uint8_t pin, uint16_t nLeds)`
Cria strip.

- `void begin()`
Inicializa hardware e apaga todos os LEDs.

- `void setColor(LedSide side, LedColor color)`
Define cor imediata para lado selecionado.

- `void clear()`
Apaga strip.

- `void blink(LedSide side, LedColor color, uint8_t repetitions, uint16_t intervalMs = 400)`
Programa pisca com repeticoes e intervalo.

- `void update()`
Deve ser chamado continuamente no loop para executar o pisca.

## Integracao no projeto
- `OpenMVCamera + ServoKit`: sinalizar deteccao antes do drop.
- `RobotControl`: sinalizar estados de missao.

## Exemplo
Veja `lib/LedStrip/examples/basic_usage.cpp`.
