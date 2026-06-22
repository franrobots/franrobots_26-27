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
  _phase = MotionPhase::DECIDE;
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
  _phase = MotionPhase::DECIDE;
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
    _phase = MotionPhase::DECIDE;
    _state = MissionState::NAVIGATING;
    return out;
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

      // If no DFS/BFS move remains, only return home when exploration is complete.
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
      return out;
    }

    out.source = source;
    _targetHeading = nextHeading;
    reserveMoveDelta(_targetHeading);
    _phase = MotionPhase::PRE_ALIGN;
  }

  if (_phase == MotionPhase::PRE_ALIGN) {
    const float yawErr = shortestYawError(headingToYaw(_targetHeading), in.yawDeg);
    const int16_t turnCmd = clampPwm(yawErr * YAW_KP);
    const int16_t centerCmd = clampPwm(((int32_t)in.tofLeftMm - (int32_t)in.tofRightMm) * CENTER_KP);

    out.command = MotionCommand::DRIVE_CLOSED_LOOP;
    out.linearPwm = (fabsf(yawErr) < 12.0f) ? LINEAR_ALIGN_PWM : 0;
    out.turnPwm = clampPwm(turnCmd + centerCmd);
    out.phase = _phase;

    if (fabsf(yawErr) <= TURN_TOL_DEG) {
      _phase = MotionPhase::TURN_90;
    }
    return out;
  }

  if (_phase == MotionPhase::TURN_90) {
    const float yawErr = shortestYawError(headingToYaw(_targetHeading), in.yawDeg);
    out.command = MotionCommand::DRIVE_CLOSED_LOOP;
    out.linearPwm = 0;
    out.turnPwm = clampPwm(yawErr * (YAW_KP * 1.2f));
    out.phase = _phase;

    if (fabsf(yawErr) <= TURN_TOL_DEG) {
      _pose.heading = _targetHeading;
      _startFlTicks = in.encFlTicks;
      _startRrTicks = in.encRrTicks;
      _phase = MotionPhase::DRIVE_TILE;
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

    if (traveledTicks(in) >= _ticksPerTile) {
      _pose.x += _tileDx;
      _pose.y += _tileDy;
      Cell* c = _map.at(_pose.x, _pose.y);
      if (c) c->visited = true;
      while (popCurrentFromDfs()) {}
      if (_returnHomeActive && _pose.x == _startX && _pose.y == _startY) {
        _returnHomeActive = false;
      }
      _phase = MotionPhase::DECIDE;
      out.command = MotionCommand::HOLD;
      out.linearPwm = 0;
      out.turnPwm = 0;
      out.pose = _pose;
      out.phase = _phase;
    }
    return out;
  }

  return out;
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
  // DFS preference: left -> front -> right -> back.
  const uint8_t order[4] = {1, 0, 2, 3};
  for (uint8_t k = 0; k < 4; k++) {
    const uint8_t rel = order[k];
    if (!relativeFree(in, rel)) continue;

    const Heading h = relativeToHeading(_pose.heading, rel);
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
  if (rel == 0) return in.frontFree;
  if (rel == 1) return in.leftFree;
  if (rel == 2) return in.rightFree;
  return in.backFree;
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

  const int32_t dfl = abs(deltaFl);
  const int32_t drr = abs(deltaRr);
  const int32_t ERROR_THRESHOLD = 150;
  bool direcao = (deltaFl * deltaRr < 0);
  int32_t diff = abs(dfl - drr);
  if (diff > ERROR_THRESHOLD || direcao) {
    if (dfl > drr) {
        return dfl; // Confia só na roda dianteira esquerda
      } else {
        return drr; // Confia só na roda traseira direita
      }
  } else{
    return (dfl + drr) / 2;
  }
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
    case MotionPhase::PRE_ALIGN: return "PRE_ALIGN";
    case MotionPhase::TURN_90: return "TURN_90";
    case MotionPhase::DRIVE_TILE: return "DRIVE_TILE";
    default: return "DECIDE";
  }
}
