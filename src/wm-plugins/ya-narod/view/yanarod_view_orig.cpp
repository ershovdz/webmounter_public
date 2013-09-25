#include "yanarod_view.h"
#include "../driver/yanarod_driver.h"
#include "webmounter.h"

namespace Ui
{
	using namespace RemoteDriver;

	YaNarodView::YaNarodView(const Data::PluginSettings* settings, const QString& title)
		: PluginView(settings, title)
	{
		_urlEdit->setText("narod.yandex.ru");
		_urlEdit->setEnabled(false);

#ifdef WM_FULL_VERSION
		_keyLabel->setVisible(false);
		_keyEdit->setVisible(false);
		_keyUrl->setVisible(false);
#endif
		_nameLabel->setVisible(false);
		_nameEdit->setVisible(false);
		_passwordLabel->setVisible(false);
		_passwordEdit->setVisible(false);
	}

	bool YaNarodView::isKeyValueValid(const Data::PluginSettings&)
	{
		return true;
	}

	void YaNarodView::updateView(int progress, int state)
	{
		PluginView::updateView(progress, state);
		_urlEdit->setEnabled(false);

		_nameLabel->setVisible(false);
		_nameEdit->setVisible(false);
		_passwordLabel->setVisible(false);
		_passwordEdit->setVisible(false);
	}
}
