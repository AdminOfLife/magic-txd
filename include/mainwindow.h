#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qconfig.h>

#include <QMainWindow>
#include <QListWidget>
#include <QFileInfo>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QAction>
#include <QMessageBox>

#include <renderware.h>

#include <sdk/MemoryUtils.h>

#include <CFileSystemInterface.h>
#include <CFileSystem.h>

#define NUMELMS(x)      ( sizeof(x) / sizeof(*x) )

#include "defs.h"

class MainWindow;

#include "versionsets.h"
#include "textureviewport.h"

#include "testmessage.h"

struct SystemEventHandlerWidget abstract
{
    ~SystemEventHandlerWidget( void );

    virtual void beginSystemEvent( QEvent *evt ) = 0;
    virtual void endSystemEvent( QEvent *evt ) = 0;
};

// Global conversion from QString to c-str and other way round.
inline std::string qt_to_ansi( const QString& str )
{
    QByteArray charBuf = str.toLatin1();

    return std::string( charBuf.data(), charBuf.size() );
}

inline QString ansi_to_qt( const std::string& str )
{
    return QString::fromLatin1( str.c_str(), str.size() );
}

#include "texinfoitem.h"
#include "txdlog.h"
#include "txdadddialog.h"
#include "rwfswrap.h"
#include "guiserialization.h"
#include "aboutdialog.h"
#include "streamcompress.h"
#include "createtxddlg.h"

#include "MagicExport.h"

#define _FEATURES_NOT_IN_CURRENT_RELEASE

class MainWindow : public QMainWindow, public magicTextLocalizationItem
{
    friend class TexAddDialog;
    friend class RwVersionDialog;
    friend class TexNameWindow;
    friend class RenderPropWindow;
    friend class TexResizeWindow;
    friend class PlatformSelWindow;
    friend class ExportAllWindow;
    friend class AboutDialog;
    friend class OptionsDialog;
    friend class mainWindowSerializationEnv;
    friend class CreateTxdDialog;

public:
    MainWindow(QString appPath, rw::Interface *rwEngine, CFileSystem *fsHandle, QWidget *parent = 0);
    ~MainWindow();

    void updateContent( MainWindow *mainWnd );

private:
    void initializeNativeFormats(void);
    void shutdownNativeFormats(void);

    void UpdateExportAccessibility(void);
    void UpdateAccessibility(void);

public:
    void openTxdFile(QString fileName);
    void setCurrentTXD(rw::TexDictionary *txdObj);
    void updateTextureList(bool selectLastItemInList);

    void updateFriendlyIcons();

public:
    void updateWindowTitle(void);
    void updateTextureMetaInfo(void);
    void updateAllTextureMetaInfo(void);

    void updateTextureView(void);

    void updateTextureViewport(void);

    void saveCurrentTXDAt(QString location);

    void clearViewImage(void);

    rw::Interface* GetEngine(void) { return this->rwEngine; }

    QString GetCurrentPlatform();

    void SetCurrentPlatform(QString platform);

    void ChangeTXDPlatform(rw::TexDictionary *txd, QString platform);

    QString GetTXDPlatform(rw::TexDictionary *txd);

private:
    void DoAddTexture(const TexAddDialog::texAddOperation& params);

    inline void setCurrentFilePath(const QString& newPath)
    {
        this->openedTXDFileInfo = QFileInfo(newPath);
        this->hasOpenedTXDFileInfo = true;

        this->updateWindowTitle();
    }

    inline void clearCurrentFilePath(void)
    {
        this->hasOpenedTXDFileInfo = false;

        this->updateWindowTitle();
    }

    public slots:
    void onCreateNewTXD(bool checked);
    void onOpenFile(bool checked);
    void onCloseCurrent(bool checked);

    void onTextureItemChanged(QListWidgetItem *texInfoItem, QListWidgetItem *prevTexInfoItem);

    void onToggleShowFullImage(bool checked);
    void onToggleShowMipmapLayers(bool checked);
    void onToggleShowBackground(bool checked);
    void onToggleShowLog(bool checked);
    void onSetupMipmapLayers(bool checked);
    void onClearMipmapLayers(bool checked);

    void onRequestSaveTXD(bool checked);
    void onRequestSaveAsTXD(bool checked);

    void onSetupRenderingProps(bool checked);
    void onSetupTxdVersion(bool checked);
    void onShowOptions(bool checked);

    void onRequestMassConvert(bool checked);
    void onRequestMassExport(bool checked);
    void onRequestMassBuild(bool checked);

    void onToogleDarkTheme(bool checked);
    void onToogleLightTheme(bool checked);

    void onRequestOpenWebsite(bool checked);
    void onAboutUs(bool checked);

private:
    QString requestValidImagePath(void);

    public slots:
    void onAddTexture(bool checked);
    void onReplaceTexture(bool checked);
    void onRemoveTexture(bool checked);
    void onRenameTexture(bool checked);
    void onResizeTexture(bool checked);
    void onManipulateTexture(bool checked);
    void onExportTexture(bool checked);
    void onExportAllTextures(bool checked);

protected:
    void addTextureFormatExportLinkToMenu(QMenu *theMenu, const char *displayName, const char *defaultExt, const char *formatName);

private:
    class rwPublicWarningDispatcher : public rw::WarningManagerInterface
    {
    public:
        inline rwPublicWarningDispatcher(MainWindow *theWindow)
        {
            this->mainWnd = theWindow;
        }

        void OnWarning(std::string&& msg) override
        {
            this->mainWnd->txdLog->addLogMessage(ansi_to_qt(msg), LOGMSG_WARNING);
        }

    private:
        MainWindow *mainWnd;
    };

    rwPublicWarningDispatcher rwWarnMan;

    rw::Interface *rwEngine;
    rw::TexDictionary *currentTXD;

    TexInfoWidget *currentSelectedTexture;

    RwVersionSets versionSets;

    QFileInfo openedTXDFileInfo;
    bool hasOpenedTXDFileInfo;

    QString newTxdName;

    QString currentTxdPlatform;

    QListWidget *textureListWidget;

    TexViewportWidget *imageView; // we handle full 2d-viewport as a scroll-area
    QLabel *imageWidget;    // we use label to put image on it

    QLabel *txdNameLabel;

    QPushButton *rwVersionButton;

    QMovie *starsMovie;

    QSplitter *mainSplitter;

    bool showFullImage;
    bool drawMipmapLayers;
    bool showBackground;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *toolsMenu;
    QMenu *exportMenu;
    QMenu *viewMenu;
    QMenu *infoMenu;

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
    QAction *actionSetupMipmaps;
    QAction *actionClearMipmaps;
    QAction *actionRenderProps;
#ifndef _FEATURES_NOT_IN_CURRENT_RELEASE
    QAction *actionViewAllChanges;
    QAction *actionCancelAllChanges;
    QAction *actionAllTextures;
#endif //_FEATURES_NOT_IN_CURRENT_RELEASE
    QAction *actionSetupTXDVersion;
    QAction *actionShowOptions;
    QAction *actionThemeDark;
    QAction *actionThemeLight;

    QHBoxLayout *friendlyIconRow;
    QLabel *friendlyIconGame;
    QWidget *friendlyIconSeparator;
    QLabel *friendlyIconPlatform;
    
    bool bShowFriendlyIcons;

    bool recheckingThemeItem;

    // EXPORT MENU.
    class TextureExportAction : public QAction
    {
    public:
        TextureExportAction( QString&& defaultExt, QString&& displayName, QString&& formatName, QWidget *parent ) : QAction( QString( "&" ) + displayName, parent )
        {
            this->defaultExt = defaultExt;
            this->displayName = displayName;
            this->formatName = formatName;
        }

        QString defaultExt;
        QString displayName;
        QString formatName;
    };

    std::list <TextureExportAction*> actionsExportItems;
    QAction *exportAllImages;

	TxdLog *txdLog; // log management class
    RwVersionDialog *verDlg; // txd version setup class
    TexNameWindow *texNameDlg; // dialog to change texture name
    RenderPropWindow *renderPropDlg; // change a texture's wrapping or filtering
    TexResizeWindow *resizeDlg; // change raster dimensions
    PlatformSelWindow *platformDlg; // set TXD platform
    AboutDialog *aboutDlg;  // about us. :-)
    QDialog *optionsDlg;    // many options.

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

        std::list <std::string> ext_array;

        bool isNativeFormat;
        std::string nativeType;
    };

    typedef std::list <registered_image_format> imageFormats_t;

    imageFormats_t reg_img_formats;

public:
    QString m_appPath;
    QString m_appPathForStyleSheet;

    // Use this if you need to get a path relatively to app directory
    QString makeAppPath(QString subPath);

    // NOTE: there are multiple ways to get absolute path to app directory coded in this editor!

public:
    CFileSystem *fileSystem;

    // Serialization properties.
    QString lastTXDOpenDir;     // maybe.
    QString lastTXDSaveDir;
    QString lastImageFileOpenDir;

    bool addImageGenMipmaps;
    bool lockDownTXDPlatform;
    bool adjustTextureChunksOnImport;
    bool texaddViewportFill;
    bool texaddViewportScaled;
    bool texaddViewportBackground;
    
    // Options.
    bool showLogOnWarning;
    bool showGameIcon;

    QString lastLanguageFileName;

    // ExportAllWindow
    std::string lastUsedAllExportFormat;
    std::wstring lastAllExportTarget;
};

typedef StaticPluginClassFactory <MainWindow> mainWindowFactory_t;

extern mainWindowFactory_t mainWindowFactory;

#endif // MAINWINDOW_H
