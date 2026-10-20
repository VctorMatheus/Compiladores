#include "Estandarte.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRadialGradient>
#include <QtMath>

namespace {
const QColor kSombraMarmore(120, 108, 88);
const QColor kDourado(201, 162, 74);
constexpr qreal kAlturaCapitel = 28;
constexpr qreal kAlturaBase = 22;
constexpr qreal kProfundidadeV = 46;   // profundidade do recorte em "V"
constexpr qreal kBordaMarmore = 9;

QBrush marmore() { return QBrush(QPixmap(QStringLiteral(":/imagens/marmore.jpg"))); }
}

Estandarte::Estandarte(QWidget *parent) : QWidget(parent)
{
    // Margens internas: conteúdo fica dentro do tecido, longe das colunas e do "V"
    setContentsMargins(larguraColuna + 30, 30, larguraColuna + 30, int(kProfundidadeV) + 50);
}

void Estandarte::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal w = width(), h = height();

    // --- Estandarte: tecido com recorte em "V" na ponta ---
    const qreal esq = larguraColuna + 8, dir = w - larguraColuna - 8;
    const qreal fundo = h - kAlturaBase - 18;
    const qreal meio = (esq + dir) / 2;
    QPainterPath bandeira;
    bandeira.moveTo(esq, 0);
    bandeira.lineTo(dir, 0);
    bandeira.lineTo(dir, fundo);
    bandeira.lineTo(meio, fundo - kProfundidadeV);
    bandeira.lineTo(esq, fundo);
    bandeira.closeSubpath();

    p.fillPath(bandeira, QBrush(QPixmap(QStringLiteral(":/imagens/tecido.jpg"))));

    // Dobras do tecido: faixas verticais de luz e sombra
    QLinearGradient dobras(esq, 0, dir, 0);
    for (int i = 0; i <= 6; ++i)
        dobras.setColorAt(i / 6.0, i % 2 ? QColor(255, 255, 255, 22) : QColor(0, 0, 0, 55));
    p.fillPath(bandeira, dobras);

    // Borda de mármore com filete dourado por dentro
    p.setPen(QPen(marmore(), kBordaMarmore, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    p.drawPath(bandeira);
    p.setPen(QPen(kSombraMarmore, 1));
    p.drawPath(bandeira);

    const qreal in = kBordaMarmore / 2 + 4;
    QPainterPath filete;
    filete.moveTo(esq + in, 0);
    filete.lineTo(esq + in, fundo - in * 0.6);
    filete.lineTo(meio, fundo - kProfundidadeV - in * 1.2);
    filete.lineTo(dir - in, fundo - in * 0.6);
    filete.lineTo(dir - in, 0);
    p.setPen(QPen(kDourado, 1.6));
    p.drawPath(filete);

    // --- Colunas jônicas dos dois lados ---
    desenharColuna(p, 0, h);
    desenharColuna(p, w - larguraColuna, h);
}

void Estandarte::desenharColuna(QPainter &p, qreal x, qreal altura)
{
    const qreal lc = larguraColuna;
    const qreal topoFuste = kAlturaCapitel, baseFuste = altura - kAlturaBase;
    const QRectF fuste(x + 4, topoFuste, lc - 8, baseFuste - topoFuste);

    // Fuste: mármore + sombreamento cilíndrico
    p.fillRect(fuste, marmore());
    QLinearGradient cilindro(fuste.left(), 0, fuste.right(), 0);
    cilindro.setColorAt(0.0, QColor(0, 0, 0, 70));
    cilindro.setColorAt(0.35, QColor(255, 255, 255, 40));
    cilindro.setColorAt(1.0, QColor(0, 0, 0, 85));
    p.fillRect(fuste, cilindro);

    // Caneluras: sulcos verticais (sombra à esquerda, luz à direita)
    const int caneluras = 5;
    const qreal passo = fuste.width() / caneluras;
    p.setPen(Qt::NoPen);
    for (int i = 0; i < caneluras; ++i) {
        const qreal cx = fuste.left() + passo * (i + 0.5);
        const QRectF sulco(cx - passo * 0.28, fuste.top() + 4, passo * 0.56, fuste.height() - 8);
        QLinearGradient g(sulco.left(), 0, sulco.right(), 0);
        g.setColorAt(0.0, QColor(60, 50, 40, 110));
        g.setColorAt(0.6, QColor(60, 50, 40, 30));
        g.setColorAt(1.0, QColor(255, 255, 255, 70));
        p.setBrush(g);
        p.drawRoundedRect(sulco, passo * 0.28, passo * 0.28);
    }
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(kSombraMarmore, 1));
    p.drawRect(fuste);

    // Capitel jônico: ábaco (placa), equino (faixa) e duas volutas
    const QRectF abaco(x - 2, 0, lc + 4, 7);
    const QRectF equino(x + 1, 7, lc - 2, 10);
    p.fillRect(abaco, marmore());
    p.drawRect(abaco);
    p.fillRect(equino, marmore());
    p.drawRect(equino);
    const qreal raio = 11.5;
    desenharVoluta(p, QPointF(x + raio - 3, 7 + raio + 2), raio, false);
    desenharVoluta(p, QPointF(x + lc - raio + 3, 7 + raio + 2), raio, true);

    // Base ática: dois toros (molduras arredondadas) e plinto
    p.setPen(QPen(kSombraMarmore, 1));
    p.setBrush(marmore());
    p.drawRoundedRect(QRectF(x + 1, baseFuste, lc - 2, 7), 3.5, 3.5);
    p.drawRoundedRect(QRectF(x - 1, baseFuste + 7, lc + 2, 8), 4, 4);
    p.drawRect(QRectF(x - 2, baseFuste + 15, lc + 4, kAlturaBase - 15));
    p.setBrush(Qt::NoBrush);
}

void Estandarte::desenharVoluta(QPainter &p, QPointF c, qreal raio, bool espelhar)
{
    // Disco de mármore com leve relevo
    QRadialGradient relevo(c - QPointF(raio * 0.3, raio * 0.3), raio * 1.3);
    relevo.setColorAt(0, QColor(255, 255, 255, 60));
    relevo.setColorAt(1, QColor(0, 0, 0, 70));
    p.setPen(QPen(kSombraMarmore, 1));
    p.setBrush(marmore());
    p.drawEllipse(c, raio, raio);
    p.setBrush(relevo);
    p.drawEllipse(c, raio, raio);

    // Espiral (2,5 voltas) fechando no "olho" da voluta
    QPainterPath espiral;
    const qreal fim = 2.5 * 2 * M_PI;
    for (qreal t = 0; t <= fim; t += 0.08) {
        const qreal r = raio * 0.80 * (1 - t / fim) + raio * 0.12;
        const qreal a = espelhar ? M_PI - t : t;
        const QPointF pt = c + QPointF(r * qCos(a), r * qSin(a));
        if (t == 0)
            espiral.moveTo(pt);
        else
            espiral.lineTo(pt);
    }
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(95, 82, 64), 1.5, Qt::SolidLine, Qt::RoundCap));
    p.drawPath(espiral);
    p.setBrush(QColor(95, 82, 64));
    p.drawEllipse(c, raio * 0.14, raio * 0.14);
    p.setBrush(Qt::NoBrush);
}
