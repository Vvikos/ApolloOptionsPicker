#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <optionset.h>
#include <dashboard.h>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <toolbox.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Dashboard *dashboard;
    OptionSet *inputRow;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setInfoText(QString s);
    void permutRows(int i1, int i2);

private:
    Ui::MainWindow *ui;
    QString lastSavePath;
    QString lastOpenPath;

private slots:
    void open();
    void close();
    void close(int rank);
    void save();
    void saveAs();
    void exportCsv();
    void switchMode(bool val);
    void about();
    void updateFileButtons(int rank);
};

#endif // MAINWINDOW_H
