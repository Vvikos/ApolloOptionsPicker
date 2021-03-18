#include "dashboard.h"

Dashboard::Dashboard(QLabel *infoText, QProgressBar *progress, QWidget *parent)
 :
    QTabWidget(parent)
{
    this->infoText = infoText;
    this->progressBar = progress;
    this->progressBar->setMaximum(99);
    this->progressBar->setMinimum(0);
    this->setTabsClosable(true);
    this->tabBar()->setCursor(Qt::PointingHandCursor);
    this->tabBar()->setIconSize(QSize(16, 16));
    this->tabBar()->setExpanding(false);
    createAddTab();

    // Create corner widgets
    QGroupBox *cornerWidget = new QGroupBox();
    cornerWidget->setObjectName("cornerWidget");
    QHBoxLayout *cornerLayout = new QHBoxLayout();
    cornerWidget->setLayout(cornerLayout);
    cornerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QLabel *textTuples = new QLabel("Total Generated Options (APOLLO_TXTRY) : ");
    cornerLayout->addWidget(textTuples);
    cornerText = new QLabel("0");
    cornerText->setObjectName("cornerText");
    cornerText->setAlignment(Qt::AlignCenter);
    cornerLayout->addWidget(cornerText);
    this->setCornerWidget(cornerWidget);

    addRowBtn = new QPushButton();
    addRowBtn->setObjectName("miniAdd");
    addRowBtn->setCursor(Qt::PointingHandCursor);
    this->setCornerWidget(addRowBtn, Qt::TopLeftCorner);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(updateTuplesCount(int)));
    connect(this, SIGNAL(tabBarClicked(int)), this, SLOT(updateAddBtn(int)));
    connect(this, SIGNAL(tabBarClicked(int)), this, SLOT(appendTab(int)));
    connect(addRowBtn, SIGNAL(released()), this, SLOT(addOption()));

    createTab(true);
}

// PUBLIC FUNCTIONS
int Dashboard::getCurrentCount(){
    return Tabs[currentIndex()]->numberRows;
}

QString Dashboard::getCurrentPath(){
    return Tabs[currentIndex()]->filePath;
}

QVBoxLayout *Dashboard::getCurrentLayout(){
    return Tabs[currentIndex()]->layout;
}

bool Dashboard::isCurrentSaved(){
    return Tabs[currentIndex()]->saved;
}

void Dashboard::setInfoText(QString s){
    infoText->setText(s+"  ");
}

void Dashboard::loadFileCurrent(QString path) {
    if(path.isEmpty())
        return;

    int rank = count()-1;
    createTab(false);
    Tabs[rank]->filePath = path;

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        setInfoText("Couln't open file.");
        return;
    }

    QTextStream in(&file);
    // Load every row from file
    while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList opts = line.split(',');
      int idxPara = opts[0].toInt();
      int idxTile = opts[1].toInt();
      int idxIntraOpt = opts[2].toInt();
      int idx2Tile = opts[3].toInt();
      int idxIdent = opts[4].toInt();
      int idxRar = opts[5].toInt();
      int idxPartTile = opts[6].toInt();
      int idxLbTile = opts[7].toInt();
      int idxFuse = opts[8].toInt();
      int idxUnroll = opts[9].toInt();
      int idxUnrollF = opts[10].toInt();
      int idxIsl = opts[11].toInt();

      Matrix tileOpt;

      int maxSuites = opts[12].toInt();
      QStringList tileOptStr = opts[13].split('/');

      if (maxSuites > tileOptStr.length())
          break;

      // Load every suites for this row from list
      for (auto it=tileOptStr.begin(); it!=tileOptStr.end(); ++it){
          QStringList suites = (*it).split(':');

          if (suites.size() < 2)
              break;

          vector<int> row;
          for(auto it2=suites.begin(); it2!=suites.end(); ++it2){
              if(!(*it2).isEmpty())
                row.push_back((*it2).toInt());
          }

          tileOpt.push_back(row);
      }
      int nrows = Tabs[rank]->numberRows;
      OptionSet *newRow = new OptionSet(nrows, idxPara, idxTile, idxIntraOpt, idx2Tile, idxIdent, idxRar, idxPartTile, idxLbTile, idxFuse, idxUnroll, idxUnrollF, idxIsl, tileOpt, this);
      addOptionSetToCurrent(newRow);
    }
    file.close();

    Tabs[rank]->saved = true;

    QFileInfo fileInfo(file);
    setTabText(rank, fileInfo.fileName());
    setTabIcon(rank, QIcon(":/check.png"));
    setInfoText("Successfully opened file at path " + path + ".");
    updateCurrentTuplesCount();
}

void Dashboard::saveCurrent(QString path){
    int rank = currentIndex();

    if (Tabs.size() <= 1 || (Tabs[rank]->saved && path==Tabs[rank]->filePath))
        return;

    QFile file(path);

    if (!file.open(QFile::WriteOnly | QFile::Text)){
        setInfoText("Couln't open file.");
        return;
    }

    // Save every row and all of its suites into file
    QList<OptionSet *> options = widget(rank)->findChildren<OptionSet *>();
    for(auto it=options.begin(); it!=options.end(); ++it){
        QTextStream out(&file);
        out << (*it)->getParaIdx() << "," << (*it)->getTileIdx() << "," << (*it)->getIntraOptIdx() << ",";
        out << (*it)->get2TileIdx() << "," << (*it)->getIdentIdx() << "," << (*it)->getRarIdx() << ",";
        out << (*it)->getPartTileIdx() << "," << (*it)->getLbTileIdx() << "," << (*it)->getFuseIdx() << ",";
        out << (*it)->getUnrollIdx() << "," << (*it)->getUnrollFIdx() << "," << (*it)->getIslIdx() << ",";

        Matrix tilingOpt = (*it)->getTileOptions();
        int suitesSize = tilingOpt.size();

        out << suitesSize << ",";

        for (auto it2=tilingOpt.begin(); it2!=tilingOpt.end(); ++it2) {
            for(auto it3=it2->begin(); it3!=it2->end(); ++it3) {
                if (*it3==0)
                    out << *it3;
                else
                    out << ":" << *it3;
            }
            out << "/";
        }

        out << "\n";
    }
    file.close();

    Tabs[rank]->filePath = path;
    Tabs[rank]->saved = true;

    QFileInfo fileInfo(file);
    setTabText(rank, fileInfo.fileName());
    setTabIcon(rank, QIcon(":/check.png"));
    setInfoText("Successfully saved file in " + path + ".");
}

void Dashboard::exportCurrent(QString path){
    if (Tabs.size() <= 1)
        return;

    int rank = currentIndex();
    progressBar->setValue(0);

    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text)){
        setInfoText("Couln't open file.");
        return;
    }

    QList<OptionSet *> options = widget(rank)->findChildren<OptionSet *>();
    int total=0, optSetAnalyzed=0, optSetCount=options.size();
    for(auto it=options.begin(); it!=options.end(); ++it){
        QTextStream out(&file);

        Matrix tilingOpt = (*it)->getTileTuples();
        for (auto it2=tilingOpt.begin(); it2!=tilingOpt.end(); ++it2){
            out << (*it)->getParaIdx() << "," << (*it)->getTileIdx() << "," << (*it)->getIntraOptIdx() << ",";
            out << (*it)->get2TileIdx() << "," << (*it)->getIdentIdx() << "," << (*it)->getRarIdx() << ",";
            out << (*it)->getPartTileIdx() << "," << (*it)->getLbTileIdx() << "," << (*it)->getFuseIdx() << ",";
            out << (*it)->getUnrollIdx() << "," << (*it)->getUnrollFIdx() << "," << (*it)->getIslIdx() << ",";

            int size = (*it2).size();
            out << size;

            for (auto it3=it2->begin(); it3!=it2->end(); ++it3)
                out << "," << (*it3);

            out << ";";
            total++;
        }
        optSetAnalyzed++;
        float prct = (float)optSetAnalyzed/(float)optSetCount * 100.;
        progressBar->setValue(prct);
    }
    file.close();
    progressBar->setValue(0);

    QString n = QString::number(total);
    std::reverse(n.begin(), n.end());
    n = n.replace(QRegularExpression("(.{3})"), "\\1 ");
    std::reverse(n.begin(), n.end());
    setInfoText("Successfully exported file at " + path + ", " + n + " options generated.");
}

void Dashboard::addOptionSetToCurrent(OptionSet *newRow){
    int tab = currentIndex();
    int rank = Tabs[tab]->numberRows;

    Tabs[tab]->layout->insertWidget(rank, newRow);
    Tabs[tab]->numberRows++;

    connect(newRow, SIGNAL(upClicked(int)), this, SLOT(moveLineUp(int)));
    connect(newRow, SIGNAL(downClicked(int)), this, SLOT(moveLineDown(int)));
    connect(newRow, SIGNAL(deleted(int)), this, SLOT(updateCurrentRows(int)));
    connect(newRow, SIGNAL(setChanged()), this, SLOT(updateCurrentTuplesCount()));
    connect(newRow, SIGNAL(setChanged()), this, SLOT(unsaveCurrent()));

    if(Tabs[tab]->saved)
        unsaveCurrent();
    updateCurrentTuplesCount();
}

// PRIVATE FUNCTIONS
void Dashboard::permutRows(int i1, int i2) {
    QList<OptionSet *> listOpts = this->widget(currentIndex())->findChildren<OptionSet *>();
    int nOpts = listOpts.length();

    if (i1<0 || i2<0 || i1>=nOpts || i2>=nOpts)
        return;

    OptionSet *clickedRow = listOpts[i1];
    OptionSet *otherRow = listOpts[i2];

    // savedRow := clickedRow
    bool idxPara =  clickedRow->getParaIdx();
    bool idxTile = clickedRow->getTileIdx();
    bool idxIntraOpt = clickedRow->getIntraOptIdx();
    bool idx2Tile = clickedRow->get2TileIdx();
    bool idxIdent = clickedRow->getIdentIdx();
    bool idxRar = clickedRow->getRarIdx();
    bool idxPartTile = clickedRow->getPartTileIdx();
    bool idxLbTile = clickedRow->getLbTileIdx();
    int idxFuse = clickedRow->getFuseIdx();
    bool idxUnroll = clickedRow->getUnrollIdx();
    int idxUnrollF = clickedRow->getUnrollFIdx();
    bool idxIsl = clickedRow->getIslIdx();
    Matrix tiling = clickedRow->getTileOptions();

    // clickedRow := otherRow
    clickedRow->setParaIdx(otherRow->getParaIdx());
    clickedRow->setIdentIdx(otherRow->getIdentIdx());
    clickedRow->setRarIdx(otherRow->getRarIdx());
    clickedRow->setFuseIdx(otherRow->getFuseIdx());
    clickedRow->setUnrollIdx(otherRow->getUnrollIdx());
    clickedRow->setUnrollFIdx(otherRow->getUnrollFIdx());
    clickedRow->setIslIdx(otherRow->getIslIdx());
    clickedRow->setTileOptions(otherRow->getTileIdx(), otherRow->getIntraOptIdx(), otherRow->get2TileIdx(), otherRow->getPartTileIdx(), otherRow->getLbTileIdx(), otherRow->getTileOptions());

    // otherRow := savedRow
    otherRow->setParaIdx(idxPara);
    otherRow->setIdentIdx(idxIdent);
    otherRow->setRarIdx(idxRar);
    otherRow->setFuseIdx(idxFuse);
    otherRow->setUnrollIdx(idxUnroll);
    otherRow->setUnrollFIdx(idxUnrollF);
    otherRow->setIslIdx(idxIsl);
    otherRow->setTileOptions(idxTile, idxIntraOpt, idx2Tile, idxPartTile, idxLbTile, tiling);
}


void Dashboard::createTab(bool firstRow){
    QScrollArea *scroll = new QScrollArea();
    TabContent *innerScroll = new TabContent();
    int rank = this->count()-1;
    Tabs.insert(rank, innerScroll);

    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setWidgetResizable(true);
    scroll->setWidget(innerScroll);

    this->insertTab(rank, scroll, "New");
    this->tabBar()->tabButton(rank, QTabBar::RightSide)->setCursor(Qt::PointingHandCursor);
    this->setCurrentIndex(rank);
    if (firstRow)
        addOption();
}

// PRIVATE SLOTS
void Dashboard::unsaveCurrent(){
    int rank = currentIndex();
    if (!Tabs[rank]->filePath.isEmpty()){
        setTabIcon(rank, QIcon(":/check-negativ.png"));
        Tabs[rank]->saved = false;
    }
}

void Dashboard::updateAddBtn(int rank){
    int tabs = this->count();
    if (tabs<=1 || rank==tabs-1)
        addRowBtn->setDisabled(false);
}

void Dashboard::moveLineUp(int rank) {
    int idxTab = currentIndex();
    int nOpts = Tabs[idxTab]->numberRows;
    int rank2 = mod(rank - 1, nOpts);

    permutRows(rank, rank2);

    setInfoText("Option " + QString::number(rank+1) + " successfully swapped with option " + QString::number(rank2+1) + ".");
}

void Dashboard::moveLineDown(int rank) {
    int idxTab = currentIndex();
    int nOpts = Tabs[idxTab]->numberRows;
    int rank2 = mod(rank + 1, nOpts);

    permutRows(rank, rank2);

    setInfoText("Option " + QString::number(rank+1) + " successfully swapped with option " + QString::number(rank2+1) + ".");
}

// PUBLIC SLOTS
void Dashboard::appendTab(){
    createTab(true);
}

void Dashboard::appendTab(int n){
    if (n == count()-1)
        createTab(true);
}

void Dashboard::createAddTab(){
    TabContent *innerScroll = new TabContent();
    innerScroll->scrollBar = new QScrollArea();
    Tabs.append(innerScroll);

    innerScroll->scrollBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    innerScroll->scrollBar->setWidget(innerScroll);
    this->addTab(innerScroll->scrollBar, "+");
    tabBar()->tabButton(0, QTabBar::RightSide)->setDisabled(true);
    tabBar()->tabButton(0, QTabBar::RightSide)->setVisible(false);
    tabBar()->tabButton(0, QTabBar::RightSide)->setFixedSize(0, 0);
}

void Dashboard::closeTab(int rank){
    if (Tabs.size() <= 1)
        return;

    Tabs.removeAt(rank);
    removeTab(rank);
    setCurrentIndex(rank-1);
    setInfoText("File successfully closed.");
}

void Dashboard::closeCurrent(){
    int index = currentIndex();
    closeTab(index);
    updateAddBtn(index);
}


void Dashboard::addOption(){
    if (Tabs.size() <= 1)
        return;

    int nrows = Tabs[currentIndex()]->numberRows;
    Matrix base;
    OptionSet *newRow = new OptionSet(nrows,1,1,0,0,0,0,0,0,0,0,8,0, base);
    addOptionSetToCurrent(newRow);

    if (isCurrentSaved())
        setTabIcon(currentIndex(), QIcon(":/check-negativ.png"));
    setInfoText("Successfully added option to current tab.");
}

void Dashboard::updateCurrentRows(int rank){
    Tabs[currentIndex()]->numberRows--;
    QList<OptionSet *> listOpts = this->widget(currentIndex())->findChildren<OptionSet *>();
    OptionSet *toDelete = listOpts[rank];
    delete toDelete;
    listOpts.removeAt(rank);

    for (int i=0; i<listOpts.length(); i++)
        listOpts[i]->setRank(i);
    updateCurrentTuplesCount();
    unsaveCurrent();
}

void Dashboard::updateTuplesCount(int rank){
    QList<OptionSet *> options = widget(rank)->findChildren<OptionSet *>();
    unsigned long total = 0;
    bool limit = false;

    for(auto it=options.begin(); it!=options.end() && !limit; ++it)
        total += (*it)->getNbTuples();

    if(total > LIMIT_TUPLES)
        total = LIMIT_TUPLES;

    QString n = QString::number(total);
    std::reverse(n.begin(), n.end());
    n = n.replace(QRegularExpression("(.{3})"), "\\1 ");
    std::reverse(n.begin(), n.end());

    if(total==LIMIT_TUPLES)
        cornerText->setText(" > " + n);
    else
        cornerText->setText(n);
}

void Dashboard::updateCurrentTuplesCount(){
    updateTuplesCount(currentIndex());
}
