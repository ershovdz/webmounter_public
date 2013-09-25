#include "fvignoredversions.h"
#include "fvversioncomparator.h"
#include <QSettings>
#include <string>
#include <QApplication>

// QSettings key for the latest skipped version
#define FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY	"FVLatestSkippedVersion"


FVIgnoredVersions::FVIgnoredVersions(QObject *parent) :
QObject(parent)
{
	// noop
}

bool FVIgnoredVersions::VersionIsIgnored(QString version)
{
	// We assume that variable 'version' contains either:
	//	1) The current version of the application (ignore)
	//	2) The version that was skipped before and thus stored in QSettings (ignore)
	//	3) A newer version (don't ignore)
	// 'version' is not likely to contain an older version in any case.

	QString currentAppVersion = QApplication::applicationVersion();

	if (version == currentAppVersion) {
		return true;
	}

	QSettings settings;

	if (settings.contains(FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY)) {
		QString lastSkippedVersion = settings.value(FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY).toString();
		if (version == lastSkippedVersion) {
			// Implicitly skipped version - skip
			return true;
		}
	}

	if (FvVersionComparator::CompareVersions(currentAppVersion.toStdString(), version.toStdString()) == FvVersionComparator::kAscending) {
		// Newer version - do not skip
		return false;
	}

	// Fallback - skip
	return true;
}

void FVIgnoredVersions::IgnoreVersion(QString version)
{
	if (version == FV_APP_VERSION) {
		// Don't ignore the current version
		return;
	}

	if (version.isEmpty()) {
		return;
	}

	QSettings settings;
	settings.setValue(FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY, version);

	return;
}
