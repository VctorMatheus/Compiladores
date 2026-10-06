#include "MainWindow.h"
#include "Compilador.h"

#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QSplitter>
#include <QTextBlock>
#include <QTextCursor>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Roman++ IDE"));
    resize(900, 650);

    // Fontes do tema (embutidas em fontes/ e registradas no main.cpp)
    QFont fonteCodigo(QStringLiteral("Alegreya"), 13);
    QFont fonteSaida(QStringLiteral("Almendra"), 12);
    // Algarismos alinhados: evita confundir o zero com a letra "o"
    fonteCodigo.setFeature(QFont::Tag("lnum"), 1);
    fonteSaida.setFeature(QFont::Tag("lnum"), 1);
    QFont fonteTitulo(QStringLiteral("Cinzel"), 20, QFont::Black);
    QFont fonteSubtitulo(QStringLiteral("Almendra"), 11);
    fonteSubtitulo.setItalic(true);
    QFont fonteBotao(QStringLiteral("Cinzel"), 10);
    fonteBotao.setBold(true);
    QFont fonteRotulo(QStringLiteral("Marcellus SC"), 11);

    // Aspecto de papiro: imagem de fundo (imagens/papiro.jpg) e texto cor de tinta
    setStyleSheet(QStringLiteral(R"(
        #papiro {
            border-image: url(:/imagens/papiro.jpg) 0 0 0 0 stretch stretch;
        }
        QLabel { color: #4a2c12; background: transparent; }
        QPlainTextEdit {
            color: #3b2410;
            background: rgba(255, 248, 225, 60);
            border: 1px solid rgba(90, 55, 25, 110);
            border-radius: 4px;
            padding: 4px;
            selection-background-color: rgba(120, 70, 30, 120);
            selection-color: #fff8e6;
        }
        QPlainTextEdit[estado="sucesso"] { color: #2f5a1c; }
        QPlainTextEdit[estado="erro"]    { color: #8b1a1a; }
        QPushButton {
            color: #3b2410;
            background: rgba(255, 248, 225, 80);
            border: 1px solid #6b4423;
            border-radius: 4px;
            padding: 5px 14px;
        }
        QPushButton:hover   { background: rgba(255, 248, 225, 150); }
        QPushButton:pressed { background: rgba(90, 55, 25, 60); }
        QSplitter::handle { background: transparent; }
    )"));

    // Cabeçalho
    auto *titulo = new QLabel(QStringLiteral("Roman++"));
    titulo->setFont(fonteTitulo);
    auto *subtitulo = new QLabel(QStringLiteral("Ambiente de desenvolvimento"));
    subtitulo->setFont(fonteSubtitulo);

    // Campo de texto
    editor = new QPlainTextEdit;
    editor->setFont(fonteCodigo);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    editor->setTabStopDistance(4 * QFontMetricsF(fonteCodigo).horizontalAdvance(' '));
    editor->setPlaceholderText(QStringLiteral("Digite seu código aqui..."));
    QPalette paleta = editor->palette();
    paleta.setColor(QPalette::PlaceholderText, QColor(110, 80, 50));
    editor->setPalette(paleta);

    // Botão Compilar
    botaoCompilar = new QPushButton(QStringLiteral("Compilar (F5)"));
    botaoCompilar->setFont(fonteBotao);
    connect(botaoCompilar, &QPushButton::clicked, this, &MainWindow::compilar);
    auto *atalho = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(atalho, &QShortcut::activated, this, &MainWindow::compilar);

    // Campo de resposta
    saida = new QPlainTextEdit;
    saida->setFont(fonteSaida);
    saida->setReadOnly(true);

    auto *cabecalho = new QHBoxLayout;
    cabecalho->addWidget(titulo, 0, Qt::AlignBottom);
    cabecalho->addSpacing(8);
    cabecalho->addWidget(subtitulo, 0, Qt::AlignBottom);
    cabecalho->addStretch();
    cabecalho->addWidget(botaoCompilar, 0, Qt::AlignVCenter);

    auto *rotuloSaida = new QLabel(QStringLiteral("Saída"));
    rotuloSaida->setFont(fonteRotulo);

    auto *painelSaida = new QWidget;
    auto *layoutSaida = new QVBoxLayout(painelSaida);
    layoutSaida->setContentsMargins(0, 0, 0, 0);
    layoutSaida->addWidget(rotuloSaida);
    layoutSaida->addWidget(saida);

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(editor);
    splitter->addWidget(painelSaida);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    auto *central = new QWidget;
    central->setObjectName(QStringLiteral("papiro"));
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(36, 26, 36, 30);  // afasta o conteúdo das bordas queimadas
    layout->addLayout(cabecalho);
    layout->addWidget(splitter);
    setCentralWidget(central);
}

void MainWindow::compilar()
{
    Compilador compilador;
    const ResultadoCompilacao r = compilador.compilar(editor->toPlainText());

    saida->setProperty("estado", r.sucesso ? "sucesso" : "erro");
    saida->style()->unpolish(saida);
    saida->style()->polish(saida);
    saida->setPlainText(r.mensagem);

    // Em caso de erro, posiciona o cursor do editor no local indicado
    if (!r.sucesso && r.linha > 0) {
        QTextBlock bloco = editor->document()->findBlockByNumber(r.linha - 1);
        if (bloco.isValid()) {
            QTextCursor cursor(bloco);
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                                qMin(r.coluna - 1, bloco.length() - 1));
            editor->setTextCursor(cursor);
            editor->setFocus();
        }
    }
}
