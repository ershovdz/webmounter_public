#ifndef FVUPDATEWINDOW_H
#define FVUPDATEWINDOW_H

#include <QWidget>
class QGraphicsScene;

class FvAvailableUpdate;

namespace Ui 
{
	class UpdateWindow;

	class FvUpdateWindow : public QWidget
	{
		Q_OBJECT

	public:
		explicit FvUpdateWindow(QWidget *parent = 0);
		~FvUpdateWindow();

		// Update the current update proposal from FvUpdater
		void closeEvent(QCloseEvent* event);

	private:
		void setProgressBarState( bool isActive );

signals:
		void installRequested();
		void skipInstallRequested();
		void remindLaterRequested();
		void cancelRequested();
		void closed();

		public slots:
			void onProgress(uint percents);
			void onFinished();
			void onFailed(QString msg);
			void onShowWindow(FvAvailableUpdate* proposedUpdate);

			private slots:
				void installClicked();
				void skipClicked();
				void remindClicked();


	private:
		Ui::UpdateWindow*	m_ui;
		QGraphicsScene* m_appIconScene;
	};
};

#endif // FVUPDATEWINDOW_H
