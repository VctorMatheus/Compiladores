"""Gera imagens/papiro.jpg: textura procedural de papiro para o fundo da IDE.
Uso: python imagens/gerar_papiro.py   (requer Pillow e numpy)"""
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
from pathlib import Path

W, H = 1600, 1200
rng = np.random.default_rng(42)

def ruido(escala):
    """Ruído suave: grade aleatória pequena ampliada com interpolação bicúbica."""
    g = rng.random((max(2, H // escala), max(2, W // escala)))
    img = Image.fromarray((g * 255).astype(np.uint8)).resize((W, H), Image.BICUBIC)
    return np.asarray(img, np.float32) / 255.0

def fbm(escalas):
    total, peso = np.zeros((H, W), np.float32), 0.0
    for i, e in enumerate(escalas):
        a = 0.6 ** i
        total += ruido(e) * a
        peso += a
    return total / peso

def camada_fibras(qtd, horizontal, comp, larg, intensidade):
    """Desenha fibras longas e levemente onduladas, claras e escuras."""
    img = Image.new("L", (W, H), 128)
    d = ImageDraw.Draw(img)
    for _ in range(qtd):
        L = rng.uniform(*comp)
        if horizontal:
            x0, y0 = rng.uniform(-L, W), rng.uniform(0, H)
            pts = [(x0 + t * L, y0 + rng.normal(0, 0.8) + np.sin(t * 3) * rng.uniform(0, 2))
                   for t in np.linspace(0, 1, 6)]
        else:
            x0, y0 = rng.uniform(0, W), rng.uniform(-L, H)
            pts = [(x0 + rng.normal(0, 0.8), y0 + t * L) for t in np.linspace(0, 1, 6)]
        tom = int(128 + rng.choice([-1, 1]) * rng.uniform(10, intensidade))
        d.line(pts, fill=tom, width=int(rng.integers(larg[0], larg[1] + 1)))
    img = img.filter(ImageFilter.GaussianBlur(0.8))
    return (np.asarray(img, np.float32) - 128) / 128

fib_h = camada_fibras(9000, True, (80, 600), (1, 3), 45)
fib_v = camada_fibras(1800, False, (60, 400), (1, 2), 25)

# Tiras largas do papiro: faixas horizontais com tonalidades levemente diferentes
alturas = []
while sum(alturas) < H:
    alturas.append(int(rng.integers(35, 90)))
faixa = np.concatenate([np.full(a, rng.normal(0, 1)) for a in alturas])[:H]
faixas = np.asarray(Image.fromarray(((faixa - faixa.min()) / np.ptp(faixa) * 255)
                    .astype(np.uint8)[:, None].repeat(W, 1)).filter(ImageFilter.GaussianBlur(3)),
                    np.float32) / 255.0

manchas = fbm([500, 220, 90])
granulado = fbm([6, 2])

# Vinheta irregular (bordas envelhecidas/queimadas)
y, x = np.mgrid[0:H, 0:W].astype(np.float32)
dx, dy = (x / W - 0.5) * 2, (y / H - 0.5) * 2
d = np.maximum(np.abs(dx), np.abs(dy)) + (ruido(40) - 0.5) * 0.08
borda = np.clip((d - 0.84) / 0.16, 0, 1) ** 1.8 + (dx**2 + dy**2) * 0.06

lum = (0.80
       + (manchas - 0.5) * 0.34
       + fib_h * 0.16
       + fib_v * 0.07
       + (faixas - 0.5) * 0.07
       + (granulado - 0.5) * 0.06
       - borda * 0.60)
lum = np.clip(lum, 0, 1)[..., None]

escuro = np.array([88, 52, 20], np.float32)    # marrom queimado
claro = np.array([243, 224, 172], np.float32)  # creme dourado do papiro
rgb = escuro + (claro - escuro) * lum

saida = Path(__file__).with_name("papiro.jpg")
Image.fromarray(rgb.astype(np.uint8)).save(saida, quality=88)
print(f"Gerado: {saida} ({saida.stat().st_size // 1024} KB)")
