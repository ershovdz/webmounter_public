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

#include <QtGui>
#include <QTranslator>

#include "control_panel.h"

#include "webmounter.h"

#include "general_view.h"

#include "plugin_interface.h"
//#include "fvupdater.h"
//#include "fvupdatewindow.h"

namespace Ui
{
	ControlPanel::ControlPanel()
	{
		setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint );

        m_contentsWidget = 0;
        m_pagesWidget = 0;
        m_horizontalLayout = 0;
        m_buttonsLayout = 0;
        m_mainLayout = 0;
        m_helpButton = 0;

        m_showOnCloseMessage = true;

		recreateAllWidgets();

		//QObject::connect(_generalView, SIGNAL(changeLanguage(const QString&)), this, SLOT(changeLanguage(const QString&)));
        QObject::connect(m_notificationDevice, SIGNAL(showMsgBox(const QString&, const QString&)), this, SLOT(showMsgBox(const QString&, const QString&)));
        QObject::connect(m_notificationDevice, SIGNAL(showTrayMsg(int, const QString&, const QString&)), this, SLOT(showTrayMsg(int, const QString&, const QString&)));
	}

	void ControlPanel::showMsgBox(const QString& title, const QString& description)
	{
		QMessageBox::critical(0, title, description, QMessageBox::Cancel);
	}

	void ControlPanel::showTrayMsg(int type, const QString& title, const QString& description)
	{
		QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(type);
        if(m_trayIcon)
		{
            m_trayIcon->showMessage(title, description, icon, 3 * 1000);
		}
	}

	void ControlPanel::recreateAllWidgets()
	{
		GeneralSettings generalSettings;

		Common::WebMounter::getSettingStorage()->getData(generalSettings);

        if(generalSettings.m_appLang == "")
		{
            generalSettings.m_appLang = QLocale::system().name();
		}

        initializeTranslators(generalSettings.m_appLang);

		createActions();
		createTrayIcon();

        m_notificationDevice = new TrayNotificationDevice(m_trayIcon);

        connect(m_trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
        connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

		QIcon icon = QIcon(":resources/drive.png");
        m_trayIcon->setIcon(icon);
		setWindowIcon(icon);

        m_trayIcon->show();

        m_contentsWidget = new QListWidget;
        m_contentsWidget->setViewMode(QListView::IconMode);
        m_contentsWidget->setIconSize(QSize(75, 55));
        m_contentsWidget->setMovement(QListView::Static);
        m_contentsWidget->setMaximumWidth(110);
        m_contentsWidget->setMinimumWidth(110);
        m_contentsWidget->setMinimumHeight(390);
        m_contentsWidget->setMaximumHeight(390);
        m_contentsWidget->setSpacing(10);

        m_pagesWidget = new QStackedWidget;

		Common::PluginList& plugins = Common::WebMounter::plugins();
		Common::PluginList::iterator iter;
		for(iter=plugins.begin(); iter!=plugins.end(); iter++)
		{
			PluginSettings settings;
			Common::WebMounter::getSettingStorage()->getData(settings, iter->first);

			PluginView* view;
			view = (PluginView*)iter->second->getView();
            m_pagesWidget->addWidget(view);
		}

        m_generalView = new GeneralView(generalSettings, this);
        connect(m_generalView, SIGNAL(mount()), this, SIGNAL(mount()));
        connect(m_generalView, SIGNAL(unmount()), this, SIGNAL(unmount()));

        m_pagesWidget->addWidget(m_generalView);

		createIcons();

        m_contentsWidget->setCurrentRow(0);

        m_horizontalLayout = new QHBoxLayout;
        m_horizontalLayout->addWidget(m_contentsWidget);
        m_horizontalLayout->addWidget(m_pagesWidget);

        m_version = new QLabel(tr("version ") + VERSION);
        m_version->setDisabled(true);

        m_helpButton = new QPushButton;
        m_helpButton->setText(tr("Help..."));
        connect(m_helpButton, SIGNAL(clicked()), this, SLOT(onHelpClicked()));

        m_buttonsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
        m_buttonsLayout->addSpacing(135);
        m_buttonsLayout->addWidget(m_version, 1, Qt::AlignLeft);
        m_buttonsLayout->addWidget(m_helpButton, 1, Qt::AlignLeft);
        m_buttonsLayout->addStretch(5);

        m_mainLayout = new QVBoxLayout;
        m_mainLayout->addLayout(m_horizontalLayout);
        m_mainLayout->addStretch(1);
        m_mainLayout->addSpacing(12);
        m_mainLayout->addLayout(m_buttonsLayout);
        setLayout(m_mainLayout);

		setWindowTitle(tr("WebMounter"));

		this->setMinimumWidth(620);
		this->setMinimumHeight(420);

		this->setMaximumWidth(620);
		this->setMaximumHeight(420);

		resize(620, 420);
	}

	void ControlPanel::onHelpClicked()
	{
		QString path = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/Help.chm");
		QDesktopServices::openUrl(QUrl("file:///" + path));
	}

	void ControlPanel::mounted()
	{
        m_generalView->mounted();
	}

	void ControlPanel::unmounted()
	{
        m_generalView->unmounted();
	}

	void ControlPanel::changeLanguage(const QString& locale)
	{
		initializeTranslators(locale);

        m_version->setText(tr("version ") + VERSION);
        m_helpButton->setText(tr("Help..."));
        m_restoreAction->setText(tr("&Restore"));
        m_aboutAction->setText(tr("&About"));
        m_quitAction->setText(tr("&Quit"));

        m_configButton->setText(tr("Configuration"));
	}

	//void ControlPanel::showNotification(const Notification& msg)
	//{
	//  _pNotificationDevice->showNotification(msg);
	//}

	void ControlPanel::createIcons()
	{
		Common::PluginList& plugins = Common::WebMounter::plugins();
		Common::PluginList::iterator iter;
		for(iter=plugins.begin(); iter!=plugins.end(); iter++)
		{
            QListWidgetItem* button = new QListWidgetItem(m_contentsWidget);
			button->setIcon(*(iter->second->getIcon()));
			button->setText(iter->first);
			button->setTextAlignment(Qt::AlignHCenter);
			button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}

		/*_configButton = new QListWidgetItem(_contentsWidget);
		_configButton->setIcon(QIcon(":/resources/config.png"));
		_configButton->setText(tr("Configuration"));
		_configButton->setTextAlignment(Qt::AlignHCenter);
		_configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);*/

        connect(m_contentsWidget,
			SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
			this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
	}

	void ControlPanel::changePage(QListWidgetItem *current, QListWidgetItem *previous)
	{
		if (!current)
			current = previous;

        m_pagesWidget->setCurrentIndex(m_contentsWidget->row(current));
	}

	void ControlPanel::setVisible(bool visible)
	{
        m_restoreAction->setEnabled(isMaximized() || !visible);
		QDialog::setVisible(visible);
	}

	void ControlPanel::closeEvent(QCloseEvent * /*event*/)
	{
        delete m_notificationDevice;

        qDeleteAll(m_transList.begin(), m_transList.end());
        m_transList.clear();

		QApplication::instance()->quit();
	}

	void ControlPanel::setIcon(int /*index*/)
	{
	}

	void ControlPanel::iconActivated(QSystemTrayIcon::ActivationReason reason)
	{
		switch (reason) 
		{
		case QSystemTrayIcon::MiddleClick:
			//showMessage();
			break;
		case QSystemTrayIcon::Trigger:
		case QSystemTrayIcon::DoubleClick:
			showNormal();
			break;
		default:
			;
		}
	}

	void ControlPanel::messageClicked()
	{
	}

	void ControlPanel::createActions()
	{
        m_restoreAction = new QAction(tr("&Restore"), this);
        connect(m_restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

        m_aboutAction = new QAction(tr("&About"), this);
        connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

        m_quitAction = new QAction(tr("&Quit"), this);
        connect(m_quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
	}

	void ControlPanel::createTrayIcon()
	{
        m_trayIconMenu = new QMenu(this);
        m_trayIconMenu->addAction(m_restoreAction);
        m_trayIconMenu->addSeparator();
        m_trayIconMenu->addAction(m_aboutAction);
        m_trayIconMenu->addSeparator();
        m_trayIconMenu->addAction(m_quitAction);

        m_trayIcon = new QSystemTrayIcon(this);
        m_trayIcon->setContextMenu(m_trayIconMenu);
	}

	QString ControlPanel::translationDir()
	{
		QDir appDir(qApp->applicationDirPath());
		appDir.cd("..");
		appDir.cd("share/webmounter");
		return appDir.absolutePath();
	}

	void ControlPanel::initializeTranslators(const QString& locale)
	{
        for(int i=m_transList.count()-1; i>=0; i--)
		{
            QTranslator* tr = m_transList[i];
			QApplication::removeTranslator(tr);
            m_transList.removeLast();
			delete tr;
		}

		printf("ControlPanel::initializeTranslators, after removing\n");
		QTranslator* tr = new QTranslator;
		bool result = tr->load(QString("wmbase_" + locale), translationDir());
		QApplication::installTranslator(tr);
        m_transList.append(tr);

		tr = new QTranslator;
		result = tr->load(QString("webmounter_" + locale), translationDir());
		QApplication::installTranslator(tr);
        m_transList.append(tr);

		Common::PluginList& plugins = Common::WebMounter::plugins();
		Common::PluginList::iterator iter;
		for(iter=plugins.begin(); iter!=plugins.end(); iter++)
		{
			//printf("ControlPanel::initializeTranslators, plugin = %p\n", iter->second);
			QString filename(iter->second->getTranslationFile(locale));
			if(filename.length())
			{
				QTranslator* tr1 = new QTranslator;
				printf("ControlPanel::initializeTranslators, tr = %p\n", tr1);
				result = tr1->load(filename, translationDir());
				if(result)
				{
					QApplication::installTranslator(tr1);
                    m_transList.append(tr1);
				}
				else
				{
					delete tr1;
				}
			}
		}
		printf("ControlPanel::initializeTranslators, after addition\n");
	}

	void ControlPanel::changeEvent(QEvent* e)
	{
		switch (e->type())
		{
		case QEvent::WindowStateChange:
			{
				if (this->windowState() & Qt::WindowMinimized)
				{
					QTimer::singleShot(250, this, SLOT(hide()));
				}
				break;
			}
		default:
			break;
		}

		QDialog::changeEvent(e);
	}

	void ControlPanel::about()
	{
		QString text1 = tr("<h4>Application allows to map http storages as local</h4>\n\n");
		QString author = tr("<h5>Author: Alex Ershov</h5>\n");
		QString text2 = tr("<h5>For technical support</h5>");
		QString text3 = "<a href=\"ershav@yandex.ru\">mailto:ershav@yandex.ru</a>";
		QString text4 = tr("<h5>or see</h5>");
		QMessageBox::about(this, tr("About"), text1+author+text2+text3+text4);
	}
};
