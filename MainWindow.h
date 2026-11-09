#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <vector>

class QPlainTextEdit;
class QPushButton;
class QTableWidget;
class QTabWidget;
struct Simbolo;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void compilar();
    void salvarAssembly();

private:
    void preencherTabela(const std::vector<Simbolo> &simbolos);

    QPlainTextEdit *editor;     // Campo de texto
    QPushButton *botaoCompilar; // Botão Compilar
    QPushButton *botaoSalvar;   // Salva o assembly gerado (.asm) para abrir no Bipide
    QTabWidget *abas;           // Abas "Saída", "Tabela de Símbolos" e "Assembly"
    QPlainTextEdit *saida;      // Campo de resposta
    QTableWidget *tabela;       // Tabela de símbolos da última compilação
    QPlainTextEdit *assembly;   // Código gerado para o BIP na última compilação
};

#endif
