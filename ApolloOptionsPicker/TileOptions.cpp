#include "TileOptions.h"

TileOptions::TileOptions(int op, int tileSize, int opRight, int start, int end, QWidget *parent)
 :
    QWidget(parent)
{
    this->op = op;
    this->tileSize = tileSize;
    this->opRight = opRight;
    this->start = start;
    this->end = end;
}
