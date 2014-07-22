#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void newProject();
    void loadProject();

private:

    void connectActions();

    Ui::MainWindow *ui;
    QString m_baseProjectDir;
};

#endif // MAINWINDOW_H
