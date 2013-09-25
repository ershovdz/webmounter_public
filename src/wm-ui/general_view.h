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

#ifndef GENERAL_VIEW_H
#define GENERAL_VIEW_H

#include <QWidget>
#include <QtGui>
#include <QTranslator>
#include "../wm-base/data.h"

//#include "control_panel.h"

#if defined(WEBMOUNTER_UI_LIBRARY)
#  define WEBMOUNTER_UI_EXPORT Q_DECL_EXPORT
#else
#  define WEBMOUNTER_UI_EXPORT Q_DECL_IMPORT
#endif

namespace Ui
{
	using namespace Data;
	class WEBMOUNTER_UI_EXPORT GeneralView : public QWidget
	{
		Q_OBJECT
	public:
		GeneralView(const Data::GeneralSettings& settings, QWidget *parent = 0);
	private:
		QPushButton *createButton(const QString &text, const char *member);
		QComboBox *createComboBox(const QString &text = QString());

		static QString translationDir();

#ifdef Q_OS_WIN
		QComboBox * createDiskComboBox(const QString& driveLetter);
#endif

		private slots:
			void mountClicked();
			void unmountClicked();

			public slots:
				void mounted();
				void unmounted();

signals:
				void mount();
				void unmount();

	private:
		QWidget* m_parent;

		QGroupBox* m_diskStatusGroup;
		QGridLayout* m_diskStatusLayout;

		QLabel* m_statusLabel;
		QLabel* m_statusValue;

		//#ifdef Q_OS_WIN
		//    QLabel *_diskLetterLabel;
		//    QComboBox *_diskLetterComboBox;
		//#endif

		QGroupBox* m_buttonGroup;
		QGridLayout* m_buttonLayout;
		QPushButton* m_mountButton;
		QPushButton* m_unmountButton;

		QVBoxLayout* m_mainLayout;
		unsigned int m_state;
		QTranslator m_translator;
	};
}
#endif
