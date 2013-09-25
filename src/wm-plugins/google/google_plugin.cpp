
#include "google_plugin.h"
#include "./driver/google_driver.h"
#include "./view/google_view.h"
#include "data.h"
#include "webmounter.h"
 
GooglePlugin::GooglePlugin() : _driver(NULL), _view(NULL), _icon(NULL)
{
}

GooglePlugin::~GooglePlugin()
{
	delete _driver;
}

QString GooglePlugin::name()
{
	QString name("Google.Docs");
	return name;
}

void* GooglePlugin::getRVFSDriver()
{
	if(!_driver)
		_driver = new RemoteDriver::GoogleRVFSDriver(name());
	return (void*)(_driver);
}

void* GooglePlugin::getView()
{
	if(!_view)
	{
		Data::PluginSettings settings;
		settings.pluginName = name();
		Common::WebMounter::getSettingStorage()->getData(settings, name());

		_view = new Ui::GoogleView(&settings, name());
	}

	return (void*)(_view);
}

void* GooglePlugin::getSettings()
{
	return NULL;
}

QIcon* GooglePlugin::getIcon()
{
	if(!_icon)
	{
		_icon = new QIcon(":/icons/docs_logo_ru.png");
	}
	return _icon;
}

QString GooglePlugin::getTranslationFile(const QString& locale)
{
	return QString("google_wm_pl_" + locale);
}

Q_EXPORT_PLUGIN2(wm-google-plugin, GooglePlugin);
