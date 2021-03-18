#ifndef TabContent_H
#define TABCONTENT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QScrollArea>
#include <QPushButton>

class TabContent : public QWidget
{
    Q_OBJECT

public:
    explicit TabContent(QWidget *parent = 0);
    QScrollArea *scrollBar;
    QVBoxLayout *layout;
    QString filePath;
    bool saved;
    int numberRows;

private:

private slots:

public slots:
};

#endif // TABCONTENT_H
