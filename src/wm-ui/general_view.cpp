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

#include "general_view.h"
#include "data.h"
#include "webmounter.h"
#include <QFSFileEngine>
#include "common_stuff.h"

#include LVFS_DRIVER_H

namespace Ui
{
	GeneralView::GeneralView(const Data::GeneralSettings& settings, QWidget *parent)
		: QWidget(parent)
	{
		QString sss =  translationDir();
        bool result = m_translator.load(QString("wmui_" + settings.m_appLang), translationDir());
		if(result)
		{
            QApplication::installTranslator(&m_translator);
		}	

        m_state = 0;
        m_parent = parent;

        m_diskStatusGroup = new QGroupBox(tr("Disk"));
        m_diskStatusLayout = new QGridLayout;
        m_statusLabel = new QLabel(tr("Disk status:"));
        m_statusValue = new QLabel(tr("<font color=\"red\"><b>Unmounted</b></font>"));
        m_statusValue->setTextFormat(Qt::RichText);

        m_diskStatusLayout->addWidget(m_statusLabel, 0,0);
        m_diskStatusLayout->addWidget(m_statusValue, 0,1);
        m_diskStatusGroup->setLayout(m_diskStatusLayout);

        m_mountButton = createButton(tr("Mount Disk"), SLOT(mountClicked()));
        m_unmountButton = createButton(tr("Unmount Disk"), SLOT(unmountClicked()));

        m_diskStatusLayout->addWidget(m_mountButton, 1, 0);
        m_diskStatusLayout->addWidget(m_unmountButton, 1, 1);

        m_unmountButton->setEnabled(false);

        m_mainLayout = new QVBoxLayout;
        m_mainLayout->addWidget(m_diskStatusGroup);
        m_mainLayout->addSpacing(12);
        m_mainLayout->addStretch(1);
        setLayout(m_mainLayout);

	}

	QPushButton *GeneralView::createButton(const QString &text, const char *member)
	{
		QPushButton *button = new QPushButton(text);
		connect(button, SIGNAL(clicked()), this, member);
		return button;
	}

	void GeneralView::mountClicked()
	{
        m_mountButton->setEnabled(false);
        m_unmountButton->setEnabled(true);

		emit mount();
	}

	void GeneralView::unmountClicked()
	{
        m_unmountButton->setEnabled(false);
		emit unmount();
	}

	void GeneralView::mounted()
	{
        m_mountButton->setEnabled(false);
        m_unmountButton->setEnabled(true);

        m_statusValue->setText(tr("<font color=\"green\"><b>Mounted</b></font>"));
        m_state = 1;
	}

	void GeneralView::unmounted()
	{
        m_statusValue->setText(tr("<font color=\"red\"><b>Unmounted</b></font>"));

        m_mountButton->setEnabled(true);
        m_unmountButton->setEnabled(false);

        m_state = 0;
	}

	QString GeneralView::translationDir()
	{
		QDir appDir(qApp->applicationDirPath());
		appDir.cd("..");
		appDir.cd("share/webmounter");
		return appDir.absolutePath();
	}
}
