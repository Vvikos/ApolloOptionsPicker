#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Window);
    this->setWindowState(Qt::WindowMaximized);
    this->setWindowTitle("Apollo Options Picker");
    this->setWindowIcon(QIcon(":/images/app_image.png"));

    // Creation du gestionnaire de tab
    dashboard = new Dashboard(ui->infoText, ui->progressBar, this);
    ui->DashboardLayout->addWidget(dashboard);

    // Bouton d'ajout de set d'options
    // Bouton d'ajout d'un tab
    connect(dashboard, SIGNAL(tabCloseRequested(int)), this, SLOT(close(int)));
    // Fichier menu section
    connect(ui->actionNouveau, SIGNAL(triggered()), dashboard, SLOT(appendTab()));
    connect(ui->actionOuvrir, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionEnregistrer, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionEnregistrer_sous, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionExporter, SIGNAL(triggered()), this, SLOT(exportCsv()));
    connect(ui->actionFermer, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionQuitter, SIGNAL(triggered()), qApp, SLOT(quit()));
    // Editer menu section
    connect(ui->actionAjouter_ligne, SIGNAL(triggered()), dashboard, SLOT(addOption()));
    // Options menu section
    connect(ui->actionMode_nuit, SIGNAL(toggled(bool)), this, SLOT(switchMode(bool)));
    // Aide menu section
    connect(ui->actionA_propos, SIGNAL(triggered()), this, SLOT(about()));

    QSettings settings;
    switchMode(settings.value("nightMode").toBool());
    ui->actionMode_nuit->setChecked(settings.value("nightMode").toBool());

    lastSavePath = QDir::currentPath();
    lastOpenPath = QDir::currentPath();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setInfoText(QString s) {
    ui->infoText->setText(s+"  ");
}

// SLOTS
void MainWindow::open(){
    QString path = QFileDialog::getOpenFileName(this,
        tr("Open file"), lastOpenPath, tr("All Files (*) ;;Text files (*.csv *.txt)"));

    if(path.isEmpty())
        return;

    dashboard->loadFileCurrent(path);
    lastOpenPath = path;
}

void MainWindow::close(int rank){
    dashboard->setCurrentIndex(rank);
    close();
}

void MainWindow::close(){
    if(dashboard->getCurrentCount() > 0){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Save", "Content of file has been changed, save changes?",
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
            save();
    }
    dashboard->closeCurrent();
}

void MainWindow::save(){
    QString path = dashboard->getCurrentPath();

    // get save path if not already defined
    if (path=="")
        path = QFileDialog::getSaveFileName(this, "Choose file path", lastSavePath, "All Files (*);; Text files (*.txt *.csv)");

    if (path.isEmpty())
        return;

    dashboard->saveCurrent(path);
    lastSavePath = path;
}

void MainWindow::saveAs(){
    QString path = QFileDialog::getSaveFileName(this, "Choose file path", lastSavePath, "All Files (*);; Text files (*.txt *.csv)" );

    if (path.isEmpty())
        return;

    dashboard->saveCurrent(path);
    lastSavePath = path;
}

void MainWindow::exportCsv(){
    if (dashboard->count() <= 1)
        return;

    QString path = QFileDialog::getSaveFileName(this, "Choose file path", lastSavePath, "All Files (*);; Text files (*.txt *.csv)" );

    if (path.isEmpty())
        return;

    dashboard->exportCurrent(path);
    lastSavePath = path;
}

void MainWindow::switchMode(bool val){
    QString stylePath;
    if (val)
        stylePath = QString(":/style/style_night.qss");
    else
        stylePath = QString(":/style/style_light.qss");

    QFile base(":/style/style_base.qss");
    QFile file(stylePath);

    base.open(QFile::ReadOnly);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(base.readAll() + "\n" + file.readAll());

    qApp->setStyleSheet(styleSheet);
    QSettings settings;
    settings.setValue("nightMode", val);
    file.close();
    base.close();
}

void MainWindow::about(){
    QMessageBox msgBox;
    msgBox.setWindowTitle("About");
    msgBox.setText("This software is meant to give Apollo users a way to create sets of options which can be used in the Apollo compiler."
                   "\n\nIt has been developped by Victor VOGT during a university project supervised by Philipe CLAUSS (member of Apollo crew)."
                   "\n\nYou can find documentation about Apollo project here :"
                   "\n      https://webpages.gitlabpages.inria.fr/apollo/about"
                   "\n\nIt comes bundled with apollo and is made to be used with it."
                   "\n\n\nList of capabilities that can be done with it :"
                   "\n\n    -   Create and manage different files, every tab represents one file. A file can be saved and re-opened in another instance of Apollo Options Picker or/and exported to be used with Apollo."
                   "\n\n    -   Add or remove rows of options and for every row configure loop depth and tile sizes to be tested for each loop depth."
                   "\n\n    -   Every option set created will generate one or more options for Apollo, more precisly it creates a number of options equivalant to the number of different combinaisons of tile sizes."
                   "\n\n    -   Dark mode available."
                   "\n\nWARING: You can easily create files that will generate billions of options and therefore that will possibly slow down your computer, do not be too greedy."
                   "\n\nThank you."
                   );
    msgBox.exec();
}

