#include "fvupdater.h"
#include "fvplatform.h"
#include "fvignoredversions.h"
#include "fvavailableupdate.h"
#include "fvdownloadmanager.h"
#include <QApplication>
#include <QtNetwork>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDebug>

#ifndef FV_APP_NAME
#	error "FV_APP_NAME is undefined (must have been defined by Fervor.pri)"
#endif
#ifndef FV_APP_VERSION
#	error "FV_APP_VERSION is undefined (must have been defined by Fervor.pri)"
#endif

#ifdef FV_DEBUG
// Unit tests
#	include "fvversioncomparatortest.h"
#endif

FvUpdater* FvUpdater::m_Instance = 0;

FvUpdater* FvUpdater::sharedUpdater()
{
	static QMutex mutex;
	if (! m_Instance) 
	{
		QMutexLocker locker(&mutex);

		if (! m_Instance) 
		{
			m_Instance = new FvUpdater;
		}
	}

	return m_Instance;
}

void FvUpdater::drop()
{
	static QMutex mutex;
	QMutexLocker locker(&mutex);
	delete m_Instance;
	m_Instance = 0;
}

FvUpdater::FvUpdater() 
	: QObject(0)
        , m_reply(NULL)
        , m_proposedUpdate(NULL)
{
	createDownloadManager();

#ifdef FV_DEBUG
	// Unit tests
	FvVersionComparatorTest* test = new FvVersionComparatorTest();
	test->runAll();
	delete test;
#endif
}

FvUpdater::~FvUpdater()
{
	if (m_proposedUpdate) 
	{
		delete m_proposedUpdate;
		m_proposedUpdate = 0;
	}
}

void FvUpdater::showUpdaterWindowUpdatedWithCurrentUpdateProposal()
{
	emit updateAvailable(GetProposedUpdate());
}

void FvUpdater::SetFeedURL(QUrl feedURL)
{
	m_feedURL = feedURL;
}

void FvUpdater::SetFeedURL(QString feedURL)
{
	SetFeedURL(QUrl(feedURL));
}

QString FvUpdater::GetFeedURL()
{
	return m_feedURL.toString();
}

FvAvailableUpdate* FvUpdater::GetProposedUpdate()
{
	return m_proposedUpdate;
}

void FvUpdater::InstallUpdate()
{
	qDebug() << "Install update";
	emit progress(0);
	m_downloadManager->startDownload(GetProposedUpdate()->GetEnclosureUrl());
}

void FvUpdater::SkipUpdate()
{
	qDebug() << "Skip update";

	FvAvailableUpdate* proposedUpdate = GetProposedUpdate();
	if (! proposedUpdate) {
		qWarning() << "Proposed update is NULL (shouldn't be at this point)";
		return;
	}

	// Start ignoring this particular version
	FVIgnoredVersions::IgnoreVersion(proposedUpdate->GetEnclosureVersion());
}

void FvUpdater::RemindMeLater()
{
	qDebug() << "Remind me later";
}

void FvUpdater::CancelUpdate()
{
	m_downloadManager->cancelDownload();
}

bool FvUpdater::CheckForUpdates(bool silentAsMuchAsItCouldGet)
{
	if (m_feedURL.isEmpty()) {
		qCritical() << "Please set feed URL via setFeedURL() before calling CheckForUpdates().";
		return false;
	}

	m_silentAsMuchAsItCouldGet = silentAsMuchAsItCouldGet;

	// Check if application's organization name and domain are set, fail otherwise
	// (nowhere to store QSettings to)
	if (QApplication::organizationName().isEmpty()) {
		qCritical() << "QApplication::organizationName is not set. Please do that.";
		return false;
	}
	if (QApplication::organizationDomain().isEmpty()) {
		qCritical() << "QApplication::organizationDomain is not set. Please do that.";
		return false;
	}

	// Set application name / version is not set yet
	if (QApplication::applicationName().isEmpty()) {
		QString appName = QString::fromUtf8(FV_APP_NAME);
		qWarning() << "QApplication::applicationName is not set, setting it to '" << appName << "'";
		QApplication::setApplicationName(appName);
	}
	if (QApplication::applicationVersion().isEmpty()) {
		QString appVersion = QString::fromUtf8(FV_APP_VERSION);
		qWarning() << "QApplication::applicationVersion is not set, setting it to '" << appVersion << "'";
		QApplication::setApplicationVersion(appVersion);
	}

	cancelDownloadFeed();
	m_httpRequestAborted = false;
	startDownloadFeed(m_feedURL);

	return true;
}

bool FvUpdater::CheckForUpdatesSilent()
{
	return CheckForUpdates(true);
}

bool FvUpdater::CheckForUpdatesNotSilent()
{
	return CheckForUpdates(false);
}

void FvUpdater::startDownloadFeed(QUrl url)
{
	m_xml.clear();

	m_reply = m_qnam.get(QNetworkRequest(url));

	connect(m_reply, SIGNAL(readyRead()), this, SLOT(httpFeedReadyRead()));
	connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(httpFeedUpdateDataReadProgress(qint64, qint64)));
	connect(m_reply, SIGNAL(finished()), this, SLOT(httpFeedDownloadFinished()));
}

void FvUpdater::cancelDownloadFeed()
{
	if (m_reply) {
		m_httpRequestAborted = true;
		m_reply->abort();
	}
}

void FvUpdater::httpFeedReadyRead()
{
	// this slot gets called every time the QNetworkReply has new data.
	// We read all of its new data and write it into the file.
	// That way we use less RAM than when reading it at the finished()
	// signal of the QNetworkReply
	m_xml.addData(m_reply->readAll());
}

void FvUpdater::httpFeedUpdateDataReadProgress(qint64 bytesRead,
	qint64 totalBytes)
{
	Q_UNUSED(bytesRead);
	Q_UNUSED(totalBytes);

	if (m_httpRequestAborted) {
		return;
	}
}

void FvUpdater::httpFeedDownloadFinished()
{
	if (m_httpRequestAborted) {
		m_reply->deleteLater();
		return;
	}

	QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (m_reply->error()) {

		// Error.
		showErrorDialog(tr("Feed download failed: %1.").arg(m_reply->errorString()), false);

	} else if (! redirectionTarget.isNull()) {
		QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());

		m_feedURL = newUrl;
		m_reply->deleteLater();

		startDownloadFeed(m_feedURL);
		return;

	} else {

		// Done.
		xmlParseFeed();

	}

	m_reply->deleteLater();
	m_reply = 0;
}

bool FvUpdater::xmlParseFeed()
{
	QString currentTag, currentQualifiedTag;

	QString xmlTitle, xmlLink, xmlReleaseNotesLink, xmlPubDate, xmlEnclosureUrl,
		xmlEnclosureVersion, xmlEnclosurePlatform, xmlEnclosureType;
	unsigned long xmlEnclosureLength;

	// Parse
	while (! m_xml.atEnd()) {

		m_xml.readNext();

		if (m_xml.isStartElement()) {

			currentTag = m_xml.name().toString();
			currentQualifiedTag = m_xml.qualifiedName().toString();

			if (m_xml.name() == "item") {

				xmlTitle.clear();
				xmlLink.clear();
				xmlReleaseNotesLink.clear();
				xmlPubDate.clear();
				xmlEnclosureUrl.clear();
				xmlEnclosureVersion.clear();
				xmlEnclosurePlatform.clear();
				xmlEnclosureLength = 0;
				xmlEnclosureType.clear();

			} else if (m_xml.name() == "enclosure") {

				QXmlStreamAttributes attribs = m_xml.attributes();

				if (attribs.hasAttribute("fervor:platform")) {
					QString platform = attribs.value("fervor:platform").toString().trimmed();

					if (FvPlatform::CurrentlyRunningOnPlatform(platform)) {
						xmlEnclosurePlatform = platform;
						if (attribs.hasAttribute("url")) {
							xmlEnclosureUrl = attribs.value("url").toString().trimmed();
						} else {
							xmlEnclosureUrl = "";
						}

						if (attribs.hasAttribute("fervor:version")) {
							xmlEnclosureVersion = attribs.value("fervor:version").toString().trimmed();
						} else if (attribs.hasAttribute("sparkle:version")) {
							xmlEnclosureVersion = attribs.value("sparkle:version").toString().trimmed();
						} else {
							xmlEnclosureVersion = "";
						}

						if (attribs.hasAttribute("length")) {
							xmlEnclosureLength = attribs.value("length").toString().toLong();
						} else {
							xmlEnclosureLength = 0;
						}
						if (attribs.hasAttribute("type")) {
							xmlEnclosureType = attribs.value("type").toString().trimmed();
						} else {
							xmlEnclosureType = "";
						}
					}
				}
			}
		} else if (m_xml.isEndElement()) {

			if (m_xml.name() == "item") {

				// That's it - we have analyzed a single <item> and we'll stop
				// here (because the topmost is the most recent one, and thus
				// the newest version.

				return searchDownloadedFeedForUpdates(xmlTitle,
					xmlLink,
					xmlReleaseNotesLink,
					xmlPubDate,
					xmlEnclosureUrl,
					xmlEnclosureVersion,
					xmlEnclosurePlatform,
					xmlEnclosureLength,
					xmlEnclosureType);

			}

		} else if (m_xml.isCharacters() && ! m_xml.isWhitespace()) {

			if (currentTag == "title") {
				xmlTitle += m_xml.text().toString().trimmed();

			} else if (currentTag == "link") {
				xmlLink += m_xml.text().toString().trimmed();

			} else if (currentQualifiedTag == "sparkle:releaseNotesLink") {
				xmlReleaseNotesLink += m_xml.text().toString().trimmed();

			} else if (currentTag == "pubDate") {
				xmlPubDate += m_xml.text().toString().trimmed();

			}
		}

		if (m_xml.error() && m_xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {

			showErrorDialog(tr("Feed parsing failed: %1 %2.").arg(QString::number(m_xml.lineNumber()), m_xml.errorString()), false);
			return false;
		}
	}

	return false;
}


bool FvUpdater::searchDownloadedFeedForUpdates(QString xmlTitle,
	QString xmlLink,
	QString xmlReleaseNotesLink,
	QString xmlPubDate,
	QString xmlEnclosureUrl,
	QString xmlEnclosureVersion,
	QString xmlEnclosurePlatform,
	unsigned long xmlEnclosureLength,
	QString xmlEnclosureType)
{
	Q_UNUSED(xmlTitle);
	Q_UNUSED(xmlPubDate);
	Q_UNUSED(xmlEnclosureLength);
	Q_UNUSED(xmlEnclosureType);

	// Validate
	if (xmlReleaseNotesLink.isEmpty()) {
		if (xmlLink.isEmpty()) {
			showErrorDialog(tr("Feed error: \"release notes\" link is empty"), false);
			return false;
		} else {
			xmlReleaseNotesLink = xmlLink;
		}
	} else {
		xmlLink = xmlReleaseNotesLink;
	}
	if (! (xmlLink.startsWith("http://") || xmlLink.startsWith("https://"))) {
		showErrorDialog(tr("Feed error: invalid \"release notes\" link"), false);
		return false;
	}
	if (xmlEnclosureUrl.isEmpty() || xmlEnclosureVersion.isEmpty() || xmlEnclosurePlatform.isEmpty()) {
		showErrorDialog(tr("Feed error: invalid \"enclosure\" with the download link"), false);
		return false;
	}

	// Relevant version?
	if (FVIgnoredVersions::VersionIsIgnored(xmlEnclosureVersion)) {
		qDebug() << "Version '" << xmlEnclosureVersion << "' is ignored, too old or something like that.";

		showInformationDialog(tr("No updates were found."), false);
		emit noUpdates();
		return true;	// Things have succeeded when you think of it.
	}

	//
	// Success! At this point, we have found an update that can be proposed
	// to the user.
	//

	if (m_proposedUpdate) {
		delete m_proposedUpdate; m_proposedUpdate = 0;
	}
	m_proposedUpdate = new FvAvailableUpdate();
	m_proposedUpdate->SetTitle(xmlTitle);
	m_proposedUpdate->SetReleaseNotesLink(xmlReleaseNotesLink);
	m_proposedUpdate->SetPubDate(xmlPubDate);
	m_proposedUpdate->SetEnclosureUrl(xmlEnclosureUrl);
	m_proposedUpdate->SetEnclosureVersion(xmlEnclosureVersion);
	m_proposedUpdate->SetEnclosurePlatform(xmlEnclosurePlatform);
	m_proposedUpdate->SetEnclosureLength(xmlEnclosureLength);
	m_proposedUpdate->SetEnclosureType(xmlEnclosureType);

	// Show "look, there's an update" window
	showUpdaterWindowUpdatedWithCurrentUpdateProposal();
	return true;
}


void FvUpdater::showErrorDialog(QString message, bool showEvenInSilentMode)
{
	if (m_silentAsMuchAsItCouldGet) {
		if (! showEvenInSilentMode) {
			// Don't show errors in the silent mode
			return;
		}
	}

	QMessageBox dlFailedMsgBox;
	dlFailedMsgBox.setIcon(QMessageBox::Critical);
	dlFailedMsgBox.setText(tr("Error"));
	dlFailedMsgBox.setInformativeText(message);
	dlFailedMsgBox.exec();
}

void FvUpdater::showInformationDialog(QString message, bool showEvenInSilentMode)
{
	if (m_silentAsMuchAsItCouldGet) {
		if (! showEvenInSilentMode) {
			// Don't show information dialogs in the silent mode
			return;
		}
	}

	QMessageBox dlInformationMsgBox;
	dlInformationMsgBox.setIcon(QMessageBox::Information);
	dlInformationMsgBox.setText(tr("Information"));
	dlInformationMsgBox.setInformativeText(message);
	dlInformationMsgBox.exec();
}

void FvUpdater::downloadUpdateFailed( const QUrl&, const QString& msg)
{
	emit failed(msg);
}

void FvUpdater::downloadUpdateFinished( const QUrl&, const QString& fileName)
{
	emit finished();
	emit closeAppToRunInstaller(fileName);
}

void FvUpdater::downloadUpdateProgress( const QUrl&, qint64 receivedBytes, qint64 totalBytes)
{
	uint percents = ((float)receivedBytes/totalBytes) * 100;
	emit progress(percents);
}

void FvUpdater::createDownloadManager()
{
	m_downloadManager = new FvDownloadManager(this);

	connect( m_downloadManager
		, SIGNAL(progress(const QUrl&, qint64, qint64))
		, this
		, SLOT(downloadUpdateProgress(const QUrl&, qint64, qint64)) );

	connect( m_downloadManager
		, SIGNAL(finished(const QUrl&, const QString&))
		, this
		, SLOT(downloadUpdateFinished(const QUrl&, const QString&)) );

	connect( m_downloadManager
		, SIGNAL(error(const QUrl&, const QString&))
		, this
		, SLOT(downloadUpdateFailed(const QUrl&, const QString&)) );
}
