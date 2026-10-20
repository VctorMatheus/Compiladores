#ifndef ESTANDARTE_H
#define ESTANDARTE_H

#include <QWidget>

// Painel lateral desenhado à mão: estandarte vermelho com recorte em "V" e
// borda de mármore, entre duas colunas jônicas (fuste canelado e volutas).
// Os widgets filhos (título, botão) ficam sobre o estandarte via layout.
class Estandarte : public QWidget
{
public:
    explicit Estandarte(QWidget *parent = nullptr);

    static constexpr int larguraColuna = 42;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void desenharColuna(QPainter &p, qreal x, qreal altura);
    void desenharVoluta(QPainter &p, QPointF centro, qreal raio, bool espelhar);
};

#endif
