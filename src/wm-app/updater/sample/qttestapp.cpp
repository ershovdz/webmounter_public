#include "qttestapp.h"
#include <QMessageBox>
#include "fvupdater.h"
#include <QProcess>
#include <QTranslator>
#include <QTextCodec>
#include "fvupdatewindow.h"
#include "fvavailableupdate.h"

QtTestApp::QtTestApp(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	initApp();
	initUpdater();

	connect(ui.pushButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
}

void QtTestApp::initApp()
{
	ui.setupUi(this);

	QApplication::setApplicationName("WebMounter");
	QApplication::setApplicationVersion("0.9");
	QApplication::setOrganizationName("SuperSoft");
	QApplication::setOrganizationDomain("SuperSoft.com");

	QTranslator translator;
	QString locale = QLocale::system().name();
	translator.load(QString("fervor_") + locale);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	qApp->installTranslator(&translator);
	m_movie = new QMovie(":/QtTestApp/progressbar.gif");
	ui.progressLabel->clear();
	ui.checkingLabel->hide();
}

void QtTestApp::initUpdater()
{
	createUpdaterWindow();
	
	FvUpdater::sharedUpdater()->SetFeedURL("https://raw.github.com/ershovdz/WebMounter_Builds/master/Appcast.xml");

	// signals from WINDOW
	connect(m_updaterWindow, SIGNAL(installRequested()), FvUpdater::sharedUpdater(), SLOT(InstallUpdate()));
	connect(m_updaterWindow, SIGNAL(skipInstallRequested()), FvUpdater::sharedUpdater(), SLOT(SkipUpdate()));
	connect(m_updaterWindow, SIGNAL(remindLaterRequested()), FvUpdater::sharedUpdater(), SLOT(RemindMeLater()));
	connect(m_updaterWindow, SIGNAL(cancelRequested()), FvUpdater::sharedUpdater(), SLOT(CancelUpdate()));

	// signals from UPDATER
	connect(FvUpdater::sharedUpdater(), SIGNAL(finished()), m_updaterWindow, SLOT(onFinished()));
	connect(FvUpdater::sharedUpdater(), SIGNAL(failed(QString)), m_updaterWindow, SLOT(onFailed(QString)));
	connect(FvUpdater::sharedUpdater(), SIGNAL(progress(uint)), m_updaterWindow, SLOT(onProgress(uint)));
	connect(FvUpdater::sharedUpdater(), SIGNAL(updateAvailable(FvAvailableUpdate*)), this, SLOT(onUpdates(FvAvailableUpdate*)));
	connect(FvUpdater::sharedUpdater(), SIGNAL(noUpdates()), this, SLOT(onNoUpdates()));
	connect(FvUpdater::sharedUpdater(), SIGNAL(closeAppToRunInstaller()), this, SLOT(onCloseApp()));
}

void QtTestApp::createUpdaterWindow()
{
	m_updaterWindow = new FvUpdateWindow();
}

QtTestApp::~QtTestApp()
{
}

void QtTestApp::onUpdates(FvAvailableUpdate* update)
{
	hideProgress();
	m_updaterWindow->onShowWindow(update);
}

void QtTestApp::onClicked()
{
	ui.progressLabel->setMovie(m_movie);
	m_movie->start();
	ui.checkingLabel->show();
	FvUpdater::sharedUpdater()->CheckForUpdatesSilent();
}

void QtTestApp::onNoUpdates()
{
	hideProgress();
}

void QtTestApp::hideProgress()
{
	ui.progressLabel->clear();
	ui.checkingLabel->hide();
	m_movie->stop();
}

void QtTestApp::onCloseApp()
{
	QApplication::instance()->quit();
}
