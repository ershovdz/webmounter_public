#include "jmm_view.h"
#include "common_stuff.h"

#include "webmounter.h"

namespace Ui
{
	JmmGalleryView::JmmGalleryView(const Data::PluginSettings& settings, QWidget *parent)
		: QWidget(parent)
	{
		_nameLabel = new QLabel(tr("Login:"));
		_nameLabel->setTextFormat(Qt::RichText);
		_nameEdit = new QLineEdit;
		_nameEdit->setText(settings.userName);
		
		_passwordLabel = new QLabel(tr("Password:"));
		_passwordEdit = new QLineEdit;
		
		_startPluginButton = new QPushButton(tr("Connect"));
		_stopPluginButton = new QPushButton(tr("Disconnect"));

		_stopPluginButton->setEnabled(false);

		_fullSyncRadioButton = new QRadioButton(tr("Full sync (download all photos)"));
		_partSyncRadioButton = new QRadioButton(tr("Partial sync (sync list of elements only)"));

		_partSyncRadioButton->setChecked(!settings.bFullSync);
		_fullSyncRadioButton->setChecked(settings.bFullSync);

		_autoSyncCheckBox  = new QCheckBox(tr("Auto Sync"));
		_startSyncButton = new QPushButton(tr("Start sync"));
		_stopSyncButton  = new QPushButton(tr("Stop sync"));

		_autoSyncCheckBox->setChecked(settings.bAutoSync);

		_startSyncButton->setEnabled(settings.bAutoSync);
		_stopSyncButton->setEnabled(settings.bAutoSync);

		_syncPeriodLabel = new QLabel(tr("Frequency synchronization:"));
		_syncPeriodBox = new QComboBox();

		_syncPeriodBox->setEditable(false);
		_syncPeriodBox->addItem(tr("30 sec"));
		_syncPeriodBox->addItem(tr("1 min"));
		_syncPeriodBox->addItem(tr("5 min"));
		_syncPeriodBox->addItem(tr("10 min"));
		_syncPeriodBox->addItem(tr("30 min"));
		_syncPeriodBox->addItem(tr("1 hour"));
		_syncPeriodBox->addItem(tr("2 hour"));
		_syncPeriodBox->addItem(tr("6 hour"));
		_syncPeriodBox->addItem(tr("12 hour"));
		_syncPeriodBox->addItem(tr("24 hour"));

		if(settings.syncPeriod == "30")
		{
			_syncPeriodBox->setCurrentIndex(0);
		}
		else if(settings.syncPeriod == "60")
		{
			_syncPeriodBox->setCurrentIndex(1);
		}
		else if(settings.syncPeriod == "300")
		{
			_syncPeriodBox->setCurrentIndex(2);
		}
		else if(settings.syncPeriod == "600")
		{
			_syncPeriodBox->setCurrentIndex(3);
		}
		else if(settings.syncPeriod == "1800")
		{
			_syncPeriodBox->setCurrentIndex(4);
		}
		else if(settings.syncPeriod == "3600")
		{
			_syncPeriodBox->setCurrentIndex(5);
		}
		else if(settings.syncPeriod == "7200")
		{
			_syncPeriodBox->setCurrentIndex(6);
		}
		else if(settings.syncPeriod == "21600")
		{
			_syncPeriodBox->setCurrentIndex(7);
		}
		else if(settings.syncPeriod == "43200")
		{
			_syncPeriodBox->setCurrentIndex(8);
		}
		else if(settings.syncPeriod == "86400")
		{
			_syncPeriodBox->setCurrentIndex(9);
		}

		_syncPeriodLabel->setEnabled(settings.bAutoSync);
		_syncPeriodBox->setEnabled(settings.bAutoSync);

		_syncPeriodBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		_authGroup = new QGroupBox(tr("Joomla (Gallery) plugin settings"));
		_authLayout = new QGridLayout;
		_authLayout->addWidget(_nameLabel, 0, 0);
		_authLayout->addWidget(_nameEdit, 0, 1);
		_authLayout->addWidget(_passwordLabel, 1, 0);
		_authLayout->addWidget(_passwordEdit, 1, 1);
		_authGroup->setLayout(_authLayout);

		_syncGroup = new QGroupBox(tr("Sync"));
		_syncLayout = new QGridLayout;


		_syncLayout->addWidget(_fullSyncRadioButton, 0, 0);
		_syncLayout->addWidget(_partSyncRadioButton, 0, 1);
		_syncLayout->addWidget(_autoSyncCheckBox, 1, 0);
		_syncLayout->addWidget(_syncPeriodLabel, 2, 0);
		_syncLayout->addWidget(_syncPeriodBox, 2, 1);
		_syncLayout->addWidget(_startSyncButton, 3, 0);
		_syncLayout->addWidget(_stopSyncButton, 3, 1);
		_syncGroup->setLayout(_syncLayout);

		_buttonGroup = new QGroupBox();
		_buttonLayout = new QGridLayout;
		_buttonLayout->addWidget(_startPluginButton, 0, 0);
		_buttonLayout->addWidget(_stopPluginButton, 0, 1);
		_buttonGroup->setLayout(_buttonLayout);

		_progressGroup = new QGroupBox(tr("Status:"));
		_progressLayout = new QGridLayout;
		_statusLabel = new QLabel(tr("Plugin status:"));
		_statusValue = new QLabel(tr("<font color=\"red\">Not connected</font>"));
		_statusValue->setTextFormat(Qt::RichText);

		_progressBarLabel = new QLabel(tr("Progress:"));
		_progressBar = new QProgressBar();
		_progressBar->setValue(0);

		_progressLayout->addWidget(_statusLabel, 0,0);
		_progressLayout->addWidget(_statusValue, 0,1);
		_progressLayout->addWidget(_progressBarLabel, 1,0);
		_progressLayout->addWidget(_progressBar, 2,0);
		_progressGroup->setLayout(_progressLayout);


		_mainLayout = new QVBoxLayout;
		_mainLayout->addWidget(_authGroup);
		_mainLayout->addWidget(_syncGroup);
		_mainLayout->addSpacing(12);
		_mainLayout->addWidget(_progressGroup);
		_mainLayout->addSpacing(12);
		_mainLayout->addWidget(_buttonGroup);
		_mainLayout->addStretch(1);
		setLayout(_mainLayout);

		_driver = Common::WebMounter::getPlugin("jmmg");

                //[YE] Tmp
/*		if(_driver)
		{
			_driver->registerView(this);
		}
*/
	}

	void JmmGalleryView::changeLang()
	{
		_nameLabel->setText(tr("Login:"));
		_passwordLabel->setText(tr("Password:"));
		
		_startPluginButton->setText(tr("Connect"));
		_stopPluginButton->setText(tr("Disconnect"));

		_fullSyncRadioButton->setText(tr("Full sync (download all photos)"));
		_partSyncRadioButton->setText(tr("Partial sync (sync list of elements only)"));

		
		_autoSyncCheckBox->setText(tr("Auto Sync"));
		_startSyncButton->setText(tr("Start sync"));
		_stopSyncButton->setText(tr("Stop sync"));

		_syncPeriodLabel->setText(tr("Frequency synchronization:"));
		
		int index = _syncPeriodBox->currentIndex();
		
		_syncPeriodBox->clear();
		_syncPeriodBox->addItem(tr("30 sec"));
		_syncPeriodBox->addItem(tr("1 min"));
		_syncPeriodBox->addItem(tr("5 min"));
		_syncPeriodBox->addItem(tr("10 min"));
		_syncPeriodBox->addItem(tr("30 min"));
		_syncPeriodBox->addItem(tr("1 hour"));
		_syncPeriodBox->addItem(tr("2 hour"));
		_syncPeriodBox->addItem(tr("6 hour"));
		_syncPeriodBox->addItem(tr("12 hour"));
		_syncPeriodBox->addItem(tr("24 hour"));

		_syncPeriodBox->setCurrentIndex(index);

		_authGroup->setTitle(tr("Joomla (Gallery) plugin settings"));
		
		_syncGroup->setTitle(tr("Sync"));
		_progressGroup->setTitle(tr("Status:"));
		_statusLabel->setText(tr("Plugin status:"));
		_statusValue->setText(tr("<font color=\"red\">Not connected</font>"));
		
		_progressBarLabel->setText(tr("Progress:"));
	}

	void JmmGalleryView::changeEvent ( QEvent * event )
	{
		if(event->type() == QEvent::LanguageChange)
		{
			changeLang();
		}
		QWidget::changeEvent(event);
	}

	JmmArticleView::JmmArticleView(const Data::PluginSettings& settings, QWidget *parent)
		: QWidget(parent)
	{
		_nameLabel = new QLabel(tr("Login:"));
		_nameLabel->setTextFormat(Qt::RichText);
		_nameEdit = new QLineEdit;
		_nameEdit->setText(settings.userName);
		
		_passwordLabel = new QLabel(tr("Password:"));
		_passwordEdit = new QLineEdit;
		
		_startPluginButton = new QPushButton(tr("Connect"));
		_stopPluginButton = new QPushButton(tr("Disconnect"));

		_stopPluginButton->setEnabled(false);

		_fullSyncRadioButton = new QRadioButton(tr("Full sync (download all articles)"));
		_partSyncRadioButton = new QRadioButton(tr("Partial sync (sync list of elements only)"));

		_partSyncRadioButton->setChecked(!settings.bFullSync);
		_fullSyncRadioButton->setChecked(settings.bFullSync);

		_autoSyncCheckBox  = new QCheckBox(tr("Auto Sync"));
		_startSyncButton = new QPushButton(tr("Start sync"));
		_stopSyncButton  = new QPushButton(tr("Stop sync"));

		_autoSyncCheckBox->setChecked(settings.bAutoSync);

		_startSyncButton->setEnabled(settings.bAutoSync);
		_stopSyncButton->setEnabled(settings.bAutoSync);

		_syncPeriodLabel = new QLabel(tr("Frequency synchronization:"));
		_syncPeriodBox = new QComboBox();

		_syncPeriodBox->setEditable(false);
		_syncPeriodBox->addItem(tr("30 sec"));
		_syncPeriodBox->addItem(tr("1 min"));
		_syncPeriodBox->addItem(tr("5 min"));
		_syncPeriodBox->addItem(tr("10 min"));
		_syncPeriodBox->addItem(tr("30 min"));
		_syncPeriodBox->addItem(tr("1 hour"));
		_syncPeriodBox->addItem(tr("2 hour"));
		_syncPeriodBox->addItem(tr("6 hour"));
		_syncPeriodBox->addItem(tr("12 hour"));
		_syncPeriodBox->addItem(tr("24 hour"));

		if(settings.syncPeriod == "30")
		{
			_syncPeriodBox->setCurrentIndex(0);
		}
		else if(settings.syncPeriod == "60")
		{
			_syncPeriodBox->setCurrentIndex(1);
		}
		else if(settings.syncPeriod == "300")
		{
			_syncPeriodBox->setCurrentIndex(2);
		}
		else if(settings.syncPeriod == "600")
		{
			_syncPeriodBox->setCurrentIndex(3);
		}
		else if(settings.syncPeriod == "1800")
		{
			_syncPeriodBox->setCurrentIndex(4);
		}
		else if(settings.syncPeriod == "3600")
		{
			_syncPeriodBox->setCurrentIndex(5);
		}
		else if(settings.syncPeriod == "7200")
		{
			_syncPeriodBox->setCurrentIndex(6);
		}
		else if(settings.syncPeriod == "21600")
		{
			_syncPeriodBox->setCurrentIndex(7);
		}
		else if(settings.syncPeriod == "43200")
		{
			_syncPeriodBox->setCurrentIndex(8);
		}
		else if(settings.syncPeriod == "86400")
		{
			_syncPeriodBox->setCurrentIndex(9);
		}

		_syncPeriodLabel->setEnabled(settings.bAutoSync);
		_syncPeriodBox->setEnabled(settings.bAutoSync);

		_syncPeriodBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		_authGroup = new QGroupBox(tr("Joomla (Article) plugin settings"));
		_authLayout = new QGridLayout;
		_authLayout->addWidget(_nameLabel, 0, 0);
		_authLayout->addWidget(_nameEdit, 0, 1);
		_authLayout->addWidget(_passwordLabel, 1, 0);
		_authLayout->addWidget(_passwordEdit, 1, 1);
		_authGroup->setLayout(_authLayout);

		_syncGroup = new QGroupBox(tr("Sync"));
		_syncLayout = new QGridLayout;


		_syncLayout->addWidget(_fullSyncRadioButton, 0, 0);
		_syncLayout->addWidget(_partSyncRadioButton, 0, 1);
		_syncLayout->addWidget(_autoSyncCheckBox, 1, 0);
		_syncLayout->addWidget(_syncPeriodLabel, 2, 0);
		_syncLayout->addWidget(_syncPeriodBox, 2, 1);
		_syncLayout->addWidget(_startSyncButton, 3, 0);
		_syncLayout->addWidget(_stopSyncButton, 3, 1);
		_syncGroup->setLayout(_syncLayout);

		_buttonGroup = new QGroupBox();
		_buttonLayout = new QGridLayout;
		_buttonLayout->addWidget(_startPluginButton, 0, 0);
		_buttonLayout->addWidget(_stopPluginButton, 0, 1);
		_buttonGroup->setLayout(_buttonLayout);

		_progressGroup = new QGroupBox(tr("Status:"));
		_progressLayout = new QGridLayout;
		_statusLabel = new QLabel(tr("Plugin status:"));
		_statusValue = new QLabel(tr("<font color=\"red\">Not connected</font>"));
		_statusValue->setTextFormat(Qt::RichText);

		_progressBarLabel = new QLabel(tr("Progress:"));
		_progressBar = new QProgressBar();
		_progressBar->setValue(0);

		_progressLayout->addWidget(_statusLabel, 0,0);
		_progressLayout->addWidget(_statusValue, 0,1);
		_progressLayout->addWidget(_progressBarLabel, 1,0);
		_progressLayout->addWidget(_progressBar, 2,0);
		_progressGroup->setLayout(_progressLayout);


		_mainLayout = new QVBoxLayout;
		_mainLayout->addWidget(_authGroup);
		_mainLayout->addWidget(_syncGroup);
		_mainLayout->addSpacing(12);
		_mainLayout->addWidget(_progressGroup);
		_mainLayout->addSpacing(12);
		_mainLayout->addWidget(_buttonGroup);
		_mainLayout->addStretch(1);
		setLayout(_mainLayout);

		_driver = Common::WebMounter::getPlugin("jmma");
/*YE tmp
		if(_driver)
		{
			_driver->registerView(this);
		}
*/
	}

	void JmmArticleView::changeLang()
	{
		_nameLabel->setText(tr("Login:"));
		_passwordLabel->setText(tr("Password:"));
		
		_startPluginButton->setText(tr("Connect"));
		_stopPluginButton->setText(tr("Disconnect"));

		_fullSyncRadioButton->setText(tr("Full sync (download all articles)"));
		_partSyncRadioButton->setText(tr("Partial sync (sync list of elements only)"));

		
		_autoSyncCheckBox->setText(tr("Auto Sync"));
		_startSyncButton->setText(tr("Start sync"));
		_stopSyncButton->setText(tr("Stop sync"));

		_syncPeriodLabel->setText(tr("Frequency synchronization:"));
		
		int index = _syncPeriodBox->currentIndex();
		
		_syncPeriodBox->clear();
		_syncPeriodBox->addItem(tr("30 sec"));
		_syncPeriodBox->addItem(tr("1 min"));
		_syncPeriodBox->addItem(tr("5 min"));
		_syncPeriodBox->addItem(tr("10 min"));
		_syncPeriodBox->addItem(tr("30 min"));
		_syncPeriodBox->addItem(tr("1 hour"));
		_syncPeriodBox->addItem(tr("2 hour"));
		_syncPeriodBox->addItem(tr("6 hour"));
		_syncPeriodBox->addItem(tr("12 hour"));
		_syncPeriodBox->addItem(tr("24 hour"));

		_syncPeriodBox->setCurrentIndex(index);

		_authGroup->setTitle(tr("Joomla (Article) plugin settings"));
		
		_syncGroup->setTitle(tr("Sync"));
		_progressGroup->setTitle(tr("Status:"));
		_statusLabel->setText(tr("Plugin status:"));
		_statusValue->setText(tr("<font color=\"red\">Not connected</font>"));
		
		_progressBarLabel->setText(tr("Progress:"));
	}

	void JmmArticleView::changeEvent ( QEvent * event )
	{
		if(event->type() == QEvent::LanguageChange)
		{
			changeLang();
		}
		QWidget::changeEvent(event);
	}
}
