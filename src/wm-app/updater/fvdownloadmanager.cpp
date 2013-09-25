#include "fvdownloadmanager.h"

#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QStringList>

FvDownloadManager::FvDownloadManager(QObject* parent) 
	: QObject(parent)
	, m_pCurrentDownload(nullptr)
{
	connect(&m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)));
}

void FvDownloadManager::startDownload(const QUrl &url)
{
	if(m_pCurrentDownload)
	{
		if(url != m_pCurrentDownload->url())
			emit error(m_pCurrentDownload->url(), tr("Another download is in progress"));
		return;
	}

	if("" == url.toString())
	{
		emit error(m_pCurrentDownload->url(), tr("Empty url"));
		return;
	}

	QNetworkRequest request(url);
	m_pCurrentDownload = m_manager.get(request);

	m_downloadAborted = false;
	connect(m_pCurrentDownload, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
}

void FvDownloadManager::downloadFinished(QNetworkReply *reply)
{
	const QUrl& url = reply->url();
	QString filename;

	if(m_pCurrentDownload != reply)
		emit error(url, "WTF !?!");

	if (m_pCurrentDownload->error()) 
	{
		if(m_pCurrentDownload->error() == QNetworkReply::OperationCanceledError)
			return;

		emit error(url, m_pCurrentDownload->errorString());
	} 
	else 
	{
		filename = saveFileName(url);
		if(saveToDisk(filename))
		{
			emit finished(url, filename);
		}
	}

	m_pCurrentDownload->deleteLater();
	m_pCurrentDownload = nullptr;
}

void FvDownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if(m_downloadAborted) 
		return;

	emit progress(m_pCurrentDownload->url(), bytesReceived, bytesTotal);
}

void FvDownloadManager::cancelDownload()
{
	m_downloadAborted = true;
	m_pCurrentDownload->abort();
	m_pCurrentDownload->deleteLater();
	m_pCurrentDownload = nullptr;
}

QString FvDownloadManager::saveFileName(const QUrl &url)
{
	QString path = url.path();

	QString basename = QFileInfo(path).baseName();
	QString fileDir = QDir::tempPath();
	QString fileExtension = QFileInfo(path).completeSuffix();

	if (basename.isEmpty())
		basename = "download";

	if (QFile::exists(basename + '.' + fileExtension)) 
	{
		// already exists, don't overwrite
		int i = 0;
		basename += '.';
		while (QFile::exists(basename + QString::number(i) + '.' + fileExtension))
			++i;
		basename += QString::number(i);
	}

	return fileDir + '/' + basename + '.' + fileExtension;
}

bool FvDownloadManager::saveToDisk(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) 
	{
		emit error(m_pCurrentDownload->url(), tr("Couldn't save file %1").arg(filename));
		return false;
	}

	file.write(m_pCurrentDownload->readAll());
	return true;
}