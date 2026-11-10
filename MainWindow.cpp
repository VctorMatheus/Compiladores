#include "MainWindow.h"
#include "CatalogoTokens.h"
#include "Compilador.h"
#include "Estandarte.h"
#include "TabelaSimbolos.h"

#include <QFile>
#include <QFileDialog>
#include <QFontMetricsF>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QTabWidget>
#include <QTableWidget>
#include <QSplitter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Roman++ IDE"));
    resize(1100, 760);

    // Fontes do tema (embutidas em fontes/ e registradas no main.cpp)
    // Editor e mensagens em tamanho 14 (requisito do T2)
    QFont fonteCodigo(QStringLiteral("Alegreya"), 14);
    QFont fonteSaida(QStringLiteral("Almendra"), 14);
    QFont fonteTabela(QStringLiteral("Alegreya"), 12);
    fonteTabela.setFeature(QFont::Tag("lnum"), 1);
    // Algarismos alinhados: evita confundir o zero com a letra "o"
    fonteCodigo.setFeature(QFont::Tag("lnum"), 1);
    fonteSaida.setFeature(QFont::Tag("lnum"), 1);
    QFont fonteTitulo(QStringLiteral("Cinzel"), 24, QFont::Black);
    QFont fonteSubtitulo(QStringLiteral("Almendra"), 11);
    fonteSubtitulo.setItalic(true);
    QFont fonteBotao(QStringLiteral("Cinzel"), 11);
    fonteBotao.setBold(true);
    QFont fonteRotulo(QStringLiteral("Marcellus SC"), 12);

    // Estética romana: estandarte vermelho entre colunas de mármore,
    // seções emolduradas em mármore e texto em papiro (texturas em imagens/)
    setStyleSheet(QStringLiteral(R"(
        #fundo {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #2a0607, stop:0.5 #4a0c0e, stop:1 #2a0607);
        }
        .Moldura {
            background-image: url(:/imagens/marmore.jpg);
            border: 1px solid #8a7a5c;
        }
        #titulo    { color: #e9c46a; background: transparent; }
        #subtitulo { color: #f3e3c0; background: transparent; }
        #divisor   { background: #c9a24a; max-height: 1px; min-height: 1px; }
        #rotulo    { color: #5a3a1a; background: transparent; }

        QPushButton {
            color: #f6e7c1;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #8a6a2c, stop:0.5 #6b4f1e, stop:1 #4e3812);
            border: 2px solid #d4b062;
            border-radius: 2px;
            padding: 8px 18px;
        }
        QPushButton:hover   { background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #a5813a, stop:0.5 #826027, stop:1 #5e4417); }
        QPushButton:pressed { background: #4e3812; }
        QPushButton:disabled { color: #b9a77e; background: #5a4a2a; border-color: #8a7a5c; }

        QPlainTextEdit {
            color: #3b2410;
            background-image: url(:/imagens/papiro.jpg);
            background-position: center;
            border: 1px solid #8a6a3a;
            padding: 6px;
            selection-background-color: rgba(120, 70, 30, 120);
            selection-color: #fff8e6;
        }
        QPlainTextEdit[estado="sucesso"] { color: #2f5a1c; }
        QPlainTextEdit[estado="aviso"]   { color: #7a4a06; }
        QPlainTextEdit[estado="erro"]    { color: #8b1a1a; }
        QSplitter::handle { background: transparent; }

        QTabWidget::pane { border: none; }
        QTabBar::tab {
            font-family: "Marcellus SC";
            font-size: 12pt;
            color: #6b4a22;
            background: transparent;
            padding: 2px 14px 4px 2px;
            border-bottom: 2px solid transparent;
        }
        QTabBar::tab:selected { color: #3b2410; border-bottom: 2px solid #b8862b; }
        QTabBar::tab:hover    { color: #7a4a06; }

        QTableWidget {
            color: #3b2410;
            background-image: url(:/imagens/papiro.jpg);
            background-position: center;
            border: 1px solid #8a6a3a;
            gridline-color: rgba(120, 80, 40, 90);
            alternate-background-color: rgba(120, 80, 40, 28);
            selection-background-color: rgba(120, 70, 30, 120);
            selection-color: #fff8e6;
        }
        QHeaderView::section {
            color: #f6e7c1;
            background: #6b4f1e;
            border: none;
            border-right: 1px solid #8a6a3a;
            padding: 4px 6px;
        }
        QTableCornerButton::section { background: #6b4f1e; border: none; }
    )"));

    // --- Estandarte (esquerda): título e botão Compilar ---
    auto *titulo = new QLabel(QStringLiteral("Roman++"));
    titulo->setObjectName(QStringLiteral("titulo"));
    titulo->setFont(fonteTitulo);
    titulo->setAlignment(Qt::AlignCenter);
    auto *subtitulo = new QLabel(QStringLiteral("Ambiente de desenvolvimento"));
    subtitulo->setObjectName(QStringLiteral("subtitulo"));
    subtitulo->setFont(fonteSubtitulo);
    subtitulo->setAlignment(Qt::AlignCenter);
    auto *divisor = new QFrame;
    divisor->setObjectName(QStringLiteral("divisor"));

    botaoCompilar = new QPushButton(QStringLiteral("Compilar (F5)"));
    botaoCompilar->setFont(fonteBotao);
    botaoCompilar->setCursor(Qt::PointingHandCursor);
    connect(botaoCompilar, &QPushButton::clicked, this, &MainWindow::compilar);
    auto *atalho = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(atalho, &QShortcut::activated, this, &MainWindow::compilar);

    botaoSalvar = new QPushButton(QStringLiteral("Salvar .asm"));
    botaoSalvar->setFont(fonteBotao);
    botaoSalvar->setCursor(Qt::PointingHandCursor);
    botaoSalvar->setEnabled(false);
    botaoSalvar->setToolTip(QStringLiteral("Salva o assembly gerado para abrir no Bipide"));
    connect(botaoSalvar, &QPushButton::clicked, this, &MainWindow::salvarAssembly);

    auto *colunas = new Estandarte;  // estandarte entre colunas jônicas
    colunas->setFixedWidth(316);
    auto *layoutEstandarte = new QVBoxLayout(colunas);
    layoutEstandarte->setContentsMargins(0, 0, 0, 0);
    layoutEstandarte->setSpacing(10);
    layoutEstandarte->addWidget(titulo);
    layoutEstandarte->addWidget(subtitulo);
    layoutEstandarte->addSpacing(10);
    layoutEstandarte->addWidget(divisor);
    layoutEstandarte->addSpacing(18);
    layoutEstandarte->addWidget(botaoCompilar);
    layoutEstandarte->addSpacing(8);
    layoutEstandarte->addWidget(botaoSalvar);
    layoutEstandarte->addStretch();

    // --- Seções (direita): código e saída, emolduradas em mármore ---
    editor = new QPlainTextEdit;
    editor->setFont(fonteCodigo);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    editor->setTabStopDistance(4 * QFontMetricsF(fonteCodigo).horizontalAdvance(' '));
    editor->setPlaceholderText(QStringLiteral("Digite seu código aqui..."));
    QPalette paleta = editor->palette();
    paleta.setColor(QPalette::PlaceholderText, QColor(110, 80, 50));
    editor->setPalette(paleta);

    saida = new QPlainTextEdit;
    saida->setFont(fonteSaida);
    saida->setReadOnly(true);

    auto criarMoldura = [](QWidget *conteudo, QLabel *rotulo = nullptr) {
        auto *moldura = new QFrame;
        moldura->setProperty("class", "Moldura");
        auto *l = new QVBoxLayout(moldura);
        l->setContentsMargins(14, rotulo ? 6 : 14, 14, 14);
        l->setSpacing(4);
        if (rotulo)
            l->addWidget(rotulo);
        l->addWidget(conteudo);
        return moldura;
    };

    // Tabela de símbolos (requisito do T3): preenchida a cada compilação
    tabela = new QTableWidget(0, 8);
    tabela->setFont(fonteTabela);
    tabela->setHorizontalHeaderLabels({
        QStringLiteral("Nome"), QStringLiteral("Tipo"), QStringLiteral("Modalidade"),
        QStringLiteral("Escopo"), QStringLiteral("Inic."), QStringLiteral("Usado"),
        QStringLiteral("Parâm."), QStringLiteral("Vetor") });
    tabela->horizontalHeaderItem(4)->setToolTip(QStringLiteral("Inicializado"));
    tabela->horizontalHeaderItem(6)->setToolTip(QStringLiteral("Posição do parâmetro na função"));
    tabela->horizontalHeaderItem(7)->setToolTip(QStringLiteral("Tamanho do vetor ([ ] = parâmetro vetor)"));
    tabela->horizontalHeader()->setFont(fonteRotulo);
    tabela->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tabela->horizontalHeader()->setStretchLastSection(true);
    tabela->verticalHeader()->setVisible(false);
    tabela->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabela->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabela->setAlternatingRowColors(true);

    // Tokens da linguagem: referência fixa, preenchida uma vez a partir do CatalogoTokens
    tabelaTokens = new QTableWidget(0, 5);
    tabelaTokens->setFont(fonteTabela);
    tabelaTokens->setHorizontalHeaderLabels({
        QStringLiteral("Código"), QStringLiteral("Token"), QStringLiteral("Lexema / Padrão"),
        QStringLiteral("Categoria"), QStringLiteral("Significado") });
    tabelaTokens->horizontalHeaderItem(0)->setToolTip(QStringLiteral("Número do token gerado pelo GALS"));
    tabelaTokens->horizontalHeaderItem(1)->setToolTip(QStringLiteral("Nome do token no Roman++.vgls"));
    tabelaTokens->horizontalHeader()->setFont(fonteRotulo);
    tabelaTokens->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tabelaTokens->horizontalHeader()->setStretchLastSection(true);
    tabelaTokens->verticalHeader()->setVisible(false);
    // Com a janela estreita, o significado quebra em mais linhas em vez de ser cortado
    tabelaTokens->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tabelaTokens->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabelaTokens->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabelaTokens->setAlternatingRowColors(true);
    preencherTokens();

    abas = new QTabWidget;
    abas->tabBar()->setFont(fonteRotulo);
    abas->tabBar()->setCursor(Qt::PointingHandCursor);
    abas->addTab(saida, QStringLiteral("Saída"));
    abas->addTab(tabela, QStringLiteral("Tabela de Símbolos"));
    abas->addTab(tabelaTokens, QStringLiteral("Tokens"));

    // Assembly do BIP (requisito do T4): fonte de largura fixa para alinhar as instruções
    assembly = new QPlainTextEdit;
    QFont fonteAssembly(QStringLiteral("Consolas"), 13);
    fonteAssembly.setStyleHint(QFont::Monospace);
    assembly->setFont(fonteAssembly);
    assembly->setReadOnly(true);
    assembly->setLineWrapMode(QPlainTextEdit::NoWrap);
    assembly->setPlaceholderText(QStringLiteral("Compile o programa para ver o assembly do BIP."));
    abas->addTab(assembly, QStringLiteral("Assembly"));

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setHandleWidth(12);
    splitter->addWidget(criarMoldura(editor));
    splitter->addWidget(criarMoldura(abas));
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    auto *central = new QWidget;
    central->setObjectName(QStringLiteral("fundo"));
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(24, 0, 20, 20);
    layout->setSpacing(24);
    layout->addWidget(colunas);
    auto *direita = new QVBoxLayout;
    direita->setContentsMargins(0, 20, 0, 0);
    direita->addWidget(splitter);
    layout->addLayout(direita, 1);
    setCentralWidget(central);
}

void MainWindow::compilar()
{
    Compilador compilador;
    const ResultadoCompilacao r = compilador.compilar(editor->toPlainText());

    saida->setProperty("estado", !r.sucesso ? "erro" : r.avisos > 0 ? "aviso" : "sucesso");
    saida->style()->unpolish(saida);
    saida->style()->polish(saida);
    saida->setPlainText(r.mensagem);
    preencherTabela(r.simbolos);

    if (!r.assembly.isEmpty())
        assembly->setPlainText(r.assembly);
    else if (!r.semAssembly.isEmpty())
        assembly->setPlainText(QStringLiteral("Código assembly não gerado.\n\n") + r.semAssembly);
    else
        assembly->setPlainText(QStringLiteral("Código assembly não gerado: o programa tem erros."));
    botaoSalvar->setEnabled(!r.assembly.isEmpty());

    // Em caso de erro, posiciona o cursor do editor no local indicado
    if (!r.sucesso)
        abas->setCurrentWidget(saida);
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

void MainWindow::salvarAssembly()
{
    const QString caminho = QFileDialog::getSaveFileName(
        this, QStringLiteral("Salvar assembly"), QStringLiteral("programa.asm"),
        QStringLiteral("Assembly do BIP (*.asm);;Todos os arquivos (*)"));
    if (caminho.isEmpty())
        return;

    QFile arquivo(caminho);
    if (!arquivo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, QStringLiteral("Salvar assembly"),
                             QStringLiteral("Não foi possível salvar em %1").arg(caminho));
        return;
    }
    QTextStream(&arquivo) << assembly->toPlainText();
}

void MainWindow::preencherTabela(const std::vector<Simbolo> &simbolos)
{
    auto simNao = [](bool v) { return v ? QStringLiteral("Sim") : QStringLiteral("Não"); };
    const QString nada = QStringLiteral("-");

    tabela->setRowCount(static_cast<int>(simbolos.size()));
    for (int i = 0; i < static_cast<int>(simbolos.size()); ++i) {
        const Simbolo &s = simbolos[i];
        const bool rotina = s.categoria == Categoria::Funcao || s.categoria == Categoria::Programa;

        QString tamanho = nada;
        if (s.vetor)
            tamanho = s.tamanho > 0 ? QString::number(s.tamanho) : QStringLiteral("[ ]");

        const QStringList colunas = {
            QString::fromStdString(s.nome),
            s.categoria == Categoria::Programa ? nada : QString::fromStdString(nomeTipo(s.tipo)),
            QString::fromStdString(modalidade(s)),
            QString::fromStdString(s.escopo),
            rotina ? nada : simNao(s.inicializado),
            s.categoria == Categoria::Programa ? nada : simNao(s.usado),
            s.posicaoParametro > 0 ? QString::number(s.posicaoParametro) : nada,
            tamanho,
        };
        for (int c = 0; c < colunas.size(); ++c)
            tabela->setItem(i, c, new QTableWidgetItem(colunas[c]));
    }
}

void MainWindow::preencherTokens()
{
    const QString nada = QStringLiteral("-");
    const std::vector<InfoToken> &tokens = catalogoTokens();
    QFont fonteItalico = tabelaTokens->font();
    fonteItalico.setItalic(true);

    tabelaTokens->setRowCount(static_cast<int>(tokens.size()));
    for (int i = 0; i < static_cast<int>(tokens.size()); ++i) {
        const InfoToken &t = tokens[i];
        const bool descartado = t.id == EPSILON;

        QString significado = QString::fromUtf8(t.significado);
        if (t.exemplo && !descartado)
            significado += QStringLiteral("; ex.: %1").arg(QString::fromUtf8(t.exemplo));
        if (t.semUso)
            significado += QStringLiteral(" (reservado, ainda sem uso na gramática)");

        const QStringList colunas = {
            descartado ? nada : QString::number(t.id),
            descartado ? nada : QString::fromUtf8(t.nome),
            QString::fromUtf8(t.lexema),
            QString::fromUtf8(t.categoria),
            significado,
        };
        for (int c = 0; c < colunas.size(); ++c) {
            auto *item = new QTableWidgetItem(colunas[c]);
            if (c == 0)
                item->setTextAlignment(Qt::AlignCenter);
            if (t.semUso || descartado) {
                item->setFont(fonteItalico);
                item->setToolTip(descartado
                    ? QStringLiteral("Descartado pelo léxico: não chega ao sintático")
                    : QStringLiteral("O léxico reconhece, mas nenhuma regra da gramática aceita (erro sintático)"));
            }
            tabelaTokens->setItem(i, c, item);
        }
    }
}
