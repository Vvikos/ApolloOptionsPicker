#include "tileoption.h"

TileOption::TileOption(int idxTile, int idxIntraOpt, int idx2Tile, int idxPartTile, int idxLbTile, Matrix opt, QWidget *parent)
 :
    QWidget(parent)
{
        this->setMinimumWidth(1000);

        // create tile toggle and loop depth
        checkTile = new QCheckBox("tile");
        checkTile->setChecked(true);
        checkTile->setCursor(Qt::PointingHandCursor);
        checkTile->setChecked(idxTile);

        labelDepth = new QLabel("loop depth");
        depth = new QSpinBox();
        depth->setMinimum(1);

        // create suites inputs
        groupTileSuites = new QGroupBox("  Configure a set of sizes to use during compiling");
        QVBoxLayout *layoutTileSuites = new QVBoxLayout();
        groupTileSuites->setLayout(layoutTileSuites);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        constant = new QCheckBox("Manual");
        constant->setCursor(Qt::PointingHandCursor);
        constant->setChecked(false);
        btnLayout->addWidget(constant);
        btnLayout->addStretch();
        layoutTileSuites->addLayout(btnLayout);
        layoutTileSuites->addSpacing(10);

        QHBoxLayout *layoutInputRow = new QHBoxLayout();

        // Create from .. to .. do .. .. input
        QVBoxLayout *layoutInputSuite = new QVBoxLayout();
        QHBoxLayout *startEndLayout = new QHBoxLayout();
        startLabel = new QLabel("From ");
        startEndLayout->addWidget(startLabel);
        startOpe = new QSpinBox();
        startOpe->setMinimum(0);
        startOpe->setMaximum(TILE_SIZE_LIMIT);
        startOpe->setValue(2);
        startOpe->setCursor(Qt::PointingHandCursor);
        startEndLayout->addWidget(startOpe);

        endLabel = new QLabel("to");
        endLabel->setAlignment(Qt::AlignCenter);
        startEndLayout->addWidget(endLabel);
        endOpe = new QSpinBox();
        endOpe->setMinimum(0);
        endOpe->setMaximum(TILE_SIZE_LIMIT);
        endOpe->setValue(32);
        endOpe->setCursor(Qt::PointingHandCursor);
        startEndLayout->addWidget(endOpe);

        doLabel = new QLabel("do");
        doLabel->setAlignment(Qt::AlignCenter);
        startEndLayout->addWidget(doLabel);

        ope = new QComboBox();
        ope->addItem("+");
        ope->addItem("-");
        ope->addItem("/");
        ope->addItem("*");
        ope->addItem("^");
        ope->setCurrentIndex(4);
        ope->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        ope->setCursor(Qt::PointingHandCursor);
        ope->setEditable(true);
        ope->lineEdit()->setAlignment(Qt::AlignCenter);
        ope->lineEdit()->setReadOnly(true);
        startEndLayout->addWidget(ope);

        operande = new QSpinBox();
        operande->setMinimum(1);
        operande->setMaximum(TILE_SIZE_LIMIT);
        operande->setValue(2);
        operande->setCursor(Qt::PointingHandCursor);
        startEndLayout->addWidget(operande);
        startEndLayout->addSpacing(20);

        layoutInputSuite->addLayout(startEndLayout);

        // Create manual csv input line
        manualInput = new QLineEdit();
        manualInput->setObjectName("constSuite");
        manualInput->setFixedHeight(30);
        manualInput->setPlaceholderText(" 2, 4, 5, 9...");
        layoutInputSuite->addWidget(manualInput);

        layoutInputSuite->addStretch();

        layoutInputRow->addLayout(layoutInputSuite);

        QVBoxLayout *layoutSetBtn = new QVBoxLayout();

        // Create set btn
        setRowBtn = new QPushButton("Set");
        setRowBtn->setFixedSize(45, 30);
        setRowBtn->setCursor(Qt::PointingHandCursor);
        setRowBtn->setObjectName("miniSet");
        layoutSetBtn->addWidget(setRowBtn);
        layoutSetBtn->addStretch();

        layoutInputRow->addLayout(layoutSetBtn);

        layoutTileSuites->addLayout(layoutInputRow);

        // Create list of suites widget
        tileOptList = new QListWidget();
        tileOptList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tileOptList->setMaximumWidth(400);
        tileOptList->setDragDropMode(QAbstractItemView::InternalMove); // QAbstractItemView::DragDrop
        tileOptList->setDefaultDropAction(Qt::MoveAction);
        tileOptList->setSelectionMode(QAbstractItemView::SingleSelection);

        // Create extra tile options toggle buttons
        groupExtraOpts = new QGroupBox("  Extra tile options");
        QHBoxLayout *extraOptsLayout = new QHBoxLayout();
        groupExtraOpts->setLayout(extraOptsLayout);

        extraOptsLayout->addStretch();
        checkIntraOpt = new QCheckBox("intratileopt");
        checkIntraOpt->setCursor(Qt::PointingHandCursor);
        checkIntraOpt->setChecked(idxIntraOpt);
        extraOptsLayout->addWidget(checkIntraOpt);

        extraOptsLayout->addStretch();

        check2Tile = new QCheckBox("l2tile");
        check2Tile->setCursor(Qt::PointingHandCursor);
        check2Tile->setChecked(idx2Tile);
        extraOptsLayout->addWidget(check2Tile);

        extraOptsLayout->addStretch();

        checkPartTile = new QCheckBox("partlbtile");
        checkPartTile->setCursor(Qt::PointingHandCursor);
        checkPartTile->setChecked(idxPartTile);
        extraOptsLayout->addWidget(checkPartTile);

        extraOptsLayout->addStretch();

        checkLbTile = new QCheckBox("lbtile");
        checkLbTile->setCursor(Qt::PointingHandCursor);
        checkLbTile->setChecked(idxLbTile);
        extraOptsLayout->addWidget(checkLbTile);

        extraOptsLayout->addStretch();

        // ===========================
        QWidget *tileOptWidget = new QWidget();
        QVBoxLayout *tileOptLayout = new QVBoxLayout();
        tileOptWidget->setLayout(tileOptLayout);
        tileOptWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        tileOptLayout->addWidget(groupTileSuites);
        tileOptLayout->addWidget(groupExtraOpts);

        // ===========================
        QHBoxLayout *headerLayout = new QHBoxLayout();
        headerLayout->addWidget(checkTile);
        headerLayout->addStretch();
        headerLayout->addWidget(labelDepth);
        headerLayout->addWidget(depth);

        QHBoxLayout *suitesLayout = new QHBoxLayout();
        suitesLayout->addWidget(tileOptWidget);
        suitesLayout->addWidget(tileOptList);

        // ===========================
        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->addLayout(headerLayout);
        mainLayout->addLayout(suitesLayout);

        this->setLayout(mainLayout);

        // Load tiling input matrix
        this->suitesData = opt;
        unsigned nbSuites = opt.size();
        this->depth->setValue(nbSuites);

        for (auto it=opt.begin(); it!=opt.end(); ++it)
            this->tileOptList->addItem(getSuiteStr(*it));

        // Activation de l'input l2tile quand tile est changé
        connect(checkTile, SIGNAL(stateChanged(int)), this, SLOT(updateInputs(int)));
        // Activation de l'input PartLbTile quand LbTile est changé
        connect(checkLbTile, SIGNAL(toggled(bool)), checkPartTile, SLOT(setDisabled(bool)));
        // et inversement
        connect(checkPartTile, SIGNAL(toggled(bool)), checkLbTile, SLOT(setDisabled(bool)));
        connect(setRowBtn, SIGNAL(released()), this, SLOT(setRow()));
        connect(tileOptList->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(swapSuites(QModelIndex,int,int,QModelIndex,int)));
        connect(tileOptList, SIGNAL(currentRowChanged(int)), this, SLOT(updateInputSuite(int)));
        connect(constant, SIGNAL(stateChanged(int)), this, SLOT(updateConstantInput(int)));
        connect(manualInput, SIGNAL(textChanged(QString)), this, SLOT(manualInputValidator(QString)));
        connect(depth, SIGNAL(valueChanged(int)), this, SLOT(updateNumberRows(int)));
        // tileChange signal
        connect(checkTile, SIGNAL(released()), this, SLOT(emitTileChanged()));
        connect(check2Tile, SIGNAL(released()), this, SLOT(emitTileChanged()));
        connect(checkIntraOpt, SIGNAL(released()), this, SLOT(emitTileChanged()));
        connect(checkLbTile, SIGNAL(released()), this, SLOT(emitTileChanged()));
        connect(checkPartTile, SIGNAL(released()), this, SLOT(emitTileChanged()));
        connect(setRowBtn, SIGNAL(released()), this, SLOT(emitTileChanged()));
        connect(tileOptList->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(emitTileChanged()));
        connect(depth, SIGNAL(valueChanged(int)), this, SLOT(emitTileChanged()));

        updateInputs(idxTile);
        if (nbSuites < 1)
            updateNumberRows(1);
        updateConstantInput(false);
        manualInputValidator("");
}

// PUBLIC FUNCTIONS
bool TileOption::getTileIdx(){
    return checkTile->isChecked();
}

bool TileOption::getIntraOptIdx(){
    return checkIntraOpt->isChecked();
}

bool TileOption::get2TileIdx(){
    return check2Tile->isChecked();
}

bool TileOption::getPartTileIdx(){
    return checkPartTile->isChecked();
}

bool TileOption::getLbTileIdx(){
    return checkLbTile->isChecked();
}

unsigned long TileOption::getNbTuples(){
    int nOpts = suitesData.size();
    Matrix suites;

    // Generate all suites
    for (int i = 0; i<nOpts; i++) {
        if (suitesData[i].at(0)){
            vector<int>::const_iterator first = suitesData[i].begin() + 2;
            vector<int>::const_iterator last = suitesData[i].end();
            vector<int> newVec(first, last);
            if ((int)newVec.size() != suitesData[i].at(1))
                qDebug() << "error not same size";
            suites.push_back(newVec);
        }else{

            int start = suitesData[i].at(2);
            int end = suitesData[i].at(3);
            int op = suitesData[i].at(4);
            int operand = suitesData[i].at(5);

            suites.push_back(getSuite(start, end, op, operand));
        }
    }

    // Compute total number of options and the repetitions for each column
    unsigned long nTuples = 1;
    for (unsigned i=0; i<suites.size(); i++)
        nTuples *= suites[i].size();

    return nTuples;
}

Matrix TileOption::getTileOptions(){
    Matrix result;
    for(unsigned i=0; i<suitesData.size(); i++)
        result.push_back(suitesData[i]);
    return result;
}

Matrix *TileOption::getTileTuples(){
    int nOpts = suitesData.size();
    Matrix *suites = new Matrix();

    if(!checkTile->isChecked()){
        vector<int> v;
        v.push_back(1);
        suites->push_back(v);
        return suites;
    }

    // Generate all suites
    for (int i = 0; i<nOpts; i++){
        if (suitesData[i].at(0)){
            vector<int>::const_iterator first = suitesData[i].begin() + 2;
            vector<int>::const_iterator last = suitesData[i].end();
            vector<int> newVec(first, last);
            if ((int)newVec.size() != suitesData[i].at(1))
                qDebug() << "error not same size";

            suites->push_back(newVec);
        }else{

            int start = suitesData[i].at(2);
            int end = suitesData[i].at(3);
            int op = suitesData[i].at(4);
            int operand = suitesData[i].at(5);

            suites->push_back(getSuite(start, end, op, operand));
        }
    }

    // Compute total number of options and the repetitions for each column
    vector<int> diviseur;
    unsigned nTuples = 1;
    for (auto it=suites->begin(); it!=suites->end(); ++it){
        nTuples *= (*it).size();
        diviseur.push_back(nTuples);
    }


    // Initialize tuples
    Matrix *tuples = new Matrix();
    for (unsigned i=0; i<nTuples; i++){
        vector<int> l;
        tuples->push_back(l);
    }

    // generer toutes les combinaisons possibles
    for(unsigned i=0; i<suites->size(); i++)                                         // For every suite
        for(unsigned pas=0; pas<nTuples; pas+=nTuples/diviseur[i])                  // Do rep times
                for(unsigned k=0; k<nTuples/diviseur[i]; k++){                       // Add n times this element to tuples
                    unsigned col = (pas/(nTuples/diviseur[i])) % suites->at(i).size();
                    tuples->at(pas+k).push_back(suites->at(i)[col]);
                }
    delete suites;

    return tuples;
}

// ==

void TileOption::setDepth(int d){
    if (d < 1)
        return;

    depth->setValue(d);
}

// PRIVATE SLOTS
void TileOption::updateConstantInput(int state){
    manualInput->setVisible(state);

    int currRow = tileOptList->currentRow();
    updateInputSuite(currRow);
    if (state)
        manualInputValidator(manualInput->text());

    ope->setVisible(!state);
    operande->setVisible(!state);
    startOpe->setVisible(!state);
    endOpe->setVisible(!state);
    startLabel->setVisible(!state);
    endLabel->setVisible(!state);
    doLabel->setVisible(!state);
}

void TileOption::swapSuites(QModelIndex parent, int start, int end, QModelIndex destination, int row){
    (void) parent;
    (void) destination;

    int dest = row;
    int maxSize = tileOptList->count();
    if(row >= maxSize)
        dest = maxSize-1;

    if(start!=end)
        return;

    std::vector<int> saved;
    saved = suitesData[start];
    suitesData[start] = suitesData[dest];
    suitesData[dest] = saved;
}

void TileOption::setRow(){
    int rank = tileOptList->currentRow();

    if (tileOptList->currentRow() < 0)
        return;

    bool manualMode = constant->isChecked();

    vector<int> l;
    l.push_back(manualMode);

    if (manualMode){
        QString input(manualInput->text().replace(" ", ""));
        QRegularExpression separator("[,;]");
        QStringList strList = input.split(separator);

        int size = 0;
        for(auto it=strList.begin(); it!=strList.end(); it++){
            if (!(*it).isEmpty())
                size++;
        }
        l.push_back(size);
        for (int i=0; i<size; i++)
            l.push_back(strList[i].toInt());

    } else {
        int start = startOpe->value();
        int end = endOpe->value();
        int op = ope->currentIndex();
        int operand = operande->value();

        l.push_back(4);
        l.push_back(start);
        l.push_back(end);
        l.push_back(op);
        l.push_back(operand);
    }


    QString newRow = getSuiteStr(l);

    if(newRow.isEmpty())
        return;
    suitesData[rank] = l;
    delete tileOptList->item(rank);
    tileOptList->insertItem(rank, newRow);
}

void TileOption::manualInputValidator(QString s){
    bool error = false;

    if (!checkTile->isChecked())
        return;

    if (!s.isEmpty()) {
        QRegularExpression re("([\\s]*[\\d]+[\\s]*[,;][\\s]*)*[\\s]*[\\d]+([\\s]*[,;][\\s]*|[\\s]*)");
        QRegularExpressionMatch match = re.match(s);

        if (!match.hasMatch() || match.capturedLength(0)!=s.length())
            error = true;
    }

    setRowBtn->setDisabled(error || s.isEmpty() || tileOptList->currentRow()<0);
    manualInput->setProperty("error", error);
    manualInput->style()->unpolish(manualInput);
    manualInput->style()->polish(manualInput);
    manualInput->update();
}

void TileOption::updateInputSuite(int n){
    QString depth = QString::number(n+1);

    if (n < 0){
        setRowBtn->setDisabled(true);
        groupTileSuites->setTitle("  Configure a set of sizes to use during compiling");
    }else{
        setRowBtn->setEnabled(true);
        groupTileSuites->setTitle("  Configure a set of sizes to use on depth: " + depth);
    }

    if(constant->isChecked())
        manualInputValidator(manualInput->text());
}

void TileOption::updateInputs(int n){
    groupExtraOpts->setEnabled(n);
    groupTileSuites->setEnabled(n);
    depth->setEnabled(n);
    labelDepth->setEnabled(n);
    tileOptList->setEnabled(n);

    if (n){
        updateInputSuite(tileOptList->currentRow());
        manualInputValidator(manualInput->text());
    }
}

void TileOption::updateNumberRows(int n){
    int nRows = tileOptList->count();

    // Update number of rows in tiling list
    if (n < nRows){
        for(int i=nRows-1; i>=n; i--){
            suitesData.erase(suitesData.begin() + i);
            delete tileOptList->item(i);
        }
    }else{
        for(int i=nRows; i<n; i++){
            vector<int> v;
            v.push_back(0);
            v.push_back(4);
            v.push_back(2);
            v.push_back(32);
            v.push_back(4);
            v.push_back(2);
            suitesData.push_back(v);
            tileOptList->addItem(getSuiteStr(v));
        }
    }
}

void TileOption::emitTileChanged(){
    emit tileChanged();
}
