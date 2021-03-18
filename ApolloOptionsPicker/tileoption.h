#ifndef TILEOPTION_H
#define TILEOPTION_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCommonStyle>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QListWidget>
#include <QLineEdit>
#include <QGroupBox>
#include <QRegularExpression>
#include <toolbox.h>

using namespace std;

class TileOption : public QWidget
{
    Q_OBJECT

    Matrix suitesData;

    QGroupBox *groupTileSuites;
    QGroupBox *groupExtraOpts;

    QCheckBox *checkTile;

    QLabel *tileOptTitle;
    QSpinBox *depth;
    QLabel *labelDepth;

    QCheckBox *constant;

    QPushButton *setRowBtn;

    QLabel *startLabel;
    QLabel *endLabel;
    QLabel *doLabel;
    QComboBox *ope;
    QSpinBox *operande;
    QSpinBox *startOpe;
    QSpinBox *endOpe;

    QLineEdit *manualInput;
    QListWidget *tileOptList;

    // EXTRA OPT
    QCheckBox *checkIntraOpt;
    QCheckBox *check2Tile;
    QCheckBox *checkPartTile;
    QCheckBox *checkLbTile;

    QPushButton *updateRowBtn;

public:
    explicit TileOption(int idxTile, int idxIntraOpt, int idx2Tile, int idxPartTile, int idxLbTile, Matrix opt, QWidget *parent = 0);

    bool getTileIdx();
    bool getIntraOptIdx();
    bool get2TileIdx();
    bool getPartTileIdx();
    bool getLbTileIdx();
    Matrix getTileOptions();
    Matrix getTileTuples();
    unsigned long getNbTuples();
    void setDepth(int d);

private:

signals:
    void tileChanged();

private slots:
    void updateConstantInput(int state);
    void swapSuites(QModelIndex parent, int start, int end, QModelIndex destination, int row);
    void setRow();
    void manualInputValidator(QString s);
    void updateInputSuite(int n);
    void updateInputs(int n);
    void updateNumberRows(int n);
    void emitTileChanged();

public slots:
};

#endif // TileOption_H
