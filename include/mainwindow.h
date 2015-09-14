#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qconfig.h>

#include <QMainWindow>
#include <QListWidget>
#include <QFileInfo>
#include <QLabel>
#include <QScrollArea>

#include <renderware.h>

#include "defs.h"

struct SystemEventHandlerWidget abstract
{
    ~SystemEventHandlerWidget( void );

    virtual void beginSystemEvent( QEvent *evt ) = 0;
    virtual void endSystemEvent( QEvent *evt ) = 0;
};

#include "texinfoitem.h"
#include "txdlog.h"
#include "txdadddialog.h"

#include "MagicExport.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class TexAddDialog;
    friend class RwVersionDialog;
    friend class TexNameWindow;
    friend class RenderPropWindow;
    friend class TexResizeWindow;
    friend class PlatformSelWindow;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void initializeNativeFormats( void );
    void shutdownNativeFormats( void );

    void UpdateAccessibility( void );

public:
    void setCurrentTXD( rw::TexDictionary *txdObj );
    void updateTextureList( void );

    void updateWindowTitle( void );
    void updateTextureMetaInfo( void );
    void updateAllTextureMetaInfo( void );

    void updateTextureView( void );

    void saveCurrentTXDAt( QString location );

	void clearViewImage( void );
    
    static const char* GetTXDPlatformString( rw::TexDictionary *txd );

    static void SetTXDPlatformString( rw::TexDictionary *txd, const char *platform );

private:
    void DoAddTexture( const TexAddDialog::texAddOperation& params );

    inline void setCurrentFilePath( const QString& newPath )
    {
        this->openedTXDFileInfo = QFileInfo( newPath );
        this->hasOpenedTXDFileInfo = true;

        this->updateWindowTitle();
    }

    inline void clearCurrentFilePath( void )
    {
        this->hasOpenedTXDFileInfo = false;

        this->updateWindowTitle();
    }

public slots:
    void onCreateNewTXD( bool checked );
    void onOpenFile( bool checked );
    void onCloseCurrent( bool checked );

	void onTextureItemChanged(QListWidgetItem *texInfoItem, QListWidgetItem *prevTexInfoItem);

    void onToggleShowMipmapLayers( bool checked );
	void onToggleShowBackground(bool checked);
    void onToggleShowLog( bool checked );
    void onSelectPlatform( bool checked );
    void onSetupMipmapLayers( bool checked );
    void onClearMipmapLayers( bool checked );

    void onRequestSaveTXD( bool checked );
    void onRequestSaveAsTXD( bool checked );

    void onSetupRenderingProps( bool checked );
	void onSetupTxdVersion(bool checked);

private:
    QString requestValidImagePath( void );

public slots:
    void onAddTexture( bool checked );
    void onReplaceTexture( bool checked );
    void onRemoveTexture( bool checked );
    void onRenameTexture( bool checked );
    void onResizeTexture( bool checked );
    void onManipulateTexture( bool checked );
    void onExportTexture( bool checked );

protected:
    void addTextureFormatExportLinkToMenu( QMenu *theMenu, const char *defaultExt, const char *formatName );

private:
    class rwPublicWarningDispatcher : public rw::WarningManagerInterface
    {
    public:
        inline rwPublicWarningDispatcher( MainWindow *theWindow )
        {
            this->mainWnd = theWindow;
        }

        void OnWarning( const std::string& msg ) override
        {
			this->mainWnd->txdLog->addLogMessage(msg.c_str(), LOGMSG_WARNING);
        }

    private:
        MainWindow *mainWnd;
    };

    rwPublicWarningDispatcher rwWarnMan;

    rw::Interface *rwEngine;
    rw::TexDictionary *currentTXD;

    TexInfoWidget *currentSelectedTexture;

    QFileInfo openedTXDFileInfo;
    bool hasOpenedTXDFileInfo;

    QListWidget *textureListWidget;

	QScrollArea *imageView; // we handle full 2d-viewport as a scroll-area
	QLabel *imageWidget;    // we use label to put image on it

    QLabel *txdNameLabel;

    QPushButton *rwVersionButton;

    bool drawMipmapLayers;
	bool showBackground;

    // Accessibility management variables (menu items).
    // FILE MENU.
    QAction *actionNewTXD;
    QAction *actionOpenTXD;
    QAction *actionSaveTXD;
    QAction *actionSaveTXDAs;
    QAction *actionCloseTXD;

    // EDIT MENU.
    QAction *actionAddTexture;
    QAction *actionReplaceTexture;
    QAction *actionRemoveTexture;
    QAction *actionRenameTexture;
    QAction *actionResizeTexture;
    QAction *actionManipulateTexture;
    QAction *actionPlatformSelect;
    QAction *actionSetupMipmaps;
    QAction *actionClearMipmaps;
    QAction *actionRenderProps;
    QAction *actionViewAllChanges;
    QAction *actionCancelAllChanges;
    QAction *actionAllTextures;
    QAction *actionSetupTXDVersion;

    // EXPORT MENU.
    std::list <QAction*> actionsExportImage;

	TxdLog *txdLog; // log management class
    RwVersionDialog *verDlg; // txd version setup class
    TexNameWindow *texNameDlg; // dialog to change texture name
    RenderPropWindow *renderPropDlg; // change a texture's wrapping or filtering
    TexResizeWindow *resizeDlg; // change raster dimensions
    PlatformSelWindow *platformDlg; // set TXD platform

    struct magf_extension
    {
        D3DFORMAT d3dformat;
        HMODULE loadedLibrary;
        void *handler;
    };

    typedef std::list <magf_extension> magf_formats_t;

    magf_formats_t magf_formats;

    // Cache of registered image formats and their interfaces.
    struct registered_image_format
    {
        std::string formatName;
        std::string defaultExt;

        bool isNativeFormat;
        std::string nativeType;
    };

    typedef std::list <registered_image_format> imageFormats_t;

    imageFormats_t reg_img_formats;
};

#endif // MAINWINDOW_H
