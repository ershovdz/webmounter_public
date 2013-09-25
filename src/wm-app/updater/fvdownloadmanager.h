#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

class QNetworkReply;

class FvDownloadManager: public QObject
{
	Q_OBJECT

public:
	FvDownloadManager(QObject* parent);

	void startDownload(const QUrl &url);
	void cancelDownload();

signals:
	void progress(const QUrl&, qint64 bytesReceived, qint64 bytesTotal);
	void finished(const QUrl &url, const QString &filename);
	void error(const QUrl &url, const QString& msg);
	
private slots:
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void downloadFinished(QNetworkReply *reply);

private:
	QString saveFileName(const QUrl &url);
	bool saveToDisk(const QString &filename);

private:
	bool m_downloadAborted;
	QNetworkAccessManager m_manager;
	QNetworkReply* m_pCurrentDownload;
};

#endif // DOWNLOADMANAGER_H
