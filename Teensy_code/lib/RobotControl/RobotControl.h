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

// Ciclo de movimento:
//
//   CENTER      posiciona no eixo em que olha (avanca ou recua ate o alvo)
//   ALIGN       quadra o rumo no cardinal ATUAL, parado
//   DECIDE      escolhe o proximo rumo (frente, esquerda, direita, tras)
//   TURN        gira ate o rumo escolhido - pulado se ja for o atual
//   DRIVE_TILE  anda um ladrilho
//
// A dupla CENTER+ALIGN roda DUAS vezes por ladrilho:
//
//   DRIVE_TILE -> CENTER -> ALIGN -> DECIDE -> TURN -> CENTER -> ALIGN
//              -> DRIVE_TILE -> ...
//
// depois de andar e depois de girar. E o que _settleThenDrive controla.
//
// Por que as duas passagens cobrem os DOIS eixos: girar 90 graus troca o
// eixo longitudinal pelo lateral. Centrar no eixo A, girar, e centrar no
// eixo B poe o robo no centro do ladrilho nos dois - que e o que a traccao
// diferencial nao consegue fazer de uma vez, ja que o robo nao anda de lado.
//
// ALIGN vem ANTES de DECIDE de proposito: um robo torto le as paredes
// laterais na diagonal e decide em cima de distancia errada.
//
// CENTER vem antes do ALIGN porque girar com o nariz colado na parede
// raspa: a diagonal do chassi e maior que a meia-largura, entao o canto
// avanca durante o giro mesmo o centro ficando parado.
enum class MotionPhase : uint8_t {
  CENTER,
  ALIGN,
  DECIDE,
  TURN,
  DRIVE_TILE
};

// Parametros da fase CENTER. Agrupados porque sao muitos e andam juntos.
struct CenteringConfig {
  uint16_t frontTargetMm = 130;   // alvo com parede a frente
  uint16_t backTargetMm  = 130;   // alvo quando so ha parede atras
  uint16_t refValidMm    = 400;   // acima disso nao e a borda deste ladrilho
  uint16_t tolMm         = 10;    // banda morta
  uint16_t frontMinMm    = 110;   // nunca avancar alem disso
  uint16_t backMinMm     = 70;    // nunca recuar alem disso
  float    kp            = 14.0f; // mm -> PWM
  int16_t  minPwm        = 1500;  // piso de arranque linear
  int16_t  maxPwm        = 1900;
  uint16_t maxMs         = 1800;  // watchdog
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
  uint16_t tofFrontMm = 0;      // front wall distance (parada no DRIVE_TILE, alvo do BACKOFF)
  uint16_t tofBackMm = 0;       // back wall distance (limite de recuo do BACKOFF)

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
  void setAlignMs(uint16_t ms)     { _alignMs = ms; }
  void setTurn90Ms(uint16_t ms)    { _turn90Ms = ms; }
  void setTileDriveMs(uint32_t ms) { _tileDriveMs = ms; }
  void setFrontStopMm(uint16_t mm) { _frontStopMm = mm; }

  void setCentering(const CenteringConfig& c) { _center = c; }

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
  MotionPhase _phase = MotionPhase::CENTER;
  uint32_t _phaseStartMs = 0;

  uint16_t _alignMs = 800;
  uint16_t _turn90Ms = 700;
  uint32_t _tileDriveMs = 1500;
  uint16_t _frontStopMm = 90;

  CenteringConfig _center;

  // Para onde o ALIGN vai ao terminar. true logo depois de um giro (segue
  // para DRIVE_TILE), false logo depois de andar (segue para DECIDE).
  // E o que faz a dupla CENTER+ALIGN rodar nos dois pontos do ciclo.
  bool _settleThenDrive = false;

  // Ciclos consecutivos dentro da tolerancia de rumo. Exigir varios evita
  // dar por alinhado o robo que so passou pelo alvo em velocidade.
  uint8_t _alignStable = 0;

  // Paredes da frente e de tras como estavam no CENTRO do ladrilho, antes
  // do recuo. O BACKOFF afasta o nariz e aproxima a traseira: reler depois
  // dele daria "frente livre" para uma parede que continua exatamente ali.
  // Esquerda e direita nao sao latchadas - correm paralelas ao percurso e
  // quase nao mudam com o recuo, e e delas que o ALIGN cuida.
  bool _frontFreeAtCenter = true;
  bool _backFreeAtCenter = true;
  bool _latchPending = true;

  // Memoria de UMA tentativa frustrada, consumida pelo proximo DECIDE.
  // Nao vai para o mapa de proposito - ver o comentario em abortTile().
  Heading _abortedHeading = Heading::NORTH;
  bool _hasAbortedHeading = false;

  Heading _targetHeading = Heading::NORTH;
  int32_t _startFlTicks = 0;
  int32_t _startRrTicks = 0;
  int16_t _tileDx = 0;
  int16_t _tileDy = 0;

  uint32_t _blueReleaseAtMs = 0;
  // Fallback: o valor real vem de setTicksPerTile(ENCODER_TICKS_PER_TILE).
  int32_t _ticksPerTile = 225;
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

  // Ciclos seguidos dentro de TURN_TOL_DEG para dar o rumo por assentado.
  // A 50 Hz (CONTROL_PERIOD_MS = 20), 5 ciclos = 100 ms.
  static constexpr uint8_t ALIGN_STABLE_CYCLES = 5;

  // Fracao do ladrilho a partir da qual uma parede frontal conta como fim
  // NORMAL do percurso (fundo de beco) em vez de aborto. Abaixo disso, a
  // parede apareceu cedo demais: nao havia caminho ali.
  static constexpr int32_t TILE_COMPLETE_PCT = 60;

  // O main.cpp fecha o PID e sobrescreve out.linearPwm/out.turnPwm em toda
  // fase, entao estes valores so chegam no motor no caso RETREAT_ONE. Os
  // ganhos que ainda mandam de verdade aqui sao TURN_TOL_DEG e
  // ALIGN_STABLE_CYCLES, que decidem as transicoes de fase.
  static constexpr int16_t LINEAR_TRAVEL_PWM = 2200;
  static constexpr int16_t LINEAR_ALIGN_PWM = 1400;

  static constexpr uint16_t DFS_STACK_MAX = 512;
  static constexpr uint16_t BFS_PATH_MAX = 256;
  Node2D _dfsStack[DFS_STACK_MAX];
  uint16_t _dfsSize = 0;
  Heading _bfsPath[BFS_PATH_MAX];
  uint16_t _bfsLen = 0;
  uint16_t _bfsIdx = 0;

  // Fim do DRIVE_TILE. completeTile avanca a pose; abortTile NAO - o robo
  // nao chegou na celula seguinte, e fingir que chegou dessincroniza o mapa.
  void completeTile(const PlannerInput& in, PlannerOutput& out);
  void abortTile(const PlannerInput& in, PlannerOutput& out, bool blockAhead);
  void enterCenter(uint32_t nowMs, bool thenDrive);

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
  Heading headingFromYaw(float yawDeg) const;
  float shortestYawError(float targetDeg, float currentDeg) const;
  int16_t clampPwm(float v) const;
  int32_t traveledTicks(const PlannerInput& in) const;
};
