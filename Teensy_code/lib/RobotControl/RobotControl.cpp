#include "RobotControl.h"
#include <math.h>

MazeMap::MazeMap() = default;

Cell* MazeMap::at(int16_t x, int16_t y) {
  const int16_t ix = x + ORIGIN_X;
  const int16_t iy = y + ORIGIN_Y;
  if (ix < 0 || iy < 0 || ix >= WIDTH || iy >= HEIGHT) return nullptr;
  return &_grid[iy][ix];
}

const Cell* MazeMap::at(int16_t x, int16_t y) const {
  const int16_t ix = x + ORIGIN_X;
  const int16_t iy = y + ORIGIN_Y;
  if (ix < 0 || iy < 0 || ix >= WIDTH || iy >= HEIGHT) return nullptr;
  return &_grid[iy][ix];
}

RobotControl::RobotControl() = default;

void RobotControl::begin(int16_t startX, int16_t startY, Heading startHeading) {
  _pose = {startX, startY, startHeading};
  _targetHeading = startHeading;
  // Comeca pelo ciclo inteiro: se o robo foi largado com o nariz na parede,
  // recua antes; e a primeira decisao tambem precisa de leitura de ToF
  // tirada com o robo quadrado, nao de como ele caiu na pista.
  enterCenter(millis(), false);
  _state = MissionState::NAVIGATING;
  _blueReleaseAtMs = 0;
  _dfsSize = 0;
  _bfsLen = 0;
  _bfsIdx = 0;
  _hasCheckpoint = false;
  _lastCheckpointX = startX;
  _lastCheckpointY = startY;
  _startX = startX;
  _startY = startY;
  _returnHomeActive = false;

  Cell* c = _map.at(_pose.x, _pose.y);
  if (c) c->visited = true;
}

bool RobotControl::relocateToLastCheckpoint() {
  if (!_hasCheckpoint) return false;

  _pose.x = _lastCheckpointX;
  _pose.y = _lastCheckpointY;
  enterCenter(millis(), false);
  _state = MissionState::NAVIGATING;
  _bfsLen = 0;
  _bfsIdx = 0;
  _dfsSize = 0;
  _blueReleaseAtMs = 0;
  _returnHomeActive = false;

  Cell* c = _map.at(_pose.x, _pose.y);
  if (c) c->visited = true;
  return true;
}

PlannerOutput RobotControl::update(const PlannerInput& in) {
  updateVictimStream(in);
  markCurrentTile(in.sensedTile);
  tryFinishBlueWait(in.nowMs);

  PlannerOutput out;
  out.pose = _pose;
  out.state = _state;
  out.phase = _phase;

  if (_state == MissionState::WAITING_BLUE) {
    out.command = MotionCommand::HOLD;
    return out;
  }

  if (in.sensedTile == TileType::BLACK) {
    _state = MissionState::ESCAPING_BLACK;
    out.command = MotionCommand::RETREAT_ONE;
    out.state = _state;
    out.linearPwm = -LINEAR_ALIGN_PWM;
    out.turnPwm = 0;

    Pose2D back = _pose;
    back.heading = turnBack(back.heading);
    stepForward(back);
    _pose = back;

    Cell* c = _map.at(_pose.x, _pose.y);
    if (c) c->visited = true;

    _bfsLen = 0;
    _bfsIdx = 0;
    enterCenter(in.nowMs, false);
    _state = MissionState::NAVIGATING;
    return out;
  }

  if (_phase == MotionPhase::CENTER) {
    // Fotografa frente e tras AGORA, antes de mexer na posicao. Depois de
    // avancar ou recuar essas duas leituras nao valem mais para decidir.
    if (_latchPending) {
      _frontFreeAtCenter = in.frontFree;
      _backFreeAtCenter = in.backFree;
      _latchPending = false;
    }

    _targetHeading = _pose.heading;   // posiciona mantendo o rumo

    const float yawErr = shortestYawError(headingToYaw(_targetHeading), in.yawDeg);
    out.command = MotionCommand::DRIVE_CLOSED_LOOP;
    out.turnPwm = clampPwm(yawErr * YAW_KP);
    out.phase = _phase;

    // Erro longitudinal em mm. POSITIVO = precisa AVANCAR.
    // A parede da frente e a referencia preferida; sem ela, a de tras serve
    // igual, so com o sinal trocado. Sem nenhuma das duas nao ha referencia
    // e a fase passa direto - num cruzamento aberto nao ha o que medir.
    int32_t errMm = 0;
    bool haveRef = false;

    if (in.tofFrontMm > 0 && in.tofFrontMm <= _center.refValidMm) {
      errMm = (int32_t)in.tofFrontMm - (int32_t)_center.frontTargetMm;
      haveRef = true;
    } else if (in.tofBackMm > 0 && in.tofBackMm <= _center.refValidMm) {
      errMm = (int32_t)_center.backTargetMm - (int32_t)in.tofBackMm;
      haveRef = true;
    }

    // Travas de seguranca: nunca avancar para dentro da parede da frente,
    // nunca recuar para dentro da de tras. Zerar o erro no lado travado faz
    // a fase assentar em vez de empurrar contra a parede ate o watchdog.
    if (errMm > 0 && in.tofFrontMm > 0 && in.tofFrontMm <= _center.frontMinMm) errMm = 0;
    if (errMm < 0 && in.tofBackMm  > 0 && in.tofBackMm  <= _center.backMinMm)  errMm = 0;

    if (haveRef && labs(errMm) > (int32_t)_center.tolMm) {
      _alignStable = 0;

      float pwm = _center.kp * (float)errMm;
      // Piso: o arranque linear tambem tem atrito estatico, so que menor
      // que o do giro - aqui as rodas rolam em vez de arrastar de lado.
      if (fabsf(pwm) < (float)_center.minPwm) {
        pwm = (errMm > 0) ? (float)_center.minPwm : -(float)_center.minPwm;
      }
      if (pwm >  (float)_center.maxPwm) pwm =  (float)_center.maxPwm;
      if (pwm < -(float)_center.maxPwm) pwm = -(float)_center.maxPwm;
      out.linearPwm = (int16_t)pwm;
    } else {
      out.linearPwm = 0;
      if (_alignStable < ALIGN_STABLE_CYCLES) _alignStable++;
    }

    const bool settled = !haveRef || (_alignStable >= ALIGN_STABLE_CYCLES);
    const bool timedOut = (in.nowMs - _phaseStartMs) >= _center.maxMs;
    if (!settled && !timedOut) return out;

    _alignStable = 0;
    _phase = MotionPhase::ALIGN;
    _phaseStartMs = in.nowMs;
    out.linearPwm = 0;
    out.phase = _phase;
    // cai para o ALIGN no mesmo ciclo
  }

  if (_phase == MotionPhase::ALIGN) {
    // Quadra no cardinal ATUAL - nao no proximo. Este e o ponto de a fase
    // vir antes do DECIDE: as leituras laterais de ToF que vao alimentar a
    // decisao so valem com o robo perpendicular as paredes.
    _targetHeading = _pose.heading;

    const float yawErr = shortestYawError(headingToYaw(_targetHeading), in.yawDeg);

    out.command = MotionCommand::DRIVE_CLOSED_LOOP;
    out.linearPwm = 0;   // alinhar e parado: sem avanco nenhum
    out.turnPwm = clampPwm(yawErr * YAW_KP);
    out.phase = _phase;

    if (fabsf(yawErr) <= TURN_TOL_DEG) {
      if (_alignStable < ALIGN_STABLE_CYCLES) _alignStable++;
    } else {
      _alignStable = 0;
    }

    const bool settled = (_alignStable >= ALIGN_STABLE_CYCLES);
    const bool timedOut = (in.nowMs - _phaseStartMs) >= _alignMs;
    if (!settled && !timedOut) return out;

    _alignStable = 0;
    _phaseStartMs = in.nowMs;

    if (_settleThenDrive) {
      // Vem de um giro: ja decidido, ja centrado no eixo novo, ja quadrado.
      // So falta andar. A baseline dos encoders e armada AQUI, depois do
      // CENTER - se fosse no fim do TURN, o deslocamento da centragem
      // entraria na conta do ladrilho.
      _settleThenDrive = false;
      _startFlTicks = in.encFlTicks;
      _startRrTicks = in.encRrTicks;
      _phase = MotionPhase::DRIVE_TILE;
    } else {
      _phase = MotionPhase::DECIDE;
    }
    out.phase = _phase;
    // cai para a proxima fase no mesmo ciclo
  }

  if (_phase == MotionPhase::DECIDE) {
    Heading nextHeading;
    bool haveMove = false;
    DecisionSource source = DecisionSource::NONE;

    if (_bfsIdx < _bfsLen) {
      nextHeading = _bfsPath[_bfsIdx++];
      haveMove = true;
      source = DecisionSource::BFS;
    } else if (chooseDfsNeighbor(in, nextHeading)) {
      haveMove = true;
      source = DecisionSource::DFS;
    } else {
      while (popCurrentFromDfs()) {}
      while (_dfsSize > 0 && !haveMove) {
        const Node2D target = _dfsStack[_dfsSize - 1];
        if (target.x == _pose.x && target.y == _pose.y) {
          _dfsSize--;
          continue;
        }

        Heading adj;
        if (headingFromTo(_pose.x, _pose.y, target.x, target.y, adj)) {
          nextHeading = adj;
          haveMove = true;
          source = DecisionSource::BFS;
          break;
        }

        if (buildBfsPathTo(target.x, target.y) && _bfsIdx < _bfsLen) {
          nextHeading = _bfsPath[_bfsIdx++];
          haveMove = true;
          source = DecisionSource::BFS;
          break;
        }

        _dfsSize--;
      }

      if (!haveMove && !_returnHomeActive && isExplorationComplete()) {
        if (_pose.x != _startX || _pose.y != _startY) {
          if (buildBfsPathTo(_startX, _startY) && _bfsIdx < _bfsLen) {
            nextHeading = _bfsPath[_bfsIdx++];
            haveMove = true;
            source = DecisionSource::BFS;
            _returnHomeActive = true;
          }
        }
      }
    }

    if (!haveMove) {
      out.command = MotionCommand::HOLD;
      out.source = DecisionSource::NONE;

      // Antes o DECIDE parava aqui e devolvia HOLD para sempre - no log,
      // 20+ ciclos travado sem nenhuma saida possivel. Agora ele so trava
      // de verdade quando a missao acabou: exploracao completa e de volta
      // ao ponto de partida.
      if (isExplorationComplete() && _pose.x == _startX && _pose.y == _startY) {
        return out;
      }

      // Caso contrario recomeca o ciclo e sente de novo. Limpar a memoria
      // de aborto e o que importa: uma direcao descartada por UMA tentativa
      // frustrada volta a ser candidata na proxima passagem.
      _hasAbortedHeading = false;
      enterCenter(in.nowMs, false);
      out.phase = _phase;
      out.pose = _pose;
      return out;
    }

    out.source = source;
    _hasAbortedHeading = false;   // escolheu: a marca cumpriu o papel
    _targetHeading = nextHeading;
    reserveMoveDelta(_targetHeading);
    _phase = MotionPhase::TURN;
    _phaseStartMs = in.nowMs;
    _alignStable = 0;
    out.phase = _phase;
    // cai para o TURN no mesmo ciclo
  }

  if (_phase == MotionPhase::TURN) {
    // Fase unica de giro. Antes eram duas (PRE_ALIGN e TURN_90) mirando o
    // MESMO _targetHeading: PRE_ALIGN ja fazia o giro inteiro e TURN_90 so
    // commitava o _pose.heading depois.
    const float yawErr = shortestYawError(headingToYaw(_targetHeading), in.yawDeg);

    out.command = MotionCommand::DRIVE_CLOSED_LOOP;
    out.linearPwm = 0;
    out.turnPwm = clampPwm(yawErr * YAW_KP);
    out.phase = _phase;

    // "Vira (ou nao)": seguir em frente nao gasta fase nenhuma.
    const bool noTurnNeeded = (_targetHeading == _pose.heading);

    if (fabsf(yawErr) <= TURN_TOL_DEG) {
      if (_alignStable < ALIGN_STABLE_CYCLES) _alignStable++;
    } else {
      _alignStable = 0;
    }
    const bool settled = (_alignStable >= ALIGN_STABLE_CYCLES);

    // Meia-volta leva o dobro do tempo de um quarto de volta.
    const uint32_t limitMs = (_targetHeading == turnBack(_pose.heading))
                             ? ((uint32_t)_turn90Ms * 2u)
                             : (uint32_t)_turn90Ms;
    const bool timedOut = (in.nowMs - _phaseStartMs) >= limitMs;

    if (noTurnNeeded || settled) {
      _pose.heading = _targetHeading;
      // Nao vai direto para o DRIVE_TILE: passa por CENTER+ALIGN de novo.
      // O giro trocou o eixo longitudinal pelo lateral, entao o que estava
      // centrado antes agora e outra coisa - e e essa segunda centragem que
      // poe o robo no centro do ladrilho tambem no eixo novo.
      enterCenter(in.nowMs, true);
      out.turnPwm = 0;
      out.linearPwm = 0;
      out.pose = _pose;
      out.phase = _phase;
      return out;
    }

    if (timedOut) {
      // O giro NAO chegou no alvo. Commitar _targetHeading aqui era o que
      // transformava o mapa em ficcao: o robo registrava heading E com o
      // yaw parado em 354 graus (ou seja, N). Pior, seguia para o
      // DRIVE_TILE e andava um ladrilho na direcao errada.
      //
      // Registra o rumo que o IMU realmente ve e recomeca o ciclo, sem
      // andar. Se o giro estiver fisicamente impossivel o robo fica
      // repetindo o ciclo no lugar - visivel e inofensivo, ao contrario de
      // sair andando as cegas.
      _alignStable = 0;
      _pose.heading = headingFromYaw(in.yawDeg);
      _bfsLen = 0;   // o caminho planejado dependia do giro ter dado certo
      _bfsIdx = 0;
      enterCenter(in.nowMs, false);
      out.command = MotionCommand::HOLD;
      out.linearPwm = 0;
      out.turnPwm = 0;
      out.pose = _pose;
      out.phase = _phase;
    }
    return out;
  }

  if (_phase == MotionPhase::DRIVE_TILE) {
    const float yawErr = shortestYawError(headingToYaw(_targetHeading), in.yawDeg);
    const int16_t turnCmd = clampPwm(yawErr * YAW_KP);
    const int16_t centerCmd = clampPwm(((int32_t)in.tofLeftMm - (int32_t)in.tofRightMm) * CENTER_KP);

    out.command = MotionCommand::DRIVE_CLOSED_LOOP;
    out.linearPwm = LINEAR_TRAVEL_PWM;
    out.turnPwm = clampPwm(turnCmd + centerCmd);
    out.phase = _phase;

    const int32_t traveled = traveledTicks(in);
    const int32_t enough = (_ticksPerTile * TILE_COMPLETE_PCT) / 100;
    const bool nearWall = (in.tofFrontMm > 0 && in.tofFrontMm <= _frontStopMm);

    // 1) Ladrilho andado por inteiro. Unica saida realmente normal.
    if (traveled >= _ticksPerTile) {
      completeTile(in, out);
      return out;
    }

    // 2) Parede frontal com o ladrilho quase todo andado: fundo de beco.
    //    Chegou na celula, so nao deu para percorrer os ultimos ticks.
    if (nearWall && traveled >= enough) {
      completeTile(in, out);
      return out;
    }

    // 3) Parede frontal logo no comeco: nao havia caminho aqui. Antes as
    //    tres saidas avancavam a pose igual - era isso que dessincronizava
    //    o mapa quando FRONT_STOP_MM (90) disparava logo depois de o
    //    TOF_WALL_CLEAR_MM (120) ter liberado a passagem.
    if (nearWall) {
      abortTile(in, out, true);
      return out;
    }

    // 4) Watchdog: roda travada ou encoder mudo. Tambem nao conta ladrilho,
    //    mas nao culpa uma parede que pode nem existir.
    if ((in.nowMs - _phaseStartMs) >= _tileDriveMs) {
      abortTile(in, out, false);
      return out;
    }

    return out;
  }

  return out;
}

void RobotControl::enterCenter(uint32_t nowMs, bool thenDrive) {
  _phase = MotionPhase::CENTER;
  _phaseStartMs = nowMs;
  _alignStable = 0;
  _settleThenDrive = thenDrive;
  // O proximo ciclo desta fase ainda ve o robo onde ele parou, antes de
  // qualquer avanco ou recuo: e o momento de fotografar frente e tras.
  _latchPending = true;
}

void RobotControl::completeTile(const PlannerInput& in, PlannerOutput& out) {
  _pose.x += _tileDx;
  _pose.y += _tileDy;

  Cell* c = _map.at(_pose.x, _pose.y);
  if (c) c->visited = true;

  while (popCurrentFromDfs()) {}
  if (_returnHomeActive && _pose.x == _startX && _pose.y == _startY) {
    _returnHomeActive = false;
  }

  enterCenter(in.nowMs, false);
  out.command = MotionCommand::HOLD;
  out.linearPwm = 0;
  out.turnPwm = 0;
  out.pose = _pose;
  out.phase = _phase;
}

void RobotControl::abortTile(const PlannerInput& in, PlannerOutput& out, bool blockAhead) {
  // A pose NAO avanca: o robo nao chegou na celula seguinte.
  //
  // Limitacao conhecida: ele parou em algum ponto entre as duas celulas, e
  // a pose diz que ainda esta na de tras. O erro e pequeno quando a parede
  // aparece cedo (que e o caso que cai aqui), mas nao e zero. Recuar ate o
  // centro pediria uma fase de re, que ainda nao existe.
  if (blockAhead) {
    // Aqui a versao anterior marcava a celula da frente como blocked no
    // mapa. Errado por dois motivos:
    //
    // 1. A parede e propriedade da ARESTA entre duas celulas, nao da
    //    celula. Marcar (1,0) por causa de uma parede a leste de (0,0)
    //    declarava (1,0) inalcancavel tambem pelos outros tres lados.
    // 2. Era permanente. Uma unica tentativa frustrada condenava a celula
    //    para sempre - e com os encoders mudos TODO ladrilho aborta, entao
    //    tres tentativas bastavam para o robo se declarar cercado e travar
    //    no DECIDE.
    //
    // Agora e so memoria de uma tentativa, consumida pelo proximo DECIDE.
    // Nada fica gravado no mapa; a parede continua sendo o que o ToF disser
    // a cada ciclo.
    _abortedHeading = _targetHeading;
    _hasAbortedHeading = true;
  }

  // O caminho reservado nao foi cumprido: o resto do BFS nao vale mais.
  _bfsLen = 0;
  _bfsIdx = 0;

  enterCenter(in.nowMs, false);
  out.command = MotionCommand::HOLD;
  out.linearPwm = 0;
  out.turnPwm = 0;
  out.pose = _pose;
  out.phase = _phase;
}

void RobotControl::markCurrentTile(TileType tile) {
  Cell* c = _map.at(_pose.x, _pose.y);
  if (!c) return;

  c->visited = true;
  if (tile != TileType::UNKNOWN) c->tile = tile;

  if (tile == TileType::RED) c->danger = true;
  if (tile == TileType::SILVER) {
    c->checkpoint = true;
    _hasCheckpoint = true;
    _lastCheckpointX = _pose.x;
    _lastCheckpointY = _pose.y;
  }
  if (tile == TileType::BLACK) c->blocked = true;

  if (tile == TileType::BLUE && !c->noReentry) {
    _state = MissionState::WAITING_BLUE;
    _blueReleaseAtMs = millis() + 5000;
  }
}

void RobotControl::updateVictimStream(const PlannerInput& in) {
  Cell* c = _map.at(_pose.x, _pose.y);
  if (!c) return;

  if (in.victimConfLeft >= 50) {
    c->victimLeft = in.victimLeft;
    c->confLeft = in.victimConfLeft;
  }
  if (in.victimConfRight >= 50) {
    c->victimRight = in.victimRight;
    c->confRight = in.victimConfRight;
  }
}

void RobotControl::tryFinishBlueWait(uint32_t nowMs) {
  if (_state != MissionState::WAITING_BLUE) return;
  if (nowMs < _blueReleaseAtMs) return;

  Cell* c = _map.at(_pose.x, _pose.y);
  if (c) c->noReentry = true;
  _state = MissionState::NAVIGATING;
}

Heading RobotControl::turnLeft(Heading h) const {
  if (h == Heading::NORTH) return Heading::WEST;
  if (h == Heading::WEST) return Heading::SOUTH;
  if (h == Heading::SOUTH) return Heading::EAST;
  return Heading::NORTH;
}

Heading RobotControl::turnRight(Heading h) const {
  if (h == Heading::NORTH) return Heading::EAST;
  if (h == Heading::EAST) return Heading::SOUTH;
  if (h == Heading::SOUTH) return Heading::WEST;
  return Heading::NORTH;
}

Heading RobotControl::turnBack(Heading h) const {
  if (h == Heading::NORTH) return Heading::SOUTH;
  if (h == Heading::SOUTH) return Heading::NORTH;
  if (h == Heading::EAST) return Heading::WEST;
  return Heading::EAST;
}

void RobotControl::stepForward(Pose2D& p) const {
  if (p.heading == Heading::NORTH) p.y += 1;
  else if (p.heading == Heading::SOUTH) p.y -= 1;
  else if (p.heading == Heading::EAST) p.x += 1;
  else p.x -= 1;
}

bool RobotControl::isBlockedOrForbidden(int16_t x, int16_t y) const {
  const Cell* c = _map.at(x, y);
  if (!c) return true;
  return c->blocked || c->noReentry;
}

bool RobotControl::chooseDfsNeighbor(const PlannerInput& in, Heading& outHeading) {
  // DFS preference: front -> left -> right -> back.
  // rel: 0 = frente, 1 = esquerda, 2 = direita, 3 = tras.
  const uint8_t order[4] = {0, 1, 2, 3};
  for (uint8_t k = 0; k < 4; k++) {
    const uint8_t rel = order[k];
    if (!relativeFree(in, rel)) continue;

    const Heading h = relativeToHeading(_pose.heading, rel);

    // Nao repete a direcao que acabou de falhar. E memoria de uma passagem
    // so: se nada mais servir, o DECIDE limpa a marca e reavalia.
    if (_hasAbortedHeading && h == _abortedHeading) continue;

    Pose2D p = _pose;
    p.heading = h;
    stepForward(p);
    const Cell* c = _map.at(p.x, p.y);
    if (!c) continue;
    if (isBlockedOrForbidden(p.x, p.y)) continue;

    if (!c->visited) {
      pushDfs(_pose.x, _pose.y);
      _bfsLen = 0;
      _bfsIdx = 0;
      outHeading = h;
      return true;
    }
  }
  return false;
}

bool RobotControl::buildBfsPathTo(int16_t tx, int16_t ty) {
  static int16_t qx[MazeMap::WIDTH * MazeMap::HEIGHT];
  static int16_t qy[MazeMap::WIDTH * MazeMap::HEIGHT];
  static bool vis[MazeMap::WIDTH][MazeMap::HEIGHT];
  static int16_t px[MazeMap::WIDTH][MazeMap::HEIGHT];
  static int16_t py[MazeMap::WIDTH][MazeMap::HEIGHT];

  const int16_t ox = MazeMap::WIDTH / 2;
  const int16_t oy = MazeMap::HEIGHT / 2;
  const int16_t sx = _pose.x + ox;
  const int16_t sy = _pose.y + oy;
  const int16_t gx = tx + ox;
  const int16_t gy = ty + oy;

  if (sx < 0 || sy < 0 || gx < 0 || gy < 0) return false;
  if (sx >= MazeMap::WIDTH || gx >= MazeMap::WIDTH) return false;
  if (sy >= MazeMap::HEIGHT || gy >= MazeMap::HEIGHT) return false;

  for (int16_t x = 0; x < MazeMap::WIDTH; x++) {
    for (int16_t y = 0; y < MazeMap::HEIGHT; y++) {
      vis[x][y] = false;
      px[x][y] = -32768;
      py[x][y] = -32768;
    }
  }

  uint16_t head = 0;
  uint16_t tail = 0;
  qx[tail] = sx;
  qy[tail] = sy;
  tail++;
  vis[sx][sy] = true;

  const int8_t dx[4] = {0, -1, 1, 0};
  const int8_t dy[4] = {1, 0, 0, -1};
  bool found = false;

  while (head < tail) {
    const int16_t cx = qx[head];
    const int16_t cy = qy[head];
    head++;

    if (cx == gx && cy == gy) {
      found = true;
      break;
    }

    for (uint8_t i = 0; i < 4; i++) {
      const int16_t nx = cx + dx[i];
      const int16_t ny = cy + dy[i];
      if (nx < 0 || ny < 0 || nx >= MazeMap::WIDTH || ny >= MazeMap::HEIGHT) continue;
      if (vis[nx][ny]) continue;

      const int16_t wx = nx - ox;
      const int16_t wy = ny - oy;
      if (!isVisitedPassable(wx, wy)) continue;

      vis[nx][ny] = true;
      px[nx][ny] = cx;
      py[nx][ny] = cy;
      qx[tail] = nx;
      qy[tail] = ny;
      tail++;
    }
  }

  if (!found) return false;

  int16_t pathX[BFS_PATH_MAX + 1];
  int16_t pathY[BFS_PATH_MAX + 1];
  uint16_t plen = 0;
  int16_t cx = gx;
  int16_t cy = gy;

  while (!(cx == sx && cy == sy)) {
    if (plen >= BFS_PATH_MAX) return false;
    pathX[plen] = cx;
    pathY[plen] = cy;
    plen++;
    const int16_t nx = px[cx][cy];
    const int16_t ny = py[cx][cy];
    if (nx == -32768 || ny == -32768) return false;
    cx = nx;
    cy = ny;
  }

  _bfsLen = 0;
  _bfsIdx = 0;
  int16_t fromX = sx;
  int16_t fromY = sy;
  for (int16_t i = (int16_t)plen - 1; i >= 0; i--) {
    const int16_t toX = pathX[i];
    const int16_t toY = pathY[i];
    Heading h;
    if (!headingFromTo(fromX - ox, fromY - oy, toX - ox, toY - oy, h)) return false;
    if (_bfsLen >= BFS_PATH_MAX) return false;
    _bfsPath[_bfsLen++] = h;
    fromX = toX;
    fromY = toY;
  }
  return (_bfsLen > 0);
}

bool RobotControl::hasFrontier() const {
  for (int16_t y = -(MazeMap::HEIGHT / 2); y <= (MazeMap::HEIGHT / 2); y++) {
    for (int16_t x = -(MazeMap::WIDTH / 2); x <= (MazeMap::WIDTH / 2); x++) {
      const Cell* c = _map.at(x, y);
      if (!c || !c->visited) continue;
      if (c->blocked || c->noReentry) continue;

      const int16_t nx[4] = {x, x - 1, x + 1, x};
      const int16_t ny[4] = {y + 1, y, y, y - 1};
      for (uint8_t i = 0; i < 4; i++) {
        const Cell* n = _map.at(nx[i], ny[i]);
        if (!n) continue;
        if (!n->visited && !n->blocked && !n->noReentry) return true;
      }
    }
  }
  return false;
}

bool RobotControl::isExplorationComplete() const {
  return !hasFrontier();
}

bool RobotControl::popCurrentFromDfs() {
  if (_dfsSize == 0) return false;
  const Node2D top = _dfsStack[_dfsSize - 1];
  if (top.x != _pose.x || top.y != _pose.y) return false;
  _dfsSize--;
  return true;
}

bool RobotControl::isVisitedPassable(int16_t x, int16_t y) const {
  const Cell* c = _map.at(x, y);
  if (!c) return false;
  if (!c->visited) return false;
  return !(c->blocked || c->noReentry);
}

bool RobotControl::headingFromTo(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Heading& out) const {
  const int16_t dx = x1 - x0;
  const int16_t dy = y1 - y0;
  if (dx == 0 && dy == 1) { out = Heading::NORTH; return true; }
  if (dx == 1 && dy == 0) { out = Heading::EAST; return true; }
  if (dx == 0 && dy == -1) { out = Heading::SOUTH; return true; }
  if (dx == -1 && dy == 0) { out = Heading::WEST; return true; }
  return false;
}

bool RobotControl::relativeFree(const PlannerInput& in, uint8_t rel) const {
  // Frente e tras combinam a leitura fresca com a latchada no centro do
  // ladrilho: o CENTER move o robo no eixo longitudinal, afastando ou
  // sem o latch uma parede frontal a 90 mm viraria "livre" so por o robo
  // ter recuado para 140 mm. Basta um dos dois acusar parede para bloquear.
  //
  // Esquerda e direita saem so da leitura fresca, ja alinhada: sao paredes
  // paralelas ao percurso e mal mudam com o recuo.
  if (rel == 0) return in.frontFree && _frontFreeAtCenter;
  if (rel == 1) return in.leftFree;
  if (rel == 2) return in.rightFree;
  return in.backFree && _backFreeAtCenter;
}

Heading RobotControl::relativeToHeading(Heading base, uint8_t rel) const {
  if (rel == 0) return base;
  if (rel == 1) return turnLeft(base);
  if (rel == 2) return turnRight(base);
  return turnBack(base);
}

bool RobotControl::pushDfs(int16_t x, int16_t y) {
  if (_dfsSize >= DFS_STACK_MAX) return false;
  _dfsStack[_dfsSize++] = {x, y};
  return true;
}

void RobotControl::reserveMoveDelta(Heading h) {
  _tileDx = 0;
  _tileDy = 0;
  if (h == Heading::NORTH) _tileDy = 1;
  else if (h == Heading::SOUTH) _tileDy = -1;
  else if (h == Heading::EAST) _tileDx = 1;
  else _tileDx = -1;
}

float RobotControl::headingToYaw(Heading h) const {
  if (h == Heading::NORTH) return 0.0f;
  if (h == Heading::EAST) return 90.0f;
  if (h == Heading::SOUTH) return 180.0f;
  return 270.0f;
}

Heading RobotControl::headingFromYaw(float yawDeg) const {
  // Cardinal mais proximo do yaw medido. Usado quando um giro nao completa:
  // vale mais registrar onde o robo esta do que onde ele deveria estar.
  float y = yawDeg;
  while (y < 0.0f) y += 360.0f;
  while (y >= 360.0f) y -= 360.0f;

  if (y < 45.0f || y >= 315.0f) return Heading::NORTH;
  if (y < 135.0f) return Heading::EAST;
  if (y < 225.0f) return Heading::SOUTH;
  return Heading::WEST;
}

float RobotControl::shortestYawError(float targetDeg, float currentDeg) const {
  float e = targetDeg - currentDeg;
  while (e > 180.0f) e -= 360.0f;
  while (e < -180.0f) e += 360.0f;
  return e;
}

int16_t RobotControl::clampPwm(float v) const {
  if (v > MAX_TURN) v = MAX_TURN;
  if (v < -MAX_TURN) v = -MAX_TURN;
  return (int16_t)v;
}

int32_t RobotControl::traveledTicks(const PlannerInput& in) const {
  const int32_t deltaFl = in.encFlTicks - _startFlTicks;
  const int32_t deltaRr = in.encRrTicks - _startRrTicks;

  // Com o sinal normalizado por Encoder::setInverted(), avancar faz os DOIS
  // deltas crescerem juntos. Sinais opostos agora significam de fato uma
  // anomalia - giro no lugar, roda patinando, fio solto - e nao o caso
  // normal de rodas espelhadas, como a versao anterior supunha.
  //
  // Nesse caso nao ha avanco que possa ser afirmado: devolve 0 e deixa o
  // watchdog de tempo da fase resolver. Contar o giro como distancia seria
  // pior, porque dessincroniza a pose do mapa.
  const bool opposite = (deltaFl > 0 && deltaRr < 0) || (deltaFl < 0 && deltaRr > 0);
  if (opposite) return 0;

  const int32_t dfl = abs(deltaFl);
  const int32_t drr = abs(deltaRr);

  // Discordancia grande entre as rodas: uma delas provavelmente parou de
  // contar. Confia na que andou mais - senao o ladrilho nunca termina.
  const int32_t ERROR_THRESHOLD = 150;
  if (abs(dfl - drr) > ERROR_THRESHOLD) {
    return (dfl > drr) ? dfl : drr;
  }
  return (dfl + drr) / 2;
}

const char* RobotControl::toString(TileType t) {
  switch (t) {
    case TileType::RED: return "RED";
    case TileType::BLUE: return "BLUE";
    case TileType::SILVER: return "SILVER";
    case TileType::BLACK: return "BLACK";
    default: return "UNKNOWN";
  }
}

const char* RobotControl::toString(MotionCommand c) {
  switch (c) {
    case MotionCommand::DRIVE_CLOSED_LOOP: return "DRIVE_CLOSED_LOOP";
    case MotionCommand::RETREAT_ONE: return "RETREAT_ONE";
    default: return "HOLD";
  }
}

const char* RobotControl::toString(DecisionSource s) {
  switch (s) {
    case DecisionSource::DFS: return "DFS";
    case DecisionSource::BFS: return "BFS";
    default: return "NONE";
  }
}

const char* RobotControl::toString(MissionState s) {
  switch (s) {
    case MissionState::WAITING_BLUE: return "WAITING_BLUE";
    case MissionState::ESCAPING_BLACK: return "ESCAPING_BLACK";
    default: return "NAVIGATING";
  }
}

const char* RobotControl::toString(MotionPhase p) {
  switch (p) {
    case MotionPhase::CENTER: return "CENTER";
    case MotionPhase::ALIGN: return "ALIGN";
    case MotionPhase::TURN: return "TURN";
    case MotionPhase::DRIVE_TILE: return "DRIVE_TILE";
    default: return "DECIDE";
  }
}
