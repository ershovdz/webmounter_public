#ifndef QTTESTAPP_H
#define QTTESTAPP_H

#include <QtGui/QMainWindow>
#include <QMovie>
#include <QLabel>
#include "ui_qttestapp.h"
#include "fvdownloadmanager.h"

class FvUpdateWindow;
class FvAvailableUpdate;

class QtTestApp : public QMainWindow
{
	Q_OBJECT

public:
	QtTestApp(QWidget *parent = 0, Qt::WFlags flags = 0);
	~QtTestApp();

private:
	void initApp();
	void createUpdaterWindow();
	void initUpdater();
	void hideProgress();

private slots:
	void onClicked();
	void onCloseApp();
	void onNoUpdates();
	void onUpdates(FvAvailableUpdate*);

private:
	Ui::QtTestAppClass ui;
	QMovie *m_movie;
	FvUpdateWindow* m_updaterWindow;								// Updater window (NULL if not shown)
};

#endif // QTTESTAPP_H
