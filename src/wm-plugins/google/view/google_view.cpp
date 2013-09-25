#include "google_view.h"
#include "../driver/google_driver.h"
#include "webmounter.h"

namespace Ui
{
	using namespace RemoteDriver;

	GoogleView::GoogleView(const Data::PluginSettings* settings, const QString& title)
		: PluginView(settings, title)
	{
		_urlEdit->setText("docs.google.com");
		_urlEdit->setEnabled(false);
#ifdef WM_FULL_VERSION
		_keyLabel->setVisible(false);
		_keyEdit->setVisible(false);
		_keyUrl->setVisible(false);
#endif
	}

	bool GoogleView::isKeyValueValid(const Data::PluginSettings&)
	{
		return true;
	}

	void GoogleView::updateView(int progress, int state)
	{
		PluginView::updateView(progress, state);
		_urlEdit->setEnabled(false);
	}
}
