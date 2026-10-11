#!/usr/bin/env python3
"""
analizar_captura.py - analiza una captura CSV de Digilent WaveForms (bus de 5 bits)
con las lineas de control de un bus 8080: RST, RD, WR, CS, RS.

Uso:
    python3 analizar_captura.py default.csv
    python3 analizar_captura.py captura_pi.csv --rst 0 --rd 1 --wr 2 --cs 3 --rs 4

Por defecto: bit0=RST, bit1=RD, bit2=WR, bit3=CS, bit4=RS (DIO0..DIO4).
Imprime los mismos numeros para el micro que funciona y para la Raspberry Pi,
asi se pueden comparar lado a lado.
"""
import argparse
import re
import statistics as st


def leer(path):
    rate = None
    vals = []
    with open(path) as f:
        for line in f:
            if line.startswith('#'):
                m = re.match(r'#Sample rate:\s*([0-9.eE+-]+)', line)
                if m:
                    rate = float(m.group(1))
                continue
            if line.startswith('Time') or not line.strip():
                continue
            vals.append(int(line.strip().split(',')[1].lstrip('hH'), 16))
    return vals, rate


def stats(xs):
    xs = list(xs)
    if not xs:
        return "sin datos"
    return f"min {min(xs):.0f} / media {st.mean(xs):.0f} / max {max(xs):.0f} ns"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('csv')
    for n, d in (('rst', 0), ('rd', 1), ('wr', 2), ('cs', 3), ('rs', 4)):
        ap.add_argument('--' + n, type=int, default=d, help=f'bit de {n.upper()} (por defecto {d})')
    a = ap.parse_args()

    V, rate = leer(a.csv)
    dt = 1e9 / rate if rate else 10.0
    N = len(V)
    bit = lambda v, b: (v >> b) & 1
    WR = lambda v: bit(v, a.wr)
    CS = lambda v: bit(v, a.cs)
    RS = lambda v: bit(v, a.rs)

    print(f"Archivo: {a.csv}")
    print(f"Muestras: {N} | paso: {dt:.1f} ns | ventana: {N * dt / 1000:.1f} us")
    print(f"RST siempre alto: {all(bit(v, a.rst) for v in V)} | RD siempre alto: {all(bit(v, a.rd) for v in V)}")

    # --- ciclos de escritura: un pulso de WR en bajo = un ciclo ---
    ciclos = []
    i = 0
    while i < N:
        if WR(V[i]) == 0:
            j = i
            while j < N and WR(V[j]) == 0:
                j += 1
            lo = i
            while lo > 0 and CS(V[lo - 1]) == 0:
                lo -= 1
            hi = j
            while hi < N and CS(V[hi]) == 0:
                hi += 1
            ciclos.append(dict(ini=i, wr=(j - i) * dt, rs=RS(V[i]), cs=(hi - lo) * dt,
                               cs_antes=(i - lo) * dt, cs_despues=(hi - j) * dt,
                               cs_fijo=(lo == 0 or hi >= N)))
            i = j
        else:
            i += 1

    if not ciclos:
        print("No se encontraron pulsos de WR en bajo.")
        return

    # CS que nunca cambia = CS fijo en bajo durante toda la captura (rafaga)
    cs_cambia = len({CS(v) for v in V}) > 1
    rs_cambia = len({RS(v) for v in V}) > 1
    print(f"CS cambia durante la captura: {cs_cambia} | RS cambia: {rs_cambia}")
    if not cs_cambia:
        print(f"  -> CS fijo en {'ALTO' if CS(V[0]) else 'BAJO'} toda la captura")
    if not rs_cambia:
        print(f"  -> RS fijo en {'ALTO (datos)' if RS(V[0]) else 'BAJO (comandos)'} toda la captura")

    cmd = [c for c in ciclos if c['rs'] == 0]
    dat = [c for c in ciclos if c['rs'] == 1]
    print(f"\nPulsos de WR: {len(ciclos)} | comandos (RS bajo): {len(cmd)} | datos (RS alto): {len(dat)}")

    for nombre, g in (("DATOS", dat), ("COMANDOS", cmd)):
        print(f"\n{nombre}:")
        print("  ancho de WR en bajo        :", stats(c['wr'] for c in g))
        if cs_cambia:
            g2 = [c for c in g if not c['cs_fijo']]
            print("  CS en bajo (total)         :", stats(c['cs'] for c in g2))
            print("  CS baja antes de WR baje   :", stats(c['cs_antes'] for c in g2))
            print("  WR sube antes de CS suba   :", stats(c['cs_despues'] for c in g2))

    per = [(ciclos[k + 1]['ini'] - ciclos[k]['ini']) * dt
           for k in range(len(ciclos) - 1) if ciclos[k]['rs'] == 1 and ciclos[k + 1]['rs'] == 1]
    print("\nPeriodo entre datos consecutivos:", stats(per))
    if per:
        print(f"  = {1e3 / st.mean(per):.2f} MHz de escritura")

    # --- estructura comando + datos ---
    seq = ''.join('C' if c['rs'] == 0 else 'D' for c in ciclos)
    grupos = [len(m.group(0)) - 1 for m in re.finditer(r'CD*', seq)]
    print("\nDatos que siguen a cada comando (primeros 30 grupos):", grupos[:30])
    if len(grupos) > 3 and set(grupos[:-1]) <= {1, 4}:  # el ultimo grupo puede quedar cortado
        print("  Patron 4,4,1 = 0x2A + 4 parametros, 0x2B + 4 parametros, 0x2C + 1 pixel")


if __name__ == '__main__':
    main()
