# ServoKit

## O que esta lib faz
Controla o servo de kits de resgate e a logica de quantidade por tipo de vitima.

## Conceitos
- Lado de drop: `LEFT` ou `RIGHT`.
- Mapa vitima->kits configuravel (`0`, `1`, `2`).

## API detalhada
- `bool begin(uint8_t pin, uint16_t minUs = 1000, uint16_t maxUs = 2000)`
Anexa servo e centraliza.

- `setAngles(centerDeg, leftDropDeg, rightDropDeg)`
Configura angulos mecanicos.

- `setTiming(moveDelayMs, betweenKitsMs)`
Configura tempos de movimento e intervalo entre kits.

- `setVictimKitMap(noKitType, oneKitType, twoKitType)`
Define regras de quantidade por classe de vitima.

- `kitsForVictim(victimType)`
Retorna `0|1|2`.

- `shouldDrop(victimType)`
Atalho booleano.

- `center()`
Leva servo ao centro.

- `dropKits(side, kits)`
Executa drop bruto por quantidade.

- `dropForVictim(side, victimType)`
Executa drop conforme mapa.

## Integracao no projeto
Fluxo recomendado para reduzir falso positivo:
1. camera detecta vitima;
2. validar parede LC/RC com ToF;
3. parar robo;
4. sinalizar LED;
5. `dropForVictim(...)`;
6. retomar navegacao.

## Exemplo
Veja `lib/ServoKit/examples/basic_usage.cpp`.
