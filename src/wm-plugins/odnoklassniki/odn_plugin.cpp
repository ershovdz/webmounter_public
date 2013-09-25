
#include "odn_plugin.h"
#include "data.h"
#include "webmounter.h"
 
OdnPlugin::OdnPlugin() : _view(NULL)
{
}

OdnPlugin::~OdnPlugin()
{
}

QString OdnPlugin::name()
{
	QString name("Odnoklassniki");
	return name;
}

void* OdnPlugin::getRVFSDriver()
{
	return (void*)(NULL);
}

void* OdnPlugin::getView()
{
	if(!_view)
	{
		Data::PluginSettings settings;
		settings.pluginName = name();
		Common::WebMounter::getSettingStorage()->getData(settings, name());

		_view = new Ui::PluginView(NULL, name());
	}

	return (void*)(_view);
}

void* OdnPlugin::getSettings()
{
	return NULL;
}

QIcon* OdnPlugin::getIcon()
{
	return NULL;
}

QString OdnPlugin::getTranslationFile(const QString& locale)
{
	return QString("odn_wm_pl_" + locale);
}

Q_EXPORT_PLUGIN2(wm-odn-plugin, OdnPlugin);
