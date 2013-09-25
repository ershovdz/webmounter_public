#ifndef JMM_GALLERY_PLUGIN_H
#define JMM_GALLERY_PLUGIN_H

#include <QObject>
#include <QtCore/qplugin.h>
#include "../../../wm-base/plugin_interface.h"
#include "./driver/jmm_driver.h"
#include "plugin_view.h"

class JmmGalleryPlugin : public QObject, public PluginInterface
{
	Q_OBJECT
	Q_INTERFACES(PluginInterface)

public:
	JmmGalleryPlugin();
	~JmmGalleryPlugin();
private:
	virtual QString name();

	virtual void* getRVFSDriver();
	virtual void* getView();
	virtual void* getSettings();
	virtual QIcon* getIcon();
	virtual QString getTranslationFile(const QString& locale);
private:
	RemoteDriver::JmmRVFSDriver* _driver;
	Ui::PluginView* _view;
	QIcon* _icon;
};

#endif // JMM_GALLERY_PLUGIN_H
