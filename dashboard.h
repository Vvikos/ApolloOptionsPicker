#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QScrollArea>
#include <QPushButton>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QProgressBar>
#include <tabcontent.h>
#include <optionset.h>

#define LIMIT_TUPLES 999999999

class Dashboard : public QTabWidget
{
    Q_OBJECT

    QList<TabContent*> Tabs;
    QLabel *infoText;
    QProgressBar *progressBar;
    OptionSet *inputRow;
    QLabel *cornerText;
    QPushButton *addRowBtn;

public:
    explicit Dashboard(QLabel *infoText, QProgressBar *progress, QWidget *parent = 0);
    int getCurrentCount();
    QString getCurrentPath();
    QVBoxLayout *getCurrentLayout();
    bool isCurrentSaved();
    void setInfoText(QString s);
    void loadFileCurrent(QString path);
    void saveCurrent(QString path);
    void exportCurrent(QString path);
    void addOptionSetToCurrent(OptionSet *newRow);

private:
    void permutRows(int i1, int i2);
    void createTab(bool firstRow);

private slots:
    void unsaveCurrent();
    void updateAddBtn(int rank);
    void moveLineDown(int rank);
    void moveLineUp(int rank);

public slots:
    void appendTab();
    void appendTab(int n);
    void createAddTab();
    void closeTab(int rank);
    void closeCurrent();
    void addOption();
    void updateCurrentRows(int n);
    void updateTuplesCount(int rank);
    void updateCurrentTuplesCount();
};

#endif // DASHBOARD_H
