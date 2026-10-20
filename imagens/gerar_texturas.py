"""Gera as texturas da IDE (sem emendas, para repetir lado a lado):
   imagens/marmore.jpg  - mármore branco com veios (molduras e colunas)
   imagens/tecido.jpg   - tecido vermelho (estandarte)
Uso: python imagens/gerar_texturas.py   (requer Pillow e numpy)"""
import numpy as np
from PIL import Image
from pathlib import Path

N = 512
rng = np.random.default_rng(3)
pasta = Path(__file__).parent

def ruido_periodico(escala):
    """Ruído suave e periódico (filtro gaussiano no domínio da frequência)."""
    f = np.fft.fftfreq(N)
    fx, fy = np.meshgrid(f, f)
    filtro = np.exp(-(fx**2 + fy**2) * (escala ** 2) * 20)
    r = np.real(np.fft.ifft2(np.fft.fft2(rng.normal(size=(N, N))) * filtro))
    return (r - r.min()) / np.ptp(r)

def salvar(rgb, nome):
    Image.fromarray(np.clip(rgb, 0, 255).astype(np.uint8)).save(pasta / nome, quality=90)
    print("Gerado:", pasta / nome)

# Mármore: veios diagonais deformados por turbulência
y, x = np.mgrid[0:N, 0:N] / N
turb = ruido_periodico(18) * 0.6 + ruido_periodico(6) * 0.4
veio = np.abs(np.sin(2 * np.pi * (2 * x + 1 * y) + turb * 9))
veio = 1 - veio ** 0.18                                  # veios finos
lum = 0.93 + (ruido_periodico(30) - 0.5) * 0.08 - veio * 0.30
base = np.array([238, 233, 222]); cinza = np.array([150, 140, 128])
salvar((cinza + (base - cinza) * np.clip(lum, 0, 1)[..., None]), "marmore.jpg")

# Tecido vermelho: trama fina + dobras suaves verticais
trama = (np.sin(2 * np.pi * x * 160) * np.sin(2 * np.pi * y * 160)) * 0.03
dobras = np.sin(2 * np.pi * x * 3 + ruido_periodico(25) * 3) * 0.10
lum = 0.75 + dobras + trama + (ruido_periodico(4) - 0.5) * 0.06
escuro = np.array([60, 6, 8]); claro = np.array([190, 28, 30])
salvar((escuro + (claro - escuro) * np.clip(lum, 0, 1)[..., None]), "tecido.jpg")
