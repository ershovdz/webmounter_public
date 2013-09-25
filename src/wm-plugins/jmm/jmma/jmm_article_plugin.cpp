
#include "jmm_article_plugin.h"
#include "./driver/jmm_driver.h"
#include "plugin_view.h"
#include "data.h"
#include "webmounter.h"
 
JmmArticlePlugin::JmmArticlePlugin() : _driver(NULL), _view(NULL), _icon(NULL)
{
}

JmmArticlePlugin::~JmmArticlePlugin()
{
}

QString JmmArticlePlugin::name()
{
	QString name("Joomla.Article");
	return name;
}

void* JmmArticlePlugin::getRVFSDriver()
{
	if(!_driver)
		_driver = new RemoteDriver::JmmRVFSDriver(name());
	return (void*)(_driver);
}

void* JmmArticlePlugin::getView()
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

void* JmmArticlePlugin::getSettings()
{
	return NULL;
}

QIcon* JmmArticlePlugin::getIcon()
{
	if(!_icon)
	{
		_icon = new QIcon(":/icons/joomla.png");
	}

	return _icon;
}

QString JmmArticlePlugin::getTranslationFile(const QString& locale)
{
	return QString("jmm_" + locale);
}

Q_EXPORT_PLUGIN2(wm-jmm-article-plugin, JmmArticlePlugin);
