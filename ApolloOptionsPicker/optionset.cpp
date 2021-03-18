#include "optionset.h"

OptionSet::OptionSet(int rank, bool idxPara, bool idxTile, bool idxIntraOpt, bool idx2Tile, bool idxIdent, bool idxRar, bool idxPartTile, bool idxLbTile, int idxFuse, bool idxUnroll, int idxUnrollF, bool idxIsl, Matrix tileOpt, QWidget *parent)
 :
    QWidget(parent)
{
        this->setFixedHeight(325);
        QVBoxLayout *mainLayout = new QVBoxLayout();
        layout = new QHBoxLayout();

        this->rank = rank;

        QVBoxLayout *vLayout = new QVBoxLayout();

        vLayout->addStretch(2);
        upArrow = new QPushButton();
        upArrow->setObjectName("upArrow");
        upArrow->setCursor(Qt::PointingHandCursor);
        vLayout->addWidget(upArrow);

        vLayout->addStretch(1);
        QHBoxLayout *middleLay = new QHBoxLayout();

        textRank = new QLabel(QString::number(rank+1));
        textRank->setAlignment(Qt::AlignCenter);
        middleLay->addWidget(textRank);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        middleLay->addWidget(line);


        QPushButton *deleteBtn = new QPushButton("");
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setObjectName("miniDelete");
        middleLay->addWidget(deleteBtn);

        vLayout->addLayout(middleLay);

        vLayout->addStretch(1);

        downArrow = new QPushButton();
        downArrow->setObjectName("downArrow");
        downArrow->setCursor(Qt::PointingHandCursor);
        vLayout->addWidget(downArrow);
        vLayout->addStretch(2);

        QWidget *arrowsWidget = new QWidget();
        arrowsWidget->setLayout(vLayout);
        arrowsWidget->setFixedWidth(150);


        checkPara = new QCheckBox("parallel");
        checkPara->setCursor(Qt::PointingHandCursor);
        checkPara->setChecked(idxPara);

        checkIdent = new QCheckBox("identity");
        checkIdent->setCursor(Qt::PointingHandCursor);
        checkIdent->setChecked(idxIdent);

        checkRar = new QCheckBox("rar");
        checkRar->setCursor(Qt::PointingHandCursor);
        checkRar->setChecked(idxRar);

        checkIsl = new QCheckBox("isl");
        checkIsl->setCursor(Qt::PointingHandCursor);
        checkIsl->setChecked(idxIsl);

        QHBoxLayout *unrollfLayout = new QHBoxLayout();
        checkUnroll = new QCheckBox("unroll");
        checkUnroll->setCursor(Qt::PointingHandCursor);
        checkUnroll->setChecked(idxUnroll);
        unrollfLayout->addWidget(checkUnroll);
        unrollfLayout->addStretch();
        textUnrollF = new QLabel("ufactor");
        unrollfLayout->addWidget(textUnrollF);
        spinUnrollF  = new QSpinBox();
        spinUnrollF->setValue(idxUnrollF);
        spinUnrollF->setSuffix("x");
        spinUnrollF->setCursor(Qt::PointingHandCursor);
        unrollfLayout->addWidget(spinUnrollF);
        updateUnrollF(idxUnroll);

        groupAddiOpt = new QGroupBox("  Additional configuration");
        QVBoxLayout *layoutAddiOpt = new QVBoxLayout();
        groupAddiOpt->setLayout(layoutAddiOpt);

        layoutAddiOpt->addWidget(checkPara);
        layoutAddiOpt->addWidget(checkRar);
        layoutAddiOpt->addWidget(checkIdent);
        layoutAddiOpt->addWidget(checkIsl);
        layoutAddiOpt->addLayout(unrollfLayout);


        groupFuse = new QGroupBox("  Fusion");
        QHBoxLayout *fuseLayout = new QHBoxLayout();
        groupFuse->setMaximumHeight(75);
        groupFuse->setLayout(fuseLayout);
        comboFuse = new QComboBox();
        comboFuse->addItem(QString("No"));
        comboFuse->addItem(QString("Maximal"));
        comboFuse->addItem(QString("Smart"));
        comboFuse->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        comboFuse->setCursor(Qt::PointingHandCursor);
        comboFuse->setEditable(true);
        comboFuse->lineEdit()->setAlignment(Qt::AlignCenter);
        comboFuse->lineEdit()->setReadOnly(true);
        comboFuse->setCurrentIndex(idxFuse);
        fuseLayout->addWidget(comboFuse);
        textFuse = new QLabel("fuse");
        fuseLayout->addWidget(textFuse);
        fuseLayout->addStretch();

        QWidget *widgetFuseAddi = new QWidget();
        widgetFuseAddi->setFixedWidth(500);
        QVBoxLayout *layoutFuseAddi = new QVBoxLayout();
        widgetFuseAddi->setLayout(layoutFuseAddi);
        layoutFuseAddi->addWidget(groupAddiOpt);
        layoutFuseAddi->addWidget(groupFuse);

        tileOpts = new TileOption(idxTile, idxIntraOpt, idx2Tile, idxPartTile, idxLbTile, tileOpt);
        // ===========================
        layout->addWidget(arrowsWidget);
        layout->addStretch();
        layout->addWidget(widgetFuseAddi);
        layout->addStretch();
        layout->addWidget(tileOpts);
        // ===========================
        mainLayout->addLayout(layout);

        QFrame* line2 = new QFrame();
        line2->setFrameShape(QFrame::HLine);
        line2->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line2);

        setLayout(mainLayout);
        // swap and delete event
        connect(upArrow, SIGNAL(clicked()), this, SLOT(emitUpArrowClicked()));
        connect(downArrow, SIGNAL(clicked()), this, SLOT(emitDownArrowClicked()));
        connect(deleteBtn, SIGNAL(released()), this, SLOT(deleteObject()));
        // Row changed
        connect(checkPara, SIGNAL(released()), this, SLOT(emitSetChanged()));
        connect(checkIdent, SIGNAL(released()), this, SLOT(emitSetChanged()));
        connect(checkRar, SIGNAL(released()), this, SLOT(emitSetChanged()));
        connect(checkIsl, SIGNAL(released()), this, SLOT(emitSetChanged()));
        connect(checkUnroll, SIGNAL(released()), this, SLOT(emitSetChanged()));
        connect(comboFuse, SIGNAL(currentIndexChanged(int)), this, SLOT(emitSetChanged()));
        connect(tileOpts, SIGNAL(tileChanged()), this, SLOT(emitSetChanged()));
        // de/activate unrollf
        updateUnrollF(idxUnroll);
        connect(checkUnroll, SIGNAL(stateChanged(int)), this, SLOT(updateUnrollF(int)));
}

// PUBLIC FUNCTIONS
bool OptionSet::getParaIdx(){
    return checkPara->isChecked();
}

bool OptionSet::getTileIdx(){
    return tileOpts->getTileIdx();
}

bool OptionSet::getIntraOptIdx(){
    return tileOpts->getIntraOptIdx();
}

bool OptionSet::get2TileIdx(){
    return tileOpts->get2TileIdx();
}

bool OptionSet::getIdentIdx(){
    return checkIdent->isChecked();
}

bool OptionSet::getRarIdx(){
    return checkRar->isChecked();
}

bool OptionSet::getPartTileIdx(){
    return tileOpts->getPartTileIdx();
}

bool OptionSet::getLbTileIdx(){
    return tileOpts->getLbTileIdx();
}

int OptionSet::getFuseIdx(){
    return comboFuse->currentIndex();
}

bool OptionSet::getUnrollIdx(){
    return checkUnroll->isChecked();
}

int OptionSet::getUnrollFIdx(){
    return spinUnrollF->value();
}

bool OptionSet::getIslIdx(){
    return checkIsl->isChecked();
}

unsigned long OptionSet::getNbTuples(){
    if(!tileOpts->getTileIdx())
        return 1;

    return tileOpts->getNbTuples();
}

Matrix OptionSet::getTileOptions(){
    return tileOpts->getTileOptions();
}

Matrix OptionSet::getTileTuples(){
    return tileOpts->getTileTuples();
}

// ==

void OptionSet::setRank(int n){
    rank = n;
    textRank->setText(QString::number(n+1));
}

void OptionSet::setParaIdx(bool val){
    checkPara->setChecked(val);
}

void OptionSet::setIdentIdx(bool val){
    checkIdent->setChecked(val);
}

void OptionSet::setRarIdx(bool val){
    checkRar->setChecked(val);
}

void OptionSet::setFuseIdx(int val){
    comboFuse->setCurrentIndex(val);
}

void OptionSet::setUnrollIdx(bool val){
    checkUnroll->setChecked(val);
}

void OptionSet::setUnrollFIdx(int val){
    spinUnrollF->setValue(val);
}

void OptionSet::setIslIdx(bool val){
    checkIsl->setChecked(val);
}

void OptionSet::setTileOptions(int idxTile, int idxIntraOpt, int idx2Tile, int idxPartTile, int idxLbTile, Matrix opts){
    delete tileOpts;
    tileOpts = new TileOption(idxTile, idxIntraOpt, idx2Tile, idxPartTile, idxLbTile, opts);
    layout->addWidget(tileOpts);
    connect(tileOpts, SIGNAL(tileChanged()), this, SLOT(emitSetChanged()));
}

// PRIVATE SLOTS
void OptionSet::updateUnrollF(int n){
    spinUnrollF->setEnabled(n);
    textUnrollF->setEnabled(n);
}

void OptionSet::deleteObject(){
    emit deleted(rank);
}

void OptionSet::emitUpArrowClicked(){
    emit upClicked(rank);
}

void OptionSet::emitDownArrowClicked(){
    emit downClicked(rank);
}

void OptionSet::emitSetChanged(){
    emit setChanged();
}

