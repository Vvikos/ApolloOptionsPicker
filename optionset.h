#ifndef OPTIONSET_H
#define OPTIONSET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCommonStyle>
#include <QPushButton>
#include <tileoption.h>
#include <QCheckBox>

using namespace std;

class OptionSet : public QWidget
{
    Q_OBJECT

public:
    explicit OptionSet(int rank, bool idxPara, bool idxTile, bool idxIntraOpt, bool idx2Tile, bool idxIdent, bool idxRar, bool idxPartTile, bool idxLbTile, int idxFuse, bool idxUnroll, int idxUnrollF, bool idxIsl, Matrix tileOpt, QWidget *parent = 0);

    QPushButton *upArrow;
    QPushButton *downArrow;

    bool getParaIdx();
    bool getTileIdx();
    bool getIntraOptIdx();
    bool get2TileIdx();
    bool getIdentIdx();
    bool getRarIdx();
    bool getPartTileIdx();
    bool getLbTileIdx();
    int getFuseIdx();
    bool getUnrollIdx();
    int getUnrollFIdx();
    bool getIslIdx();
    unsigned long getNbTuples();
    Matrix getTileOptions();
    Matrix *getTileTuples();

    void setRank(int n);
    void setParaIdx(bool val);
    void setIdentIdx(bool val);
    void setRarIdx(bool val);
    void setFuseIdx(int val);
    void setUnrollIdx(bool val);
    void setUnrollFIdx(int val);
    void setIslIdx(bool val);
    void setTileOptions(int idxTile, int idxIntraOpt, int idx2Tile, int idxPartTile, int idxLbTile, Matrix opts);

private:
    int rank = -1;
    QHBoxLayout *layout;

    QGroupBox *groupAddiOpt;
    QGroupBox *groupFuse;

    QLabel *textRank;
    QCheckBox *checkPara;
    QCheckBox *checkTile;
    QCheckBox *checkIntraOpt;
    QCheckBox *check2Tile;
    QCheckBox *checkIdent;
    QCheckBox *checkRar;
    QCheckBox *checkPartTile;
    QCheckBox *checkLbTile;
    QComboBox *comboFuse;
    QLabel *textFuse;
    QCheckBox *checkUnroll;
    QSpinBox *spinUnrollF;
    QLabel *textUnrollF;
    QCheckBox *checkIsl;

    TileOption *tileOpts;

signals:
    void upClicked(int rank);
    void downClicked(int rank);
    void deleted(int rank);
    void setChanged();


private slots:
    void updateUnrollF(int n);
    void deleteObject();
    void emitUpArrowClicked();
    void emitDownArrowClicked();
    void emitSetChanged();
};

#endif // OptionSet_H
