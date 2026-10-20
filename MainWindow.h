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

private:
    void preencherTabela(const std::vector<Simbolo> &simbolos);

    QPlainTextEdit *editor;     // Campo de texto
    QPushButton *botaoCompilar; // Botão Compilar
    QTabWidget *abas;           // Abas "Saída" e "Tabela de Símbolos"
    QPlainTextEdit *saida;      // Campo de resposta
    QTableWidget *tabela;       // Tabela de símbolos da última compilação
};

#endif
