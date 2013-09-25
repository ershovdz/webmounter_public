#ifndef GOOGLE_VIEW_H
#define GOOGLE_VIEW_H

#include "plugin_view.h"

namespace Ui
{
	class GoogleView : public PluginView
	{
		Q_OBJECT
	public:
		GoogleView(const Data::PluginSettings* settings, const QString& title);
	
	public slots:
		virtual void updateView(int progress, int state);
	private:
		virtual bool isKeyValueValid(const Data::PluginSettings& settings);
	};
}
#endif // GOOGLE_VIEW_H