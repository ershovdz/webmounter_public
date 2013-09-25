#ifndef GOOGLE_PLUGIN_H
#define GOOGLE_PLUGIN_H

#include <QObject>
#include <QtCore/qplugin.h>
#include "../../wm-base/plugin_interface.h"
#include "./driver/google_driver.h"
//#include "./view/google_view.h"
#include "plugin_view.h"

class GooglePlugin : public QObject, public PluginInterface
{
	Q_OBJECT
	Q_INTERFACES(PluginInterface)

public:
	GooglePlugin();
	~GooglePlugin();
private:
	virtual QString name();

	virtual void* getRVFSDriver();
	virtual void* getView();
	virtual void* getSettings();
	virtual QIcon* getIcon();
	virtual QString getTranslationFile(const QString& locale);
private:
	RemoteDriver::GoogleRVFSDriver* _driver;
	Ui::PluginView* _view;
	QIcon* _icon;
};


#endif // GOOGLE_PLUGIN_H
