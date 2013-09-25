#include "fvupdatewindow.h"
#include "ui_fvupdatewindow.h"
#include "fvavailableupdate.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>

using namespace Ui;

FvUpdateWindow::FvUpdateWindow(QWidget *parent) :
QWidget(parent),
	m_ui(new Ui::UpdateWindow)
{
	m_ui->setupUi(this);

	m_appIconScene = 0;

	// Delete on close
	setAttribute(Qt::WA_DeleteOnClose, true);

	// Set the "new version is available" string
	QString newVersString = m_ui->newVersionIsAvailableLabel->text().arg(QApplication::applicationName());
	m_ui->newVersionIsAvailableLabel->setText(newVersString);

	// Connect buttons
	connect(m_ui->installUpdateButton, SIGNAL(clicked()), this, SLOT(installClicked()));
	connect(m_ui->skipThisVersionButton, SIGNAL(clicked()), this, SLOT(skipClicked()));
	connect(m_ui->remindMeLaterButton, SIGNAL(clicked()), this, SLOT(remindClicked()));
}

FvUpdateWindow::~FvUpdateWindow()
{
	m_ui->releaseNotesWebView->stop();
	delete m_ui;
}

void FvUpdateWindow::closeEvent(QCloseEvent* event)
{
	if(!m_ui->progressBar->isEnabled())
		hide();
	event->ignore();
}

void FvUpdateWindow::installClicked()
{
	if(m_ui->progressBar->isEnabled())
	{
		emit cancelRequested();
		setProgressBarState(false);
		m_ui->progressBar->setValue(0);
		m_ui->progressBar->setFormat(tr("Canceled"));
	}
	else
	{
		emit installRequested();
	}
}

void FvUpdateWindow::skipClicked()
{
	emit skipInstallRequested();
	hide();
}

void FvUpdateWindow::remindClicked()
{
	emit remindLaterRequested();
	hide();
}

void FvUpdateWindow::onProgress( uint percents )
{
	setProgressBarState(true);
	m_ui->progressBar->setValue(percents);
}

void FvUpdateWindow::onShowWindow(FvAvailableUpdate* proposedUpdate)
{
	if (! proposedUpdate) {
		return;
	}

	show();

	QString downloadString = m_ui->wouldYouLikeToDownloadLabel->text()
		.arg( proposedUpdate->GetEnclosureVersion(), QApplication::applicationVersion() );
	m_ui->wouldYouLikeToDownloadLabel->setText(downloadString);

	m_ui->releaseNotesWebView->stop();
	m_ui->releaseNotesWebView->load(proposedUpdate->GetReleaseNotesLink());

	return;
}

void FvUpdateWindow::onFinished()
{
	setProgressBarState(false);
}

void FvUpdateWindow::setProgressBarState( bool isActive )
{
	if(!isActive)
	{
		m_ui->installUpdateButton->setText(tr("Install Update"));
	}
	else
	{
		m_ui->installUpdateButton->setText(tr("Cancel"));
	}

	m_ui->skipThisVersionButton->setEnabled(!isActive);
	m_ui->remindMeLaterButton->setEnabled(!isActive);
	m_ui->prgressLabel->setEnabled(isActive);
	m_ui->progressBar->setEnabled(isActive);
	m_ui->progressBar->setFormat("%p%");
}

void FvUpdateWindow::onFailed( QString msg )
{
	setProgressBarState(false);
	m_ui->progressBar->setValue(0);
	m_ui->progressBar->setFormat(tr("Failed"));
}
