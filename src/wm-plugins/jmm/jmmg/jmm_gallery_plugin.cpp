
#include "jmm_gallery_plugin.h"
#include "./driver/jmm_driver.h"
#include "plugin_view.h"
#include "data.h"
#include "webmounter.h"
 
JmmGalleryPlugin::JmmGalleryPlugin() : _driver(NULL), _view(NULL), _icon(NULL)
{
}

JmmGalleryPlugin::~JmmGalleryPlugin()
{
}

QString JmmGalleryPlugin::name()
{
	QString name("Joomla.Gallery");
	return name;
}

void* JmmGalleryPlugin::getRVFSDriver()
{
	if(!_driver)
		_driver = new RemoteDriver::JmmRVFSDriver(name());
	return (void*)(_driver);
}

void* JmmGalleryPlugin::getView()
{
	if(!_view)
	{
		Data::PluginSettings settings;
		settings.pluginName = name();
		Common::WebMounter::getSettingStorage()->getData(settings, name());

		_view = new Ui::PluginView(&settings, name());
	}

	return (void*)(_view);
}

void* JmmGalleryPlugin::getSettings()
{
	return NULL;
}

QIcon* JmmGalleryPlugin::getIcon()
{
	if(!_icon)
	{
		_icon = new QIcon(":/icons/joomla.png");
	}

	return _icon;
}

QString JmmGalleryPlugin::getTranslationFile(const QString& locale)
{
	return QString("jmm_" + locale);
}

Q_EXPORT_PLUGIN2(wm-jmm-gallery-plugin, JmmGalleryPlugin);
