#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QPlainTextEdit;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void compilar();

private:
    QPlainTextEdit *editor;     // Campo de texto
    QPushButton *botaoCompilar; // Botão Compilar
    QPlainTextEdit *saida;      // Campo de resposta
};

#endif
