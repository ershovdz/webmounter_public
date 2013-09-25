#ifndef FVUPDATER_H
#define FVUPDATER_H

#include <QObject>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QXmlStreamReader>

class FvAvailableUpdate;
class FvDownloadManager;

class FvUpdater : public QObject
{
	Q_OBJECT

public:
	// Singleton
	static FvUpdater* sharedUpdater();
	static void drop();

	// Set / get feed URL
	void SetFeedURL(QUrl feedURL);
	void SetFeedURL(QString feedURL);
	QString GetFeedURL();

signals:
	void updateAvailable(FvAvailableUpdate* update);
	void noUpdates();
	void progress(uint percents);
	void finished();
	void failed(QString msg);
	void closeAppToRunInstaller(QString pathToInstaller);
	
public slots:
	bool CheckForUpdates(bool silentAsMuchAsItCouldGet = true);
	void InstallUpdate();
	void SkipUpdate();
	void RemindMeLater();
	void CancelUpdate();

	// Aliases
	bool CheckForUpdatesSilent();
	bool CheckForUpdatesNotSilent();

protected:
	friend class FvUpdateConfirmDialog;	// Uses GetProposedUpdate() and others
	FvAvailableUpdate* GetProposedUpdate();
	
protected slots:
	// Download updates
	void downloadUpdateFailed(const QUrl&, const QString&);
	void downloadUpdateFinished(const QUrl&, const QString&);
	void downloadUpdateProgress(const QUrl&, qint64, qint64);

private:
	//
	// Singleton business
	//
	// (we leave just the declarations, so the compiler will warn us if we try
	//  to use those two functions by accident)
	FvUpdater();							// Hide main constructor
	~FvUpdater();							// Hide main destructor
	FvUpdater(const FvUpdater&);			// Hide copy constructor
	FvUpdater& operator=(const FvUpdater&);	// Hide assign op

	void createDownloadManager();
	void showUpdaterWindowUpdatedWithCurrentUpdateProposal();		// Show updater window
	void hideUpdaterWindow();										// Hide + destroy m_updaterWindow

	// Dialogs (notifications)
	void showErrorDialog(QString message, bool showEvenInSilentMode = false);			// Show an error message
	void showInformationDialog(QString message, bool showEvenInSilentMode = false);		// Show an informational message

	void startDownloadFeed(QUrl url);	// Start downloading feed
	void cancelDownloadFeed();			// Stop downloading the current feed

private:
	static FvUpdater* m_Instance;			// Singleton instance


	// Available update (NULL if not fetched)
	FvAvailableUpdate* m_proposedUpdate;

	// If true, don't show the error dialogs and the "no updates." dialog
	// (silentAsMuchAsItCouldGet from CheckForUpdates() goes here)
	// Useful for automatic update checking upon application startup.
	bool m_silentAsMuchAsItCouldGet;

	//
	// HTTP feed fetcher infrastructure
	//
	QUrl m_feedURL;					// Feed URL that will be fetched
	QNetworkAccessManager m_qnam;
	QNetworkReply* m_reply;
	int m_httpGetId;
	bool m_httpRequestAborted;
	FvDownloadManager* m_downloadManager;

private slots:
	void httpFeedReadyRead();
	void httpFeedUpdateDataReadProgress(qint64 bytesRead,
										qint64 totalBytes);
	void httpFeedDownloadFinished();

private:

	//
	// XML parser
	//
	QXmlStreamReader m_xml;				// XML data collector and parser
	bool xmlParseFeed();				// Parse feed in m_xml
	bool searchDownloadedFeedForUpdates(QString xmlTitle,
										QString xmlLink,
										QString xmlReleaseNotesLink,
										QString xmlPubDate,
										QString xmlEnclosureUrl,
										QString xmlEnclosureVersion,
										QString xmlEnclosurePlatform,
										unsigned long xmlEnclosureLength,
										QString xmlEnclosureType);

};

#endif // FVUPDATER_H
