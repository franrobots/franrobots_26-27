# ⚡ Quick Start - FranRobots Teensy

## 🚀 Iniciar em 5 Minutos

### **Opção 1: Automático (Recomendado)**

```bash
# 1. Abra PowerShell no diretório do projeto
cd c:\Users\Instrutor\Desktop\franrobots_26-27\Teensy_code

# 2. Execute o script (Windows PowerShell)
.\setup_and_build.ps1

# Ou execute o script batch:
setup_and_build.bat

# ✅ Pronto! Aguarde a compilação terminar
```

---

### **Opção 2: Manual Passo-a-Passo**

```bash
# Terminal / PowerShell / CMD

# 1. Navegar até o diretório
cd c:\Users\Instrutor\Desktop\franrobots_26-27\Teensy_code

# 2. Limpar builds anteriores
pio run -e teensy40 --target=clean

# 3. Baixar dependências + compilar
pio run -e teensy40

# 4. Se compilar com sucesso, você verá:
# ======================== [SUCCESS] Took X.XX seconds ========================
```

---

### **Opção 3: Usar VS Code**

1. Abra VS Code
2. Abra a pasta do projeto
3. Instale extensão **PlatformIO** (se não tiver)
4. Na barra inferior, clique no ícone PlatformIO → **Build**
5. Aguarde a compilação

---

## 📱 Fazer Upload para a Teensy

### **Após a compilação bem-sucedida:**

```bash
# Terminal / PowerShell / CMD

# CONECTE A TEENSY AO COMPUTADOR!

# 1. Upload automático
pio run -e teensy40 --target=upload

# 2. Se pedir, pressione o botão PROGRAM na Teensy
# Espere a mensagem: "Programming complete"
```

---

## 🔍 Monitorar Saída Serial

```bash
# Terminal / PowerShell / CMD

# Conectar ao monitor serial (após upload bem-sucedido)
pio device monitor -b 115200

# Você deve ver mensagens como:
# Sistema iniciando...
# Aviso: BNO055 nao respondeu.
# Erro ToF
# Sistema pronto.
# Deus abençoe o round.

# Pressione Ctrl+C para desconectar
```

---

## 📋 Checklist

- [ ] Python 3.x instalado
- [ ] PlatformIO instalado (`pip install platformio`)
- [ ] Teensy 4.0 conectada via USB
- [ ] Arquivo `platformio.ini` existe
- [ ] Compilação bem-sucedida (sem erros)
- [ ] Upload concluído
- [ ] Serial Monitor mostra mensagens
- [ ] Botões e sensores respondem

---

## ❌ Erros Comuns

| Erro | Solução |
|------|---------|
| `Cannot find platformio` | `pip install platformio` |
| `Board not found` | Reconectar USB, reinstalar driver Teensy |
| `Compilation error` | `pio run -e teensy40 --target=clean` |
| `Upload failed` | Pressionar botão PROGRAM na Teensy |
| `Serial Monitor vazio` | Aguardar 5s, verificar baud rate (115200) |

---

## 📚 Documentos Complementares

- **Setup Completo:** [SETUP_TEENSY.md](SETUP_TEENSY.md)
- **Análise do Código:** [analise_completa.html](../../analise_completa.html)
- **Configuração de Pinos:** [config.h](../include/config.h)

---

## 🎯 Estrutura de Diretórios

```
franrobots_26-27/Teensy_code/
├── src/
│   └── main.cpp              (arquivo principal)
├── include/
│   └── config.h              (configuração de pinos)
├── lib/                       (bibliotecas customizadas)
│   ├── Robot/
│   ├── RobotControl/
│   ├── Motor/
│   ├── VL53Mux12_FRAN/
│   └── ... (outras bibliotecas)
├── platformio.ini            (configuração PlatformIO)
├── setup_and_build.ps1       (script PowerShell)
├── setup_and_build.bat       (script Batch)
└── .pio/build/teensy40/
    └── firmware.hex          (arquivo gerado após build)
```

---

## 💡 Dicas

1. **Primeira vez?**
   - Deixe o script automatizado fazer o trabalho
   - Acompanhe as mensagens de status

2. **Muitas compilações?**
   - Use VS Code + PlatformIO Extension (mais rápido)
   - Código auto-complete e debugging integrado

3. **Problemas persistentes?**
   - Desconecte/reconecte a Teensy
   - Reinicie o computador
   - Reinstale driver Teensy

4. **Debug avançado?**
   - Abra `Serial Monitor` enquanto usa o robô
   - Monitore valores de sensores em tempo real
   - Veja logs de navegação (DFS/BFS)

---

**✅ Pronto! Seu robô está pronto para explorar o labirinto!**
