#ifndef YANDEX_NAROD_VIEW_H
#define YANDEX_NAROD_VIEW_H

#include "plugin_view.h"

namespace Ui
{
	class YaNarodView : public PluginView
	{
		Q_OBJECT
	public:
		YaNarodView(const Data::PluginSettings* settings, const QString& title);
	
	public slots:
		virtual void updateView(int progress, int state);
	private:
		virtual bool isKeyValueValid(const Data::PluginSettings& settings);
	};
}

#endif // YANDEX_NAROD_VIEW_H
