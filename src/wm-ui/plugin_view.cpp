/* Copyright (c) 2013, Alexander Ershov
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 * Contact e-mail: Alexander Ershov <ershav@yandex.ru>
 */

#include "plugin_view.h"

#include "webmounter.h"

namespace Ui
{
	PluginView::PluginView(const Data::PluginSettings* settings, const QString& /*title*/)
	{
        m_driver = 0;

		if(!settings)
		{
            m_dummyLabel = new QLabel(tr("<font size=\"5\" color=\"red\" align=\"right\">Not implemented yet</font>"));

            m_mainLayout = new QVBoxLayout;
            m_mainLayout->setAlignment(Qt::AlignHCenter);
            m_mainLayout->addWidget(m_dummyLabel);
            setLayout(m_mainLayout);
			return;
		}

        m_oauthCheckBox = new QCheckBox(tr("Use OAuth"));
        m_oauthCheckBox->setChecked(settings->m_isOAuthUsing);
        m_oauthCheckBox->setVisible(false);

		/*_urlLabel = new QLabel(tr("Server address:"));*/
        m_urlEdit = new QLineEdit;
        m_urlEdit->setText(settings->m_serverUrl);

        m_nameLabel = new QLabel(tr("Login:"));
        m_nameLabel->setTextFormat(Qt::RichText);
        m_nameEdit = new QLineEdit;
        m_nameEdit->setText(!settings->m_isOAuthUsing ? settings->m_userName : "");
        m_nameEdit->setEnabled(!settings->m_isOAuthUsing);

        m_passwordLabel = new QLabel(tr("Password:"));
        m_passwordEdit = new QLineEdit;

        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_passwordEdit->setEnabled(!settings->m_isOAuthUsing);

        m_startPluginButton = new QPushButton(tr("Connect"));
        m_stopPluginButton = new QPushButton(tr("Disconnect"));

        m_stopPluginButton->setEnabled(false);
        m_fullSyncRadioButton = new QRadioButton(tr("Full sync (download all files)"));
        m_partSyncRadioButton = new QRadioButton(tr("Partial sync (sync list of elements only)"));

        m_partSyncRadioButton->setChecked(!settings->m_fullSync);
        m_fullSyncRadioButton->setChecked(settings->m_fullSync);

		/*_autoSyncCheckBox  = new QCheckBox(tr("Auto Sync"));*/
		//_startSyncButton = new QPushButton(tr("Start sync"));
		//_stopSyncButton  = new QPushButton(tr("Stop sync"));

		//_autoSyncCheckBox->setChecked(settings->bAutoSync);

		//_startSyncButton->setEnabled(false);
		//_stopSyncButton->setEnabled(false);

		//_syncPeriodLabel = new QLabel(tr("Frequency synchronization:"));
		//_syncPeriodBox = new QComboBox();

		/*_syncPeriodBox->setEditable(false);
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

		if(settings->syncPeriod == "30")
		{
		_syncPeriodBox->setCurrentIndex(0);
		}
		else if(settings->syncPeriod == "60")
		{
		_syncPeriodBox->setCurrentIndex(1);
		}
		else if(settings->syncPeriod == "300")
		{
		_syncPeriodBox->setCurrentIndex(2);
		}
		else if(settings->syncPeriod == "600")
		{
		_syncPeriodBox->setCurrentIndex(3);
		}
		else if(settings->syncPeriod == "1800")
		{
		_syncPeriodBox->setCurrentIndex(4);
		}
		else if(settings->syncPeriod == "3600")
		{
		_syncPeriodBox->setCurrentIndex(5);
		}
		else if(settings->syncPeriod == "7200")
		{
		_syncPeriodBox->setCurrentIndex(6);
		}
		else if(settings->syncPeriod == "21600")
		{
		_syncPeriodBox->setCurrentIndex(7);
		}
		else if(settings->syncPeriod == "43200")
		{
		_syncPeriodBox->setCurrentIndex(8);
		}
		else if(settings->syncPeriod == "86400")
		{
		_syncPeriodBox->setCurrentIndex(9);
		}*/

		//_syncPeriodLabel->setEnabled(settings->bAutoSync);
		//_syncPeriodBox->setEnabled(settings->bAutoSync);

		//_syncPeriodBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		//_authGroup = new QGroupBox(tr("Authorization"));
		//_authLayout = new QGridLayout;
		///*_authLayout->addWidget(_urlLabel, 0, 0);
		//_authLayout->addWidget(_urlEdit, 0, 1);
		//_authLayout->addWidget(_nameLabel, 0, 0);
		//_authLayout->addWidget(_nameEdit, 0, 1);
		//_authLayout->addWidget(_passwordLabel, 1, 0);
		//_authLayout->addWidget(_passwordEdit, 1, 1);
		//_authLayout->addWidget(_oauthCheckBox, 2, 0);

		//_authGroup->setLayout(_authLayout);

        m_syncGroup = new QGroupBox(tr("Sync"));
        m_syncLayout = new QGridLayout;

        m_syncLayout->addWidget(m_fullSyncRadioButton, 0, 0, 1, 2);
        m_syncLayout->addWidget(m_partSyncRadioButton, 1, 0, 2, 2);
		//		_syncLayout->addWidget(_startSyncButton, 3, 0);
		//		_syncLayout->addWidget(_stopSyncButton, 3, 1);
        m_syncGroup->setLayout(m_syncLayout);

        m_buttonGroup = new QGroupBox();
        m_buttonLayout = new QGridLayout;
        m_buttonLayout->addWidget(m_startPluginButton, 0, 0);
        m_buttonLayout->addWidget(m_stopPluginButton, 0, 1);
        m_buttonGroup->setLayout(m_buttonLayout);

        m_progressGroup = new QGroupBox(tr("Status"));
        m_progressLayout = new QGridLayout;
        m_progressLayout->setAlignment(Qt::AlignRight);
        m_statusLabel = new QLabel(tr("Plugin status:"));
        m_statusValue = new QLabel(tr("<font color=\"red\">Not connected</font>"));
        m_statusValue->setTextFormat(Qt::RichText);

        m_progressBarLabel = new QLabel(tr("Progress:"));
        m_progressBar = new QProgressBar();
        m_progressBar->setValue(0);

        m_progressLayout->addWidget(m_statusLabel, 0,0);
        m_progressLayout->addWidget(m_statusValue, 0,1);
        m_progressLayout->addWidget(m_progressBarLabel, 1,0);
        m_progressLayout->addWidget(m_progressBar, 1,1);
        m_progressGroup->setLayout(m_progressLayout);

        m_mainLayout = new QVBoxLayout;
		//_mainLayout->addWidget(_authGroup);
        m_mainLayout->addWidget(m_syncGroup);
        m_mainLayout->addSpacing(12);
        m_mainLayout->addWidget(m_progressGroup);
        m_mainLayout->addSpacing(12);
        m_mainLayout->addWidget(m_buttonGroup);
        m_mainLayout->addStretch(1);
        setLayout(m_mainLayout);

        m_pluginName = settings->m_pluginName;
        m_driver = Common::WebMounter::getPlugin(m_pluginName);

        connect(m_oauthCheckBox, SIGNAL(stateChanged(int)), this, SLOT(oauthClicked(int)));

        if(m_driver)
		{
            m_driverState = m_driver->getState();

			//connect(_startSyncButton, SIGNAL(clicked (bool)), this, SLOT(startSyncClicked(bool)));
			//connect(_stopSyncButton, SIGNAL(clicked (bool)), this, SLOT(stopSyncClicked(bool)));
            connect(m_startPluginButton, SIGNAL(clicked (bool)), this, SLOT(startPluginClicked(bool)));
            connect(m_stopPluginButton, SIGNAL(clicked (bool)), this, SLOT(stopPluginClicked(bool)));

            connect(this, SIGNAL(connectPlugin(Data::PluginSettings&)), m_driver, SLOT(startPlugin(Data::PluginSettings&)));
            connect(this, SIGNAL(disconnectPlugin()), m_driver, SLOT(stopPlugin()));
            connect(this, SIGNAL(startSync()), m_driver, SLOT(startSync()));
            connect(this, SIGNAL(stopSync()), m_driver, SLOT(stopSync()));

            connect(m_driver, SIGNAL(updateView(int, int)), this, SLOT(updateView(int, int)));

		}
		else
		{
            m_driverState = RemoteDriver::eNotConnected;

			//_urlEdit->setEnabled(false);
            m_nameEdit->setEnabled(false);
            m_passwordEdit->setEnabled(false);

            m_startPluginButton->setEnabled(false);
            m_stopPluginButton->setEnabled(false);

			//_startSyncButton->setEnabled(false);
			//_stopSyncButton->setEnabled(false);

            m_partSyncRadioButton->setEnabled(false);
            m_fullSyncRadioButton->setEnabled(false);

            m_statusValue->setText(tr("<font color=\"red\">Plugin not available...</font>"));
		}
	}

	void PluginView::changeLang()
	{
        if(!m_driver)
		{
            m_dummyLabel->setText(tr("<font size=\"5\" color=\"red\" align=\"right\">Not implemented yet</font>"));
			return;
		}
        m_oauthCheckBox->setText(tr("Use OAuth"));
		//_urlLabel->setText(tr("Server address:"));
        m_nameLabel->setText(tr("Login:"));
        m_passwordLabel->setText(tr("Password:"));

        m_startPluginButton->setText(tr("Connect"));
        m_stopPluginButton->setText(tr("Disconnect"));

        m_fullSyncRadioButton->setText(tr("Full sync (download all files)"));
        m_partSyncRadioButton->setText(tr("Partial sync (sync list of elements only)"));


		//_startSyncButton->setText(tr("Start sync"));
		//_stopSyncButton->setText(tr("Stop sync"));

		//_authGroup->setTitle(tr("Authorization"));

        m_syncGroup->setTitle(tr("Sync"));
        m_progressGroup->setTitle(tr("Status:"));
        m_statusLabel->setText(tr("Plugin status:"));

        if(!m_driver)
		{
            m_statusValue->setText(tr("<font color=\"red\">Not available</font>"));
		}

        switch(m_driver->getState())
		{
		case RemoteDriver::eAuthInProgress:
			{
                m_statusValue->setText(tr("<font color=\"green\">Authorization...</font>"));
				break;
			}
		case RemoteDriver::eAuthorized:
			{
                m_statusValue->setText(tr("<font color=\"green\">Authorized</font>"));
				break;
			}
		case RemoteDriver::eConnected:
			{
                m_statusValue->setText(tr("<font color=\"green\">Connected</font>"));
				break;
			}
		case RemoteDriver::eNotConnected:
			{
                m_statusValue->setText(tr("<font color=\"red\">Not Connected</font>"));
				break;
			}
		case RemoteDriver::eSyncStopping:
			{
                m_statusValue->setText(tr("<font color=\"green\">Sync stopping...</font>"));
				break;
			}
		case RemoteDriver::eSync:
			{
                m_statusValue->setText(tr("<font color=\"green\">Synchronization...</font>"));
				break;
			}
		};

        m_progressBarLabel->setText(tr("Progress:"));
	}

	void PluginView::changeEvent ( QEvent * event )
	{
		if(event->type() == QEvent::LanguageChange)
		{
			changeLang();
		}
		QWidget::changeEvent(event);
	}

	void PluginView::oauthClicked(int state)
	{
		if(state)
		{
            m_nameEdit->setText("");
            m_passwordEdit->setText("");
		}

        m_nameEdit->setEnabled(!state);
        m_passwordEdit->setEnabled(!state);
	}

	/*void PluginView::startSyncClicked(bool)
	{
	_startSyncButton->setEnabled(false);
	_stopSyncButton->setEnabled(true);

	_statusValue->setText(tr("<font color=\"green\">Synchronization...</font>"));

	emit startSync();
	}*/

	//void PluginView::stopSyncClicked(bool)
	//{
	//	_startSyncButton->setEnabled(true);
	//	_stopSyncButton->setEnabled(false);

	//	//_statusValue->setText(tr("<font color=\"green\">Connected</font>"));

	//	emit stopSync();
	//}

	void PluginView::startPluginClicked(bool)
	{
		Data::PluginSettings pluginSettings;
        WebMounter::getSettingStorage()->getData(pluginSettings, m_pluginName);

        pluginSettings.m_autoSync = true;
        pluginSettings.m_fullSync = m_fullSyncRadioButton->isChecked();
        pluginSettings.m_pluginName = m_pluginName;
        pluginSettings.m_serverUrl = m_urlEdit->text();
        pluginSettings.m_syncPeriod.setNum(300);

        if(m_nameEdit->isEnabled())
		{
            pluginSettings.m_userName = m_nameEdit->text();
            pluginSettings.m_userPassword = m_passwordEdit->text();
		}

        pluginSettings.m_isOAuthUsing = m_oauthCheckBox->isChecked();

        if(pluginSettings.m_serverUrl == "")
		{
			QMessageBox::critical(0, tr("Error"),
				tr("Enter server address"), QMessageBox::Ok);
			return;
		}
        else if(pluginSettings.m_userName == "" && !pluginSettings.m_isOAuthUsing)
		{
			QMessageBox::critical(0, tr("Error"),
				tr("Enter login"), QMessageBox::Ok);
			return;
		}
        else if(m_passwordEdit->text() == "" && !pluginSettings.m_isOAuthUsing)
		{
			QMessageBox::critical(0, tr("Error"),
				tr("Enter password"), QMessageBox::Ok);
			return;
		}

		//_urlEdit->setEnabled(false);
        m_nameEdit->setEnabled(false);
        m_passwordEdit->setEnabled(false);

        m_startPluginButton->setEnabled(false);
        m_stopPluginButton->setEnabled(true);

		//		_startSyncButton->setEnabled(false);
		//		_stopSyncButton->setEnabled(false);

        m_partSyncRadioButton->setEnabled(false);
        m_fullSyncRadioButton->setEnabled(false);


        m_statusValue->setText(tr("<font color=\"green\">Connecting...</font>"));

		emit connectPlugin(pluginSettings);
	}

	void PluginView::stopPluginClicked(bool)
	{
        m_stopPluginButton->setEnabled(false);
		//_statusValue->setText(tr("<font color=\"red\">Not Connected</font>"));
		emit disconnectPlugin();
	}

	void PluginView::updateView(int progress, int state)
	{
        m_progressBar->setValue(progress);

		switch(state)
		{
		case RemoteDriver::eAuthInProgress:
			{
                if(m_driverState == RemoteDriver::eNotConnected)
				{
                    m_driverState = RemoteDriver::eAuthInProgress;
                    m_statusValue->setText(tr("<font color=\"green\">Authorization...</font>"));
				}
				break;
			}
		case RemoteDriver::eAuthorized:
			{
                if(m_driverState == RemoteDriver::eAuthInProgress)
				{
                    m_driverState = RemoteDriver::eAuthorized;
                    m_statusValue->setText(tr("<font color=\"green\">Authorized</font>"));
				}
				break;
			}
		case RemoteDriver::eConnected:
			{
                if(m_driverState == RemoteDriver::eSync
                    || m_driverState == RemoteDriver::eSyncStopping)
				{
                    m_driverState = RemoteDriver::eConnected;

					//					_startSyncButton->setEnabled(true);
					//					_stopSyncButton->setEnabled(false);
                    m_statusValue->setText(tr("<font color=\"green\">Connected</font>"));

					//_progressBar->setValue(0);
				}
				break;
			}
		case RemoteDriver::eNotConnected:
			{
                m_driverState = RemoteDriver::eNotConnected;

				//_urlEdit->setEnabled(true);
                m_nameEdit->setEnabled(!m_oauthCheckBox->isChecked());
                m_passwordEdit->setEnabled(!m_oauthCheckBox->isChecked());

                m_startPluginButton->setEnabled(true);
                m_stopPluginButton->setEnabled(false);

				//				_startSyncButton->setEnabled(false);
				//				_stopSyncButton->setEnabled(false);

                m_partSyncRadioButton->setEnabled(true);
                m_fullSyncRadioButton->setEnabled(true);

                m_statusValue->setText(tr("<font color=\"red\">Not Connected</font>"));

                m_progressBar->setValue(0);

				break;
			}
		case RemoteDriver::eSyncStopping:
			{
                if(m_driverState == RemoteDriver::eConnected
                    || m_driverState == RemoteDriver::eSync
                    || m_driverState == RemoteDriver::eSyncStopping)
				{
                    m_driverState = RemoteDriver::eSyncStopping;

                    m_statusValue->setText(tr("<font color=\"green\">Sync stopping...</font>"));
				}

				break;
			}
		case RemoteDriver::eSync:
			{
                if(m_driverState == RemoteDriver::eAuthorized
                    || m_driverState == RemoteDriver::eConnected)
				{
                    m_driverState = RemoteDriver::eSync;
					//					_startSyncButton->setEnabled(false);
					//					_stopSyncButton->setEnabled(true);

                    m_statusValue->setText(tr("<font color=\"green\">Synchronization...</font>"));
				}
				break;
			}
		}
	}
};

