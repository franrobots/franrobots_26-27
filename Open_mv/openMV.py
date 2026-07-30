import sensor, time, pyb
from machine import LED

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.set_windowing((150, 320))
sensor.skip_frames(time=2000)
sensor.set_auto_whitebal(False)
sensor.set_auto_gain(False, gain_db=12)
sensor.set_hmirror(True)
sensor.set_vflip(True)
clock = time.clock()
r = 0
x = 0
y = 0

ledB = LED("LED_BLUE")
ledR = LED("LED_RED")

I2C_ADDR = 0x13

frames_sem_circulo = 0
confirmacao = 0

TOLERANCIA_XY = 3
TOLERANCIA_R = 4

historico_aneis = [[] for _ in range(5)]
TAMANHO_FILTRO = 9

vitima = 0
estado = 0
state_count = []

calibration_mode = False

CORES_REF = {
    "Vermelho": (45, 45, 20),
    "Amarelo":  (78, -15, 40),
    "Verde":    (56, -40, 30),
    "Azul":     (20, 20, -10),
    "Preto":    (10, 0, 0),
    "Branco":   (99, 0, 0)
}

CAL_ROI = (5, 5, 15, 15)


def calibrate_color(stats, margem=10):
    L_mean = stats.l_mean
    A_mean = stats.a_mean
    B_mean = stats.b_mean

    L_min = max(0, int(L_mean - margem))
    L_max = min(100, int(L_mean + margem))
    A_min = max(-128, int(A_mean - margem))
    A_max = min(127, int(A_mean + margem))
    B_min = max(-128, int(B_mean - margem))
    B_max = min(127, int(B_mean + margem))

    return (L_min, L_max, A_min, A_max, B_min, B_max)


def show_calibrate(img):
    roi = (140, 100, 40, 40)
    img.draw_rectangle(roi, color=(255, 255, 255))
    stats = img.get_statistics(roi=roi)
    threshold = calibrate_color(stats)

    print("\n=========== Threshold =============")
    print(threshold)
    print("=====================================")


def classificar_cor(stats1, stats2, stats3, stats4, l_fundo):
    L = (stats1.l_mean + stats2.l_mean + stats3.l_mean + stats4.l_mean)
    A = (stats1.a_mean + stats2.a_mean + stats3.a_mean + stats4.a_mean)
    B = (stats1.b_mean + stats2.b_mean + stats3.b_mean + stats4.b_mean)

    # print(f"L: {L} || A: {A} || {B}")

    if L < 42:
        return "Preto"

    if L > (L_fundo - 10) and abs(A) < 10 and abs(B) < 10:
        return "Branco"

    cor = "Indefinida"
    menor_distancia = 1000

    for nome, (rL, rA, rB) in CORES_REF.items():
        if nome in ["Branco", "Preto"]:
            continue

        dist = ((L - rL)**2 + (3.5 * (A - rA))**2 + (3.5 * (B - rB))**2)**0.5

        if dist < menor_distancia:
            menor_distancia = dist
            cor = nome
    return cor


def circulo():
    c = img.find_circles(threshold=6000, r_min=20, r_max=100, x_margin=50, y_margin=50, r_margin=50)

    if c:
        best = max(c, key=lambda c: c.magnitude)
        x = best.x
        y = best.y
        r = best.r
        espessura = int(r * 0.15)
        altura = int(r * 0.15)
        y_comum = int(y - (altura / 2))
        x_comum = int(x - (altura / 2))
        off_x = espessura // 2

        pontos = [0, 0.3, 0.55, 0.73, 0.95]
        cores = []

        for i, p in enumerate(pontos):
            # Cruz nos círculos
            xR_p = int(x + (r * p)) - off_x
            roixR = (xR_p, y_comum, espessura, altura)
            yR_p = int(y + (r * p)) - off_x
            roiyR = (x_comum, yR_p, espessura, altura)
            xL_p = int(x - (r * p)) + off_x
            roixL = (xL_p, y_comum, espessura, altura)
            yL_p = int(y - (r * p)) - off_x
            roiyL = (x_comum, yL_p, espessura, altura)

            if 0 <= xR_p < (240 - espessura):
                statsxR = img_color.get_statistics(roi=roixR)
                statsyR = img_color.get_statistics(roi=roiyR)
                statsxL = img_color.get_statistics(roi=roixL)
                statsyL = img_color.get_statistics(roi=roiyL)
                cor_atual = classificar_cor(statsxR, statsyR, statsxL, statsyL, L_fundo)

                historico_aneis[i].append(cor_atual)
                if len(historico_aneis[i]) > TAMANHO_FILTRO:
                    historico_aneis[i].pop(0)
                cor_votos = max(set(historico_aneis[i]), key=historico_aneis[i].count)
                cores.append(cor_votos)

                img_color.draw_rectangle(roixR, color=(255, 0, 0))
                img_color.draw_rectangle(roiyR, color=(255, 0, 0))
                img_color.draw_rectangle(roixL, color=(255, 0, 0))
                img_color.draw_rectangle(roiyL, color=(255, 0, 0))
        img_color.draw_circle((x, y, r), color=(255, 0, 0))
        return cores


bus = pyb.I2C(2, pyb.I2C.SLAVE, addr=I2C_ADDR)
bus.deinit()
bus = pyb.I2C(2, pyb.I2C.SLAVE, addr=I2C_ADDR)

buffer = bytearray([0, 0])  # [valor identificado, confiabilidade]
print("Esperando o ESP32...")


def enviar_i2c():
    try:
        cmd = bus.recv(1, timeout=100)
        if cmd and cmd[0] == 0x00:
            bus.send(buffer)
            print(f"[I2C] Enviado: {buffer[0]}, {buffer[1]}")
            time.sleep_ms(30)
            ledB.on()
        else:
            print(f"[I2C] Comando inválido: {cmd}")
    except Exception as e:
        pass


while True:
    clock.tick()
    img = sensor.snapshot()

    img_color = img.copy()
    if calibration_mode:
        show_calibrate(img_color)
        pyb.delay(500)
    else:
        stats_cal = img_color.get_statistics(roi=CAL_ROI)
        L_fundo = stats_cal.l_mean
        thres_geral = (0, int(L_fundo - 5), -128, 127, -128, 127)
        thres_amarelo = (50, 100, -128, 127, 10, 127)
        img.gaussian(1)
        img.binary([thres_geral, thres_amarelo], invert=False)
        img.erode(5)
        img.dilate(4)
        img_color.draw_rectangle(CAL_ROI, color=(0, 255, 0))
        cores = circulo()

        if cores:
            while not len(estado) > 99:
                for i in range(len(cores)):
                    if cores[i] == "Azul":
                        vitima += 2
                    elif cores[i] == "Verde":
                        vitima += 1
                    elif cores[i] == "Amarelo":
                        continue
                    elif cores[i] == "Vermelho":
                        vitima -= 1
                    else:
                        vitima -= 2

                if vitima == 2:
                    estado = "Harmed"
                elif vitima == 1:
                    estado = "Stable"
                elif vitima == 0:
                    estado = "Unharmed"
                else:
                    estado = "False"
                state_count.append(estado)
        else:
            pass
            # Verifica letras

    img.draw_image(img_color, 0, 0)
    print("FPS:", clock.fps())
