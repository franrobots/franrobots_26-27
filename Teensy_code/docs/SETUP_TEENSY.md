# 🤖 Guia Completo: Setup Teensy 4.0 - FranRobots

## 📋 Pré-Requisitos

### 1. **Instalar PlatformIO**
```bash
# Opção A: CLI (recomendado)
# Baixe em: https://platformio.org/install/cli

# Opção B: Extensão VS Code
# - Abra VS Code
# - Vá para Extensions (Ctrl+Shift+X)
# - Procure por "PlatformIO"
# - Clique em "Install"
```

### 2. **Instalar Teensy Loader**
```
- Baixe em: https://www.pjrc.com/teensy/loader.html
- Instale para seu sistema operacional
```

### 3. **Python** (requerido por PlatformIO)
```bash
# Verificar se está instalado:
python --version

# Se não tiver:
# Baixe em: https://www.python.org/downloads/
```

---

## 🚀 Passos de Instalação

### **Passo 1: Abrir o Projeto**

```bash
# Navegue até o diretório do projeto
cd c:\Users\Instrutor\Desktop\franrobots_26-27\Teensy_code

# Se usar VS Code:
code .

# Se usar PlatformIO CLI:
pio project init --ide vscode
```

---

### **Passo 2: Baixar Todas as Dependências**

```bash
# Opção A: Via VS Code + PlatformIO
# 1. Abra a paleta de comandos (Ctrl+Shift+P)
# 2. Digite: "PlatformIO: Build"
# Isso baixará todas as dependências automaticamente

# Opção B: Via CLI
cd c:\Users\Instrutor\Desktop\franrobots_26-27\Teensy_code
pio run --target=build
```

**O que será baixado:**
- ✓ Framework Arduino para Teensy
- ✓ Compilador ARM
- ✓ Biblioteca Pololu VL53L0X (sensores ToF)
- ✓ Biblioteca Adafruit BNO055 (IMU)
- ✓ Biblioteca Adafruit NeoPixel (LEDs WS2812)
- ✓ Biblioteca Adafruit BusIO (dependência)
- ✓ Todas as bibliotecas customizadas em `/lib`

---

### **Passo 3: Conectar a Teensy**

1. Conecte a Teensy 4.0 ao computador via USB
2. Aguarde o computador reconhecer o dispositivo
3. Anote a porta COM (ex: `COM3`)

---

### **Passo 4: Compilar o Código**

#### **Opção A: VS Code + PlatformIO (Recomendado)**

```bash
# 1. Abra a paleta de comandos (Ctrl+Shift+P)
# 2. Digite: "PlatformIO: Build"
# Aguarde a compilação completar

# Ou use o atalho na barra inferior:
# Clique no ícone PlatformIO → Build
```

#### **Opção B: CLI**

```bash
cd c:\Users\Instrutor\Desktop\franrobots_26-27\Teensy_code
pio run -e teensy40
```

**Resultado esperado:**
```
Building in release mode
Compiling .pio/build/teensy40/src/main.cpp.o
Linking .pio/build/teensy40/firmware.elf
Building .pio/build/teensy40/firmware.hex

======================== [SUCCESS] Took X.XX seconds ========================
```

---

### **Passo 5: Upload para a Teensy**

#### **Opção A: VS Code + PlatformIO**

```bash
# 1. Abra a paleta de comandos (Ctrl+Shift+P)
# 2. Digite: "PlatformIO: Upload"
# Aguarde o upload

# Ou clique no ícone PlatformIO → Upload
```

#### **Opção B: CLI**

```bash
cd c:\Users\Instrutor\Desktop\franrobots_26-27\Teensy_code
pio run -e teensy40 --target=upload
```

#### **Opção C: Manual com Teensy Loader**

```bash
# 1. Abra Teensy Loader
# 2. Clique em "File" → "Open Hex File"
# 3. Navegue até: .pio/build/teensy40/firmware.hex
# 4. Clique em "Program"
# 5. Pressione o botão PROGRAM na Teensy
# 6. Aguarde "Programming complete"
```

---

### **Passo 6: Monitorar Serial**

Depois que o upload terminar, você pode monitorar a saída serial:

#### **Opção A: VS Code + PlatformIO**

```bash
# 1. Abra a paleta de comandos (Ctrl+Shift+P)
# 2. Digite: "PlatformIO: Monitor"
# Baud rate padrão: 115200
```

#### **Opção B: CLI**

```bash
pio device monitor -b 115200
```

#### **Opção C: Serial Monitor Padrão**

Você verá mensagens como:
```
Sistema iniciando...
Aviso: BNO055 nao respondeu.
Erro ToF
Calibracao de cor carregada da EEPROM.
Sistema pronto.
Deus abençoe o round.

Tile=UNKNOWN | State=NAVIGATING | Phase=DECIDE | Cmd=HOLD | Nav=NONE | PWM(0,0) | Yaw=0.0 | Enc(0,0) | Vict(0@0,0@0) | Pose=(0,0)
```

---

## 📦 Dependências Detalhadas

### **Dependências Externas (baixadas automaticamente)**

| Biblioteca | Versão | Função |
|-----------|--------|--------|
| pololu/VL53L0X | ^1.3.1 | Sensores ToF (distância) |
| adafruit/Adafruit BNO055 | ^1.6.4 | IMU (ângulo/yaw) |
| adafruit/Adafruit NeoPixel | ^1.12.5 | LEDs RGB endereçáveis |
| adafruit/Adafruit Unified Sensor | ^1.1.15 | API padrão Adafruit |
| adafruit/Adafruit BusIO | ^1.11.0 | I2C/SPI abstrato |

### **Dependências Padrão Teensy (já incluídas)**

- `Wire.h` - Comunicação I2C
- `SPI.h` - Comunicação SPI (não usado neste projeto)
- `Servo.h` - Controle de servo motor
- `Arduino.h` - Core Arduino

### **Bibliotecas Customizadas (locais em /lib)**

```
lib/
├── BNO055_FranRobots/         (Wrapper IMU customizado)
├── CalibrationMenu/            (Menu de calibração)
├── ColorCalibration/           (Calibração de cores)
├── Encoder/                    (Codificador com ISR)
├── LedStrip/                   (Controle de LEDs)
├── Motor/                      (Driver PWM individual)
├── OpenMVCamera/               (Interface câmera OpenMV)
├── ReflectancePlate/           (Sensor cor/reflectância)
├── Robot/                      (Controle de 4 motores)
├── RobotControl/               (Navegação + planejamento)
├── Scan360/                    (Processamento ToF)
├── ServoKit/                   (Controle servo para vítimas)
├── SwitchAvoidance/            (Detecção bumper)
├── TCA9548A/                   (Multiplexador I2C)
├── ToFCalibration/             (Calibração sensores ToF)
└── VL53Mux12_FRAN/            (Controlador 12 sensores ToF)
```

---

## 🔧 Troubleshooting

### **Problema: "Cannot find platformio"**
```bash
# Solução:
pip install platformio
pio --version
```

### **Problema: "Board not found"**
```bash
# Solução 1: Verificar conexão USB
# - Desconecte e reconecte a Teensy
# - Use outro cabo USB

# Solução 2: Instalar driver Teensy
# - Baixe em: https://www.pjrc.com/teensy/td_download.html

# Solução 3: Listar portas disponíveis
pio device list
```

### **Problema: "Compilation error"**
```bash
# Solução:
# 1. Limpar cache de build
pio run -e teensy40 --target=clean

# 2. Compilar novamente
pio run -e teensy40

# 3. Verificar mensagens de erro
# Se houver erro de biblioteca faltante:
pio lib install "nome-da-biblioteca"
```

### **Problema: "Upload failed"**
```bash
# Solução 1: Pressionar botão PROGRAM na Teensy durante upload
# Solução 2: Verificar driver USB instalado
# Solução 3: Tentar upload manual com Teensy Loader
```

### **Problema: "Serial Monitor não mostra nada"**
```bash
# Verificar:
# 1. Teensy conectada?
# 2. Código foi enviado com sucesso?
# 3. Baud rate é 115200?
# 4. Aguardar 3-5 segundos após reset (hardware inicializa)

# Tentar reset manual:
# - Pressione o botão RESET na Teensy
```

---

## ✅ Checklist de Verificação

- [ ] Python 3.x instalado e no PATH
- [ ] PlatformIO instalado (CLI ou VS Code)
- [ ] Teensy 4.0 conectada via USB
- [ ] Teensy Loader instalado
- [ ] Projeto clonado/disponível
- [ ] Dependências baixadas (`pio run --target=build`)
- [ ] Código compila sem erros
- [ ] Upload bem-sucedido
- [ ] Serial Monitor conectado (115200 baud)
- [ ] Mensagens aparecem no serial (esperado após 5s)

---

## 🎯 Próximos Passos Após Compilar

1. **Verificar Sensores**
   - Abra o Serial Monitor
   - Aguarde as mensagens de inicialização
   - Procure por "Sistema pronto."

2. **Testar Motores**
   - Levante o robô para as rodas não tocarem o chão
   - Dê comando de movimento via Serial
   - Verifique se as rodas giram

3. **Testar Sensores**
   - Aproxime a mão dos sensores ToF
   - Verifique mudanças no Serial Monitor
   - Teste os botões e bumpers

4. **Calibrar ToF** (Se necessário)
   - Pressione o botão BUTTON_PIN durante startup (8 segundos)
   - Siga as instruções de calibração

---

## 📝 Configurações Importantes em config.h

Se precisar ajustar algo, edite `include/config.h`:

```cpp
// I2C
constexpr uint32_t I2C_FREQUENCY = 400000;   // 400 kHz

// PWM Motors
constexpr uint16_t PWM_FREQUENCY = 30000;    // 30 kHz
constexpr uint8_t  PWM_RESOLUTION = 12;      // 12 bits (0-4095)

// ADC
constexpr uint8_t  ADC_RESOLUTION = 12;      // 12 bits (0-4095)

// Encoder
constexpr uint16_t ENCODER_PPR = 600;        // Pulsos por revolução

// Navegação
constexpr uint16_t TILE_SIZE_MM = 300;       // Tamanho do ladrilho
constexpr int32_t ENCODER_TICKS_PER_TILE = 860; // CALIBRAR COM SUA PISTA!

// Sensores
constexpr uint16_t TOF_WALL_CLEAR_MM = 120;  // Limiar: caminho livre
```

---

## 🚨 Notas Importantes

1. **Calibração de Encoder:**
   - O valor `ENCODER_TICKS_PER_TILE` (860) precisa ser calibrado com sua pista real
   - Meça uma distância conhecida e ajuste esse valor

2. **Câmeras OpenMV:**
   - Ainda estão comentadas no código
   - Se tiver câmeras, descomente as linhas em main.cpp:245-261

3. **Sensores I2C:**
   - Se houver problemas, use `lib/i2c_scanner/i2c_scanner.cpp` para debug
   - Compile com: `pio run -e teensy40 --target=upload`

4. **Backup EEPROM:**
   - Calibrações são salvas na EEPROM
   - Se resetar a Teensy, as calibrações podem ser perdidas

---

## 📚 Links Úteis

- **PlatformIO Docs:** https://docs.platformio.org/
- **Teensy Doc:** https://www.pjrc.com/teensy/
- **Adafruit BNO055:** https://github.com/adafruit/Adafruit_BNO055
- **Pololu VL53L0X:** https://github.com/pololu/vl53l0x-arduino
- **Teensy Board Defs:** https://github.com/PaulStoffregen/cores

---

**Última atualização:** 11/08/2026
**Projeto:** FranRobots - Maze Navigation Challenge
**Plataforma:** Teensy 4.0 (i.MX RT1062)
