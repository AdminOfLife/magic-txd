#include "mainwindow.h"
#include "rwversiondialog.h"

RwVersionDialog::RwVersionDialog( MainWindow *mainWnd ) : versionGUI( mainWnd, this ), QDialog( mainWnd )
{
	setObjectName("background_1");
    setWindowFlags( this->windowFlags() & ~Qt::WindowContextHelpButtonHint );
    setAttribute( Qt::WA_DeleteOnClose );

    setWindowModality( Qt::WindowModal );

    this->mainWnd = mainWnd;

    MagicLayout<QVBoxLayout> layout(this);

    layout.top->addLayout( this->versionGUI.GetVersionRootLayout() );

	QPushButton *buttonAccept = CreateButtonL( "Main.SetupTV.Accept" );
	QPushButton *buttonCancel = CreateButtonL( "Main.SetupTV.Cancel" );

    this->applyButton = buttonAccept;

    connect( buttonAccept, &QPushButton::clicked, this, &RwVersionDialog::OnRequestAccept );
    connect( buttonCancel, &QPushButton::clicked, this, &RwVersionDialog::OnRequestCancel );

	layout.bottom->addWidget(buttonAccept);
    layout.bottom->addWidget(buttonCancel);
		
    // Initiate the ready dialog.
    this->versionGUI.InitializeVersionSelect();

    RegisterTextLocalizationItem( this );
}

RwVersionDialog::~RwVersionDialog( void )
{
    UnregisterTextLocalizationItem( this );

    // There can only be one version dialog.
    this->mainWnd->verDlg = NULL;
}

void RwVersionDialog::updateContent( MainWindow *mainWnd )
{
    // Update localization items.
    this->setWindowTitle( getLanguageItemByKey( "Main.SetupTV.Desc" ) );
}

void RwVersionDialog::UpdateAccessibility( void )
{
    rw::LibraryVersion libVer;

    // Check whether we should allow setting this version.
    bool hasValidVersion = this->versionGUI.GetSelectedVersion( libVer );

    // Alright, set enabled-ness based on valid version.
    this->applyButton->setDisabled( !hasValidVersion );
}

void RwVersionDialog::OnRequestAccept( bool clicked )
{
    // Set the version and close.
    rw::LibraryVersion libVer;

    bool hasVersion = this->versionGUI.GetSelectedVersion( libVer );

    if ( !hasVersion )
        return;

    // Set the version of the entire TXD.
    // Also patch the platform if feasible.
    if ( rw::TexDictionary *currentTXD = this->mainWnd->currentTXD )
    {
        int set = this->versionGUI.gameSelectBox->currentIndex();
        int platform = this->versionGUI.platSelectBox->currentIndex();
        int dataType = this->versionGUI.dataTypeSelectBox->currentIndex();

        RwVersionSets::eDataType dataTypeId;

        if (set == 0) // Custom
        {
            dataTypeId = (RwVersionSets::eDataType)(dataType + 1);
        }
        else
        {
            const RwVersionSets::Set& selectedSet = this->mainWnd->versionSets.sets[set - 1];
            const RwVersionSets::Set::Platform& selectedPlatform = selectedSet.availablePlatforms[platform];

            dataTypeId = selectedPlatform.availableDataTypes[dataType];
        }

        // todo: Maybe make SetEngineVersion sets the version for all children objects?
        currentTXD->SetEngineVersion(libVer);

        if (currentTXD->numTextures > 0) {
            for (rw::TexDictionary::texIter_t iter(currentTXD->GetTextureIterator()); !iter.IsEnd(); iter.Increment())
            {
                iter.Resolve()->SetEngineVersion(libVer);
            }
        }

        QString previousPlatform = this->mainWnd->GetCurrentPlatform();
        QString currentPlatform = RwVersionSets::dataNameFromId(dataTypeId);

        // If platform was changed
        if (previousPlatform != currentPlatform)
        {
            this->mainWnd->SetRecommendedPlatform(currentPlatform);
            this->mainWnd->ChangeTXDPlatform(currentTXD, currentPlatform);

            // The user might want to be notified of the platform change.
            this->mainWnd->txdLog->addLogMessage(
                QString("changed the TXD platform to match version (") + previousPlatform + 
                QString(">") + currentPlatform + QString(")"),
                LOGMSG_INFO
            );

            // Also update texture item info, because it may have changed.
            this->mainWnd->updateAllTextureMetaInfo();

            // The visuals of the texture _may_ have changed.
            this->mainWnd->updateTextureView();
        }

        // Done. :)
    }

    // Update the MainWindow stuff.
    this->mainWnd->updateWindowTitle();

    // Since the version has changed, the friendly icons should have changed.
    this->mainWnd->updateFriendlyIcons();

    this->close();
}

void RwVersionDialog::OnRequestCancel( bool clicked )
{
    this->close();
}

void RwVersionDialog::updateVersionConfig()
{
    MainWindow *mainWnd = this->mainWnd;

    // Try to find a set for current txd version
    bool setFound = false;

    if (rw::TexDictionary *currentTXD = mainWnd->getCurrentTXD())
    {
        rw::LibraryVersion version = currentTXD->GetEngineVersion();

        QString platformName = mainWnd->GetCurrentPlatform();

        if (!platformName.isEmpty())
        {
            RwVersionSets::eDataType platformDataTypeId = RwVersionSets::dataIdFromEnginePlatformName(platformName);

            if (platformDataTypeId != RwVersionSets::RWVS_DT_NOT_DEFINED)
            {
                int setIndex, platformIndex, dataTypeIndex;

                if (mainWnd->versionSets.matchSet(version, platformDataTypeId, setIndex, platformIndex, dataTypeIndex))
                {
                    this->versionGUI.gameSelectBox->setCurrentIndex(setIndex + 1);
                    this->versionGUI.platSelectBox->setCurrentIndex(platformIndex);
                    this->versionGUI.dataTypeSelectBox->setCurrentIndex(dataTypeIndex);

                    setFound = true;
                }
            }
        }
    }

    if (!setFound)
    {
        if (this->versionGUI.gameSelectBox->currentIndex() != 0)
        {
            this->versionGUI.gameSelectBox->setCurrentIndex(0);
        }
        else
        {
            this->versionGUI.InitializeVersionSelect();
        }

        if (rw::TexDictionary *currentTXD = mainWnd->getCurrentTXD())
        {
            const rw::LibraryVersion& txdVersion = currentTXD->GetEngineVersion();

            std::string verString = rwVersionToString( txdVersion );
            std::string buildString;

            if (txdVersion.buildNumber != 0xFFFF)
            {
                std::stringstream hex_stream;

                hex_stream << std::hex << txdVersion.buildNumber;

                buildString = hex_stream.str();
            }

            this->versionGUI.versionLineEdit->setText(ansi_to_qt(verString));
            this->versionGUI.buildLineEdit->setText(ansi_to_qt(buildString));
        }
    }
}