/********************************************************************************
** Form generated from reading UI file 'fvupdatewindow.ui'
**
** Created: Sun 15. Sep 01:13:30 2013
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FVUPDATEWINDOW_H
#define UI_FVUPDATEWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtWebKit/QWebView>

QT_BEGIN_NAMESPACE

class Ui_UpdateWindow
{
public:
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout;
    QLabel *newVersionIsAvailableLabel;
    QSpacerItem *horizontalSpacer;
    QLabel *wouldYouLikeToDownloadLabel;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_7;
    QWebView *releaseNotesWebView;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *skipThisVersionButton;
    QLabel *prgressLabel;
    QProgressBar *progressBar;
    QPushButton *installUpdateButton;
    QPushButton *remindMeLaterButton;

    void setupUi(QWidget *UpdateWindow)
    {
        if (UpdateWindow->objectName().isEmpty())
            UpdateWindow->setObjectName(QString::fromUtf8("UpdateWindow"));
        UpdateWindow->resize(584, 604);
        horizontalLayout_6 = new QHBoxLayout(UpdateWindow);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        newVersionIsAvailableLabel = new QLabel(UpdateWindow);
        newVersionIsAvailableLabel->setObjectName(QString::fromUtf8("newVersionIsAvailableLabel"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        newVersionIsAvailableLabel->setFont(font);

        verticalLayout->addWidget(newVersionIsAvailableLabel);

        horizontalSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout->addItem(horizontalSpacer);

        wouldYouLikeToDownloadLabel = new QLabel(UpdateWindow);
        wouldYouLikeToDownloadLabel->setObjectName(QString::fromUtf8("wouldYouLikeToDownloadLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(wouldYouLikeToDownloadLabel->sizePolicy().hasHeightForWidth());
        wouldYouLikeToDownloadLabel->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(wouldYouLikeToDownloadLabel);

        horizontalSpacer_2 = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout->addItem(horizontalSpacer_2);

        groupBox = new QGroupBox(UpdateWindow);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QFont font1;
        font1.setBold(false);
        font1.setWeight(50);
        groupBox->setFont(font1);
        horizontalLayout_7 = new QHBoxLayout(groupBox);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        releaseNotesWebView = new QWebView(groupBox);
        releaseNotesWebView->setObjectName(QString::fromUtf8("releaseNotesWebView"));
        releaseNotesWebView->setMinimumSize(QSize(160, 80));
        releaseNotesWebView->setUrl(QUrl("about:blank"));

        horizontalLayout_7->addWidget(releaseNotesWebView);


        verticalLayout->addWidget(groupBox);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        skipThisVersionButton = new QPushButton(UpdateWindow);
        skipThisVersionButton->setObjectName(QString::fromUtf8("skipThisVersionButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(skipThisVersionButton->sizePolicy().hasHeightForWidth());
        skipThisVersionButton->setSizePolicy(sizePolicy1);

        horizontalLayout_3->addWidget(skipThisVersionButton);

        prgressLabel = new QLabel(UpdateWindow);
        prgressLabel->setObjectName(QString::fromUtf8("prgressLabel"));
        prgressLabel->setEnabled(false);

        horizontalLayout_3->addWidget(prgressLabel);

        progressBar = new QProgressBar(UpdateWindow);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setEnabled(false);
        sizePolicy1.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy1);
        progressBar->setStyleSheet(QString::fromUtf8("QProgressBar {\n"
"     text-align: center;\n"
" }"));
        progressBar->setValue(0);

        horizontalLayout_3->addWidget(progressBar);

        installUpdateButton = new QPushButton(UpdateWindow);
        installUpdateButton->setObjectName(QString::fromUtf8("installUpdateButton"));
        sizePolicy1.setHeightForWidth(installUpdateButton->sizePolicy().hasHeightForWidth());
        installUpdateButton->setSizePolicy(sizePolicy1);
        installUpdateButton->setAutoDefault(true);
        installUpdateButton->setDefault(true);

        horizontalLayout_3->addWidget(installUpdateButton);

        remindMeLaterButton = new QPushButton(UpdateWindow);
        remindMeLaterButton->setObjectName(QString::fromUtf8("remindMeLaterButton"));
        sizePolicy1.setHeightForWidth(remindMeLaterButton->sizePolicy().hasHeightForWidth());
        remindMeLaterButton->setSizePolicy(sizePolicy1);
        remindMeLaterButton->setFlat(false);

        horizontalLayout_3->addWidget(remindMeLaterButton);


        verticalLayout->addLayout(horizontalLayout_3);


        horizontalLayout_6->addLayout(verticalLayout);


        retranslateUi(UpdateWindow);

        QMetaObject::connectSlotsByName(UpdateWindow);
    } // setupUi

    void retranslateUi(QWidget *UpdateWindow)
    {
        UpdateWindow->setWindowTitle(QApplication::translate("UpdateWindow", "Software Update", 0, QApplication::UnicodeUTF8));
        newVersionIsAvailableLabel->setText(QApplication::translate("UpdateWindow", "A new version of %1 is available!", 0, QApplication::UnicodeUTF8));
        wouldYouLikeToDownloadLabel->setText(QApplication::translate("UpdateWindow", "<b>Version %1 </b> is now available - you have <b>%2</b>. Would you like to download it now?", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("UpdateWindow", "Release Notes:", 0, QApplication::UnicodeUTF8));
        skipThisVersionButton->setText(QApplication::translate("UpdateWindow", "Skip This Version", 0, QApplication::UnicodeUTF8));
        prgressLabel->setText(QApplication::translate("UpdateWindow", "Downloading: ", 0, QApplication::UnicodeUTF8));
        progressBar->setFormat(QApplication::translate("UpdateWindow", "%p%", 0, QApplication::UnicodeUTF8));
        installUpdateButton->setText(QApplication::translate("UpdateWindow", "Install Update", 0, QApplication::UnicodeUTF8));
        remindMeLaterButton->setText(QApplication::translate("UpdateWindow", "Remind Me Later", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class UpdateWindow: public Ui_UpdateWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FVUPDATEWINDOW_H
