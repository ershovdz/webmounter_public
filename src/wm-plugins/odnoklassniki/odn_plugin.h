#ifndef ODN_PLUGIN_H
#define ODN_PLUGIN_H

#include <QObject>
#include <QtCore/qplugin.h>
#include "../../wm-base/plugin_interface.h"
#include "../../wm-ui/plugin_view.h"

class OdnPlugin : public QObject, public PluginInterface
{
	Q_OBJECT
	Q_INTERFACES(PluginInterface)

public:
	OdnPlugin();
	~OdnPlugin();
private:
	virtual QString name();

	virtual void* getRVFSDriver();
	virtual void* getView();
	virtual void* getSettings();
	virtual QIcon* getIcon();
	virtual QString getTranslationFile(const QString& locale);
private:
	Ui::PluginView* _view;
};


#endif // ODN_PLUGIN_H
