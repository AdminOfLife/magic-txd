#pragma once

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include <regex>
#include <string>
#include <sstream>

#include <vector>

#include "qtutils.h"
#include "languages.h"

class RwVersionDialog : public QDialog, public magicTextLocalizationItem
{
	MainWindow *mainWnd;

    QPushButton *applyButton;

public:
    QLineEdit *versionLineEdit;
    QLineEdit *buildLineEdit;

    QComboBox *gameSelectBox;
    QComboBox *platSelectBox;
    QComboBox *dataTypeSelectBox;

private:
    bool GetSelectedVersion( rw::LibraryVersion& verOut ) const
    {
        QString currentVersionString = this->versionLineEdit->text();

        std::string ansiCurrentVersionString = qt_to_ansi( currentVersionString );

        rw::LibraryVersion theVersion;

        bool hasValidVersion = false;

        // Verify whether our version is valid while creating our local version struct.
        unsigned int rwLibMajor, rwLibMinor, rwRevMajor, rwRevMinor;
        bool hasProperMatch = false;
        {
            std::regex ver_regex( "(\\d)\\.(\\d{1,2})\\.(\\d{1,2})\\.(\\d{1,2})" );

            std::smatch ver_match;

            std::regex_match( ansiCurrentVersionString, ver_match, ver_regex );

            if ( ver_match.size() == 5 )
            {
                rwLibMajor = std::stoul( ver_match[ 1 ] );
                rwLibMinor = std::stoul( ver_match[ 2 ] );
                rwRevMajor = std::stoul( ver_match[ 3 ] );
                rwRevMinor = std::stoul( ver_match[ 4 ] );

                hasProperMatch = true;
            }
        }

        if ( hasProperMatch )
        {
            if ( ( rwLibMajor >= 3 && rwLibMajor <= 6 ) &&
                 ( rwLibMinor <= 15 ) &&
                 ( rwRevMajor <= 15 ) && 
                 ( rwRevMinor <= 63 ) )
            {
                theVersion.rwLibMajor = rwLibMajor;
                theVersion.rwLibMinor = rwLibMinor;
                theVersion.rwRevMajor = rwRevMajor;
                theVersion.rwRevMinor = rwRevMinor;

                hasValidVersion = true;
            }
        }

        if ( hasValidVersion )
        {
            // Also set the build number, if valid.
            QString buildNumber = this->buildLineEdit->text();

            std::string ansiBuildNumber = qt_to_ansi( buildNumber );

            unsigned int buildNum;

            int matchCount = sscanf( ansiBuildNumber.c_str(), "%x", &buildNum );

            if ( matchCount == 1 )
            {
                if ( buildNum <= 65535 )
                {
                    theVersion.buildNumber = buildNum;
                }
            }

            // Having an invalid build number does not mean that our version is invalid.
            // The build number is just candy anyway.
        }

        if ( hasValidVersion )
        {
            verOut = theVersion;
        }

        return hasValidVersion;
    }

    void UpdateAccessibility( void )
    {
        rw::LibraryVersion libVer;

        // Check whether we should even enable input.
        // This is only if the user selected "Custom".
        bool shouldAllowInput = ( this->gameSelectBox->currentIndex() == 0 );

        this->versionLineEdit->setDisabled( !shouldAllowInput );
        this->buildLineEdit->setDisabled( !shouldAllowInput );

        bool hasValidVersion = this->GetSelectedVersion( libVer );

        // Alright, set enabled-ness based on valid version.
        this->applyButton->setDisabled( !hasValidVersion );
    }

public slots:
    void OnChangeVersion( const QString& newText )
    {
        // The version must be validated.
        this->UpdateAccessibility();
    }

    void OnChangeSelectedGame(int newIndex)
    {
        if (newIndex >= 0) {
            if (newIndex == 0) { // Custom
                this->platSelectBox->setCurrentIndex(-1);
                this->platSelectBox->setDisabled(true);
                QString lastDataTypeName = this->dataTypeSelectBox->currentText();
                this->dataTypeSelectBox->clear();
                for (int i = 1; i <= RwVersionSets::RWVS_DT_NUM_OF_TYPES; i++) {
                    const char *dataName = RwVersionSets::dataNameFromId((RwVersionSets::eDataType)i);
                    this->dataTypeSelectBox->addItem(dataName);
                    if (lastDataTypeName == dataName)
                        this->dataTypeSelectBox->setCurrentIndex(i - 1);
                }
                this->dataTypeSelectBox->setDisabled(false);
            }
            else {
                this->platSelectBox->clear();
                for (unsigned int i = 0; i < this->mainWnd->versionSets.sets[newIndex - 1].availablePlatforms.size(); i++) {
                    this->platSelectBox->addItem(RwVersionSets::platformNameFromId(
                        this->mainWnd->versionSets.sets[newIndex - 1].availablePlatforms[i].platformType));
                }
                if(this->mainWnd->versionSets.sets[newIndex - 1].availablePlatforms.size() < 2)
                    this->platSelectBox->setDisabled(true);
                else
                    this->platSelectBox->setDisabled(false);
                //this->OnChangeSelecteedPlatform( 0 );
            }
        }
        
        // We want to update the accessibility.
        this->UpdateAccessibility();
    }

    void OnChangeSelecteedPlatform(int newIndex) {
        if (newIndex >= 0) {
            this->dataTypeSelectBox->clear();
            unsigned int set = this->gameSelectBox->currentIndex();

            const RwVersionSets::Set& versionSet = this->mainWnd->versionSets.sets[ set - 1 ];
            const RwVersionSets::Set::Platform& platformOfSet = versionSet.availablePlatforms[ newIndex ];

            for (unsigned int i = 0; i < platformOfSet.availableDataTypes.size(); i++) {
                this->dataTypeSelectBox->addItem(
                    RwVersionSets::dataNameFromId(platformOfSet.availableDataTypes[i])
                );
            }
            if (platformOfSet.availableDataTypes.size() < 2)
                this->dataTypeSelectBox->setDisabled(true);
            else
                this->dataTypeSelectBox->setDisabled(false);

            std::string verString =
                std::to_string(platformOfSet.version.rwLibMajor) + "." +
                std::to_string(platformOfSet.version.rwLibMinor) + "." +
                std::to_string(platformOfSet.version.rwRevMajor) + "." +
                std::to_string(platformOfSet.version.rwRevMinor);

            std::string buildString;

            if (platformOfSet.version.buildNumber != 0xFFFF)
            {
                std::stringstream hex_stream;

                hex_stream << std::hex << platformOfSet.version.buildNumber;

                buildString = hex_stream.str();
            }

            this->versionLineEdit->setText(ansi_to_qt(verString));
            this->buildLineEdit->setText(ansi_to_qt(buildString));
        }
    }

    void OnRequestAccept( bool clicked )
    {
        // Set the version and close.
        rw::LibraryVersion libVer;

        bool hasVersion = this->GetSelectedVersion( libVer );

        if ( !hasVersion )
            return;

        // Set the version of the entire TXD.
        // Also patch the platform if feasible.
        if ( rw::TexDictionary *currentTXD = this->mainWnd->currentTXD )
        {
            int set = this->gameSelectBox->currentIndex();
            int platform = this->platSelectBox->currentIndex();
            int dataType = this->dataTypeSelectBox->currentIndex();

            RwVersionSets::eDataType dataTypeId;

            if (set == 0) // Custom
                dataTypeId = (RwVersionSets::eDataType)(dataType + 1);
            else
                dataTypeId = this->mainWnd->versionSets.sets[set - 1].availablePlatforms[platform].availableDataTypes[dataType];

            currentTXD->SetEngineVersion(libVer);

            // Maybe make SetEngineVersion sets the version for all children objects?
            if (currentTXD->numTextures > 0) {
                for (rw::TexDictionary::texIter_t iter(currentTXD->GetTextureIterator()); !iter.IsEnd(); iter.Increment())
                    iter.Resolve()->SetEngineVersion(libVer);
            }

            QString previousPlatform = this->mainWnd->GetCurrentPlatform();
            QString currentPlatform = RwVersionSets::dataNameFromId(dataTypeId);

            // If platform was changed
            if (previousPlatform != currentPlatform) {
                this->mainWnd->SetRecommendedPlatform(currentPlatform);
                this->mainWnd->ChangeTXDPlatform(currentTXD, currentPlatform);

                // The user might want to be notified of the platform change.
                this->mainWnd->txdLog->addLogMessage(QString("changed the TXD platform to match version (") + previousPlatform + 
                    QString(">") + currentPlatform + QString(")"), LOGMSG_INFO);

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

    void OnRequestCancel( bool clicked )
    {
        this->close();
    }

public:
	RwVersionDialog( MainWindow *mainWnd ) : QDialog( mainWnd )
    {
		setObjectName("background_1");
        setWindowFlags( this->windowFlags() & ~Qt::WindowContextHelpButtonHint );
        setAttribute( Qt::WA_DeleteOnClose );

        setWindowModality( Qt::WindowModal );

        this->mainWnd = mainWnd;

        MagicLayout<QVBoxLayout> layout(this);

        /************* Set ****************/
		QHBoxLayout *selectGameLayout = new QHBoxLayout;
		QLabel *gameLabel = CreateLabelL( "Main.SetupTV.Set" );
		gameLabel->setObjectName("label25px");
		QComboBox *gameComboBox = new QComboBox;
        gameComboBox->setFixedWidth(300);
		gameComboBox->addItem(getLanguageItemByKey("Main.SetupTV.Custom"));   /// HAXXXXXXX
        for (unsigned int i = 0; i < this->mainWnd->versionSets.sets.size(); i++)
            gameComboBox->addItem(this->mainWnd->versionSets.sets[i].name);
        this->gameSelectBox = gameComboBox;

        connect( gameComboBox, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, &RwVersionDialog::OnChangeSelectedGame );

		selectGameLayout->addWidget(gameLabel);
		selectGameLayout->addWidget(gameComboBox);

        /************* Platform ****************/
        QHBoxLayout *selectPlatformLayout = new QHBoxLayout;
        QLabel *platLabel = CreateLabelL("Main.SetupTV.Plat");
        platLabel->setObjectName("label25px");
        QComboBox *platComboBox = new QComboBox;
        platComboBox->setFixedWidth(300);
        this->platSelectBox = platComboBox;

        connect(platComboBox, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, &RwVersionDialog::OnChangeSelecteedPlatform);

        selectPlatformLayout->addWidget(platLabel);
        selectPlatformLayout->addWidget(platComboBox);

        /************* Data type ****************/
        QHBoxLayout *selectDataTypeLayout = new QHBoxLayout;
        QLabel *dataTypeLabel = CreateLabelL( "Main.SetupTV.Data" );
        dataTypeLabel->setObjectName("label25px");
        QComboBox *dataTypeComboBox = new QComboBox;
        dataTypeComboBox->setFixedWidth(300);
        this->dataTypeSelectBox = dataTypeComboBox;

        selectDataTypeLayout->addWidget(dataTypeLabel);
        selectDataTypeLayout->addWidget(dataTypeComboBox);

		QHBoxLayout *versionLayout = new QHBoxLayout;
		QLabel *versionLabel = CreateFixedWidthLabelL( "Main.SetupTV.Version", 25 );
		versionLabel->setObjectName("label25px");

		QHBoxLayout *versionNumbersLayout = new QHBoxLayout;
		QLineEdit *versionLine1 = new QLineEdit;

		versionLine1->setInputMask("0.00.00.00");
        versionLine1->setFixedWidth(80);

		versionNumbersLayout->addWidget(versionLine1);

		versionNumbersLayout->setMargin(0);

        this->versionLineEdit = versionLine1;

        connect( versionLine1, &QLineEdit::textChanged, this, &RwVersionDialog::OnChangeVersion );

		QLabel *buildLabel = CreateFixedWidthLabelL( "Main.SetupTV.Build", 25 );
		buildLabel->setObjectName("label25px");
		QLineEdit *buildLine = new QLineEdit;
		buildLine->setInputMask(">HHHH");
        buildLine->clear();
		buildLine->setFixedWidth(60);

        this->buildLineEdit = buildLine;

		versionLayout->addWidget(versionLabel);
		versionLayout->addLayout(versionNumbersLayout);
		versionLayout->addWidget(buildLabel);
		versionLayout->addWidget(buildLine);

        versionLayout->setAlignment(Qt::AlignRight);

        layout.top->addLayout(selectGameLayout);
        layout.top->addLayout(selectPlatformLayout);
        layout.top->addLayout(selectDataTypeLayout);
        layout.top->addSpacing(8);
        layout.top->addLayout(versionLayout);

		QPushButton *buttonAccept = CreateButtonL( "Main.SetupTV.Accept" );
		QPushButton *buttonCancel = CreateButtonL( "Main.SetupTV.Cancel" );

        this->applyButton = buttonAccept;

        connect( buttonAccept, &QPushButton::clicked, this, &RwVersionDialog::OnRequestAccept );
        connect( buttonCancel, &QPushButton::clicked, this, &RwVersionDialog::OnRequestCancel );

		layout.bottom->addWidget(buttonAccept);
        layout.bottom->addWidget(buttonCancel);
		
        // Initiate the ready dialog.
        this->OnChangeSelectedGame( 0 );

        RegisterTextLocalizationItem( this );
	}

    ~RwVersionDialog( void )
    {
        UnregisterTextLocalizationItem( this );

        // There can only be one version dialog.
        this->mainWnd->verDlg = NULL;
    }

    void updateContent( MainWindow *mainWnd ) override
    {
        this->setWindowTitle( getLanguageItemByKey( "Main.SetupTV.Desc" ) );
    }

    void SelectGame(unsigned int gameId) {
        this->gameSelectBox->setCurrentIndex(gameId);
    }
};