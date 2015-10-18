#pragma once

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>

// The mass export window is a simple dialog that extracts the contents of multiple TXD files into a directory.
// It is a convenient way of dumping all image files, even from IMG containers.
struct MassExportWindow : public QDialog
{
    friend struct massexportEnv;

    MassExportWindow( MainWindow *mainWnd );
    ~MassExportWindow( void );

public slots:
    void OnRequestExport( bool checked );
    void OnRequestCancel( bool checked );

private:
    void serialize( void );

    MainWindow *mainWnd;

    QLineEdit *editGameRoot;
    QLineEdit *editOutputRoot;
    QComboBox *boxRecomImageFormat;
    QRadioButton *optionExportPlain;
    QRadioButton *optionExportTXDName;
    QRadioButton *optionExportFolders;

    RwListEntry <MassExportWindow> node;
};