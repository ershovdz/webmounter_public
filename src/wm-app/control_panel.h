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

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "tray_notification_device.h"
#include "plugin_view.h"
#include "IGui.h"
#include <QDialog>

#include <QtGui/QMainWindow>
#include <QSystemTrayIcon>
#include <QTranslator>
#include <QtGui>

class FvAvailableUpdate;

namespace Ui
{
	class GeneralView;
	class FvUpdateWindow;

	class ControlPanel : public QDialog
		, public IGui
	{
		Q_OBJECT

	public:
		ControlPanel();

		void setVisible(bool visible);

		// IGui implementation
	public:  
		virtual NotificationDevice* getNotificationDevice()
		{
            return m_notificationDevice;
		}

		virtual void mounted();
		virtual void unmounted();

	protected:
		void closeEvent(QCloseEvent *event);
		public slots:
			void showMsgBox(const QString&, const QString&);
			void showTrayMsg(int, const QString&, const QString&);
			void changeLanguage(const QString&);
signals:
			void mount();
			void unmount();

			//public slots:
			//virtual void showNotification(const Notification& msg);

			private slots:
				void setIcon(int index);
				void iconActivated(QSystemTrayIcon::ActivationReason reason);
				void messageClicked();
				void onHelpClicked();
				void about();

	private:
		void recreateAllWidgets();

		void createIconGroupBox();
		void createMessageGroupBox();
		void createActions();
		void createTrayIcon();

		static QString translationDir();
		void initializeTranslators(const QString& locale);
		void createIcons();
		void initUpdater();
		void createUpdateWindow();
		void changeEvent(QEvent* e);

	private:
        GeneralView* m_generalView;
        QList<PluginView*> m_pluginViewList;
        QVBoxLayout* m_mainLayout;
        QBoxLayout* m_buttonsLayout;
        QHBoxLayout* m_horizontalLayout;
        QList<QTranslator*> m_transList;
        QGroupBox* m_messageGroupBox;
        QLabel* m_typeLabel;
        QLabel* m_titleLabel;
        QLabel* m_bodyLabel;
        QComboBox* m_typeComboBox;
        QLineEdit* m_titleEdit;
        QTextEdit* m_bodyEdit;
        QPushButton* m_showMessageButton;
        QPushButton* m_showMessageButton2;
        QListWidget* m_contentsWidget;
        QStackedWidget* m_pagesWidget;
        QAction* m_restoreAction;
        QAction* m_aboutAction;
        QAction* m_quitAction;
        QSystemTrayIcon* m_trayIcon;
        QMenu* m_trayIconMenu;
        QPushButton* m_closeButton;

        QList<QListWidgetItem*> m_pluginButtonsList;
        QListWidgetItem* m_configButton;
        bool m_showOnCloseMessage;
        TrayNotificationDevice* m_notificationDevice;
        QLabel* m_version;
        QPushButton* m_helpButton;

		public slots:
			void changePage(QListWidgetItem *current, QListWidgetItem *previous);
	};
}

#endif // CONTROLPANEL_H
