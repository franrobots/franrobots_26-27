#pragma once
#include <Arduino.h>

enum class TileType : uint8_t {
  UNKNOWN,
  RED,
  BLUE,
  SILVER,
  BLACK
};

enum class Heading : uint8_t {
  NORTH,
  EAST,
  SOUTH,
  WEST
};

enum class MissionState : uint8_t {
  NAVIGATING,
  WAITING_BLUE,
  ESCAPING_BLACK
};

enum class MotionPhase : uint8_t {
  DECIDE,
  PRE_ALIGN,
  TURN_90,
  DRIVE_TILE
};

enum class MotionCommand : uint8_t {
  HOLD,
  DRIVE_CLOSED_LOOP,
  RETREAT_ONE
};

enum class DecisionSource : uint8_t {
  NONE,
  DFS,
  BFS
};

struct Pose2D {
  int16_t x = 0;
  int16_t y = 0;
  Heading heading = Heading::NORTH;
};

struct Cell {
  TileType tile = TileType::UNKNOWN;
  bool visited = false;
  bool blocked = false;
  bool checkpoint = false;
  bool danger = false;
  bool noReentry = false;
  uint8_t victimLeft = 0;
  uint8_t victimRight = 0;
  uint8_t confLeft = 0;
  uint8_t confRight = 0;
};

struct PlannerInput {
  TileType sensedTile = TileType::UNKNOWN;
  bool frontFree = true;
  bool leftFree = true;
  bool rightFree = true;
  bool backFree = true;

  // Fusion inputs for motion control.
  float yawDeg = 0.0f;          // 0..360 (BNO055)
  int32_t encFlTicks = 0;       // encoder front-left absolute ticks
  int32_t encRrTicks = 0;       // encoder rear-right absolute ticks
  uint16_t tofLeftMm = 0;       // wall distance for lateral centering
  uint16_t tofRightMm = 0;      // wall distance for lateral centering
  uint16_t tofFrontMm = 0;      // front wall distance (parada de seguranca no DRIVE_TILE)

  // OpenMV victim stream.
  uint8_t victimLeft = 0;
  uint8_t victimRight = 0;
  uint8_t victimConfLeft = 0;
  uint8_t victimConfRight = 0;

  uint32_t nowMs = 0;
};

struct PlannerOutput {
  MotionCommand command = MotionCommand::HOLD;
  DecisionSource source = DecisionSource::NONE;
  MissionState state = MissionState::NAVIGATING;
  MotionPhase phase = MotionPhase::DECIDE;
  Pose2D pose;
  int16_t linearPwm = 0;
  int16_t turnPwm = 0;
};

class MazeMap {
public:
  static constexpr int16_t WIDTH = 41;
  static constexpr int16_t HEIGHT = 41;

  MazeMap();
  Cell* at(int16_t x, int16_t y);
  const Cell* at(int16_t x, int16_t y) const;

private:
  Cell _grid[HEIGHT][WIDTH];
  static constexpr int16_t ORIGIN_X = WIDTH / 2;
  static constexpr int16_t ORIGIN_Y = HEIGHT / 2;
};

class RobotControl {
public:
  RobotControl();

  void begin(int16_t startX = 0, int16_t startY = 0, Heading startHeading = Heading::NORTH);
  void setTicksPerTile(int32_t ticks) { _ticksPerTile = (ticks > 1) ? ticks : 1; }

  // Limites de tempo por fase. Servem de fallback quando nao ha IMU/encoder
  // reais: sem eles o erro de yaw e a contagem de ticks nunca convergem.
  // Com IMU/encoder ligados, a condicao normal dispara antes do timeout.
  void setPreAlignMs(uint16_t ms)  { _preAlignMs = ms; }
  void setTurn90Ms(uint16_t ms)    { _turn90Ms = ms; }
  void setTileDriveMs(uint32_t ms) { _tileDriveMs = ms; }
  void setFrontStopMm(uint16_t mm) { _frontStopMm = mm; }

  PlannerOutput update(const PlannerInput& in);
  bool relocateToLastCheckpoint();
  bool isExplorationComplete() const;

  const Pose2D& pose() const { return _pose; }
  // Rumo que o robo deve estar perseguindo agora. Durante PRE_ALIGN/TURN_90
  // ele difere de pose().heading, que so e atualizado ao fim do giro.
  Heading targetHeading() const { return _targetHeading; }
  MissionState state() const { return _state; }
  MotionPhase phase() const { return _phase; }
  const MazeMap& map() const { return _map; }

  static const char* toString(TileType t);
  static const char* toString(MotionCommand c);
  static const char* toString(DecisionSource s);
  static const char* toString(MissionState s);
  static const char* toString(MotionPhase p);

private:
  struct Node2D {
    int16_t x;
    int16_t y;
  };

  MazeMap _map;
  Pose2D _pose;
  MissionState _state = MissionState::NAVIGATING;
  MotionPhase _phase = MotionPhase::DECIDE;
  uint32_t _phaseStartMs = 0;

  uint16_t _preAlignMs = 250;
  uint16_t _turn90Ms = 700;
  uint32_t _tileDriveMs = 1500;
  uint16_t _frontStopMm = 90;

  Heading _targetHeading = Heading::NORTH;
  int32_t _startFlTicks = 0;
  int32_t _startRrTicks = 0;
  int16_t _tileDx = 0;
  int16_t _tileDy = 0;

  uint32_t _blueReleaseAtMs = 0;
  int32_t _ticksPerTile = 860;  // TODO calibrar com 28 cm reais
  bool _hasCheckpoint = false;
  int16_t _lastCheckpointX = 0;
  int16_t _lastCheckpointY = 0;
  int16_t _startX = 0;
  int16_t _startY = 0;
  bool _returnHomeActive = false;

  static constexpr float YAW_KP = 7.0f;
  static constexpr float CENTER_KP = 0.5f;
  static constexpr float MAX_TURN = 1800.0f;
  static constexpr float TURN_TOL_DEG = 3.0f;

  static constexpr int16_t LINEAR_TRAVEL_PWM = 2200;
  static constexpr int16_t LINEAR_ALIGN_PWM = 1400;
  // Positivo = girar para a ESQUERDA (mesma convencao de Robot::turnLeft).
  // Se o robo girar ao contrario na pista, inverta o sinal aqui.
  static constexpr int16_t TURN_PWM = 1500;

  static constexpr uint16_t DFS_STACK_MAX = 512;
  static constexpr uint16_t BFS_PATH_MAX = 256;
  Node2D _dfsStack[DFS_STACK_MAX];
  uint16_t _dfsSize = 0;
  Heading _bfsPath[BFS_PATH_MAX];
  uint16_t _bfsLen = 0;
  uint16_t _bfsIdx = 0;

  void markCurrentTile(TileType tile);
  void updateVictimStream(const PlannerInput& in);
  void tryFinishBlueWait(uint32_t nowMs);

  Heading turnLeft(Heading h) const;
  Heading turnRight(Heading h) const;
  Heading turnBack(Heading h) const;
  void stepForward(Pose2D& p) const;

  bool isBlockedOrForbidden(int16_t x, int16_t y) const;
  void reserveMoveDelta(Heading h);
  bool chooseDfsNeighbor(const PlannerInput& in, Heading& outHeading);
  bool buildBfsPathTo(int16_t tx, int16_t ty);
  bool hasFrontier() const;
  bool popCurrentFromDfs();
  bool isVisitedPassable(int16_t x, int16_t y) const;
  bool headingFromTo(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Heading& out) const;
  bool relativeFree(const PlannerInput& in, uint8_t rel) const;
  Heading relativeToHeading(Heading base, uint8_t rel) const;
  bool pushDfs(int16_t x, int16_t y);

  float headingToYaw(Heading h) const;
  float shortestYawError(float targetDeg, float currentDeg) const;
  int16_t clampPwm(float v) const;
  int32_t traveledTicks(const PlannerInput& in) const;
};
