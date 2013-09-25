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

#ifndef PLUGIN_VIEW_H
#define PLUGIN_VIEW_H

#include <QWidget>
#include <QtGui>

#include "data.h"

#include "rvfs_driver.h"

#include "vfs_cache.h"
#include "view.h"

#if defined(WEBMOUNTER_UI_LIBRARY)
#  define WEBMOUNTER_UI_EXPORT Q_DECL_EXPORT
#else
#  define WEBMOUNTER_UI_EXPORT Q_DECL_IMPORT
#endif

namespace Ui
{
	using namespace Common;

	class WEBMOUNTER_UI_EXPORT PluginView : public View
	{
		Q_OBJECT

	public:
		PluginView(const Data::PluginSettings* settings, const QString& title);
		virtual void changeLang();

	protected:
		void changeEvent ( QEvent * event );
		//int getSyncPeriod();

		private slots:
			//void autoSyncClicked(int state);
			void oauthClicked(int state);
			//void startSyncClicked(bool);
			//void stopSyncClicked(bool);
			void startPluginClicked(bool);
			void stopPluginClicked(bool);

			public slots:
				virtual void updateView(int progress, int state);

	protected:
		QCheckBox* m_oauthCheckBox;
		QLabel* m_dummyLabel;
		/*QLabel *_urlLabel;*/
		QLineEdit* m_urlEdit;
		QLabel* m_nameLabel;
		QLineEdit* m_nameEdit;
		QLabel* m_passwordLabel;
		QLineEdit* m_passwordEdit;

		QPushButton* m_startPluginButton;
		QPushButton* m_stopPluginButton;
		//QPushButton *_startSyncButton;
		//QPushButton *_stopSyncButton;

		QRadioButton*  m_fullSyncRadioButton;
		QRadioButton* m_partSyncRadioButton;
		//QCheckBox* _autoSyncCheckBox; 
		//QLabel *_syncPeriodLabel; 
		//QComboBox *_syncPeriodBox;
		QGroupBox* m_authGroup;
		QGridLayout* m_authLayout;
		QGroupBox* m_syncGroup;
		QGridLayout* m_syncLayout;
		QGroupBox* m_buttonGroup; 
		QGridLayout* m_buttonLayout;
		QGroupBox* m_progressGroup;
		QGridLayout* m_progressLayout;
		QLabel* m_statusLabel;
		QLabel* m_statusValue;
		QLabel* m_progressBarLabel; 
		QProgressBar* m_progressBar;
		QVBoxLayout* m_mainLayout;

		RemoteDriver::RVFSDriver* m_driver;
		QString m_pluginName;
		RemoteDriver::DriverState m_driverState;

	};
}
#endif