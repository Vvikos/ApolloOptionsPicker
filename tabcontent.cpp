#include "tabcontent.h"

TabContent::TabContent(QWidget *parent)
 :
    QWidget(parent)
{
    saved = false;
    numberRows = 0;
    layout = new QVBoxLayout(this);
    layout->addStretch();
    this->setLayout(layout);
}
