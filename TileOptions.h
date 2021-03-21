#ifndef TileOptions_H
#define TileOptions_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCommonStyle>
#include <QPushButton>
#include <QTileOption.h>

class TileOptions : public QWidget
{
    Q_OBJECT

public:
    explicit TileOptions(int op, int tileSize, int opRight, int start, int end, QWidget *parent=0);

private:
    int op;
    int tileSize;
    int opRight;
    int start;
    int end;


signals:

private slots:

public slots:
};

#endif // TileOptions_H
