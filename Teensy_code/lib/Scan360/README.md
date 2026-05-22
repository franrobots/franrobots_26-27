# Scan360

## O que esta lib faz
Transforma o snapshot bruto dos 12 ToFs em features prontas de navegacao.

## Features calculadas
- `minFront()`: menor entre FL/FC/FR.
- `minLeft()`: menor entre LF/LC/LB.
- `minRight()`: menor entre RF/RC/RB.
- `frontSkew()`: `FL - FR` (erro angular frontal).
- `leftSkew()`: `LF - LB` (erro de paralelismo lateral).
- `corridorError()`: `LC - RC` (erro de centragem).

## Como usar
1. chamar `tof.snapshot(scan.s)`
2. consumir metodos de feature
3. alimentar controle e decisao

## Integracao no projeto
- `RobotControl`: decide caminhos e corrige centragem.
- `main.cpp`: define flags `frontFree/leftFree/rightFree/backFree` por limiares.

## Exemplo
Veja `lib/Scan360/examples/basic_usage.cpp`.
