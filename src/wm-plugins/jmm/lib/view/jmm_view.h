#ifndef JMM_VIEW_H
#define JMM_VIEW_H

#include <QWidget>
#include <QtGui>

#include "rvfs_driver.h"

#if defined(WEBMOUNTER_JMM_LIBRARY)
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_EXPORT
#else
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_IMPORT
#endif

namespace Ui
{
	class WEBMOUNTER_JMM_EXPORT JmmArticleView : public QWidget
	{
		Q_OBJECT
	public:
		JmmArticleView(const Data::PluginSettings& settings, QWidget *parent = 0);
		void changeLang();

	protected:
		void changeEvent ( QEvent * event );

	private:
		QLabel *_nameLabel;
		QLineEdit *_nameEdit;
		QLabel *_passwordLabel;
		QLineEdit *_passwordEdit;
		QPushButton *_startPluginButton;
		QPushButton *_stopPluginButton;
		QRadioButton *_fullSyncRadioButton;
		QRadioButton *_partSyncRadioButton;
		QCheckBox* _autoSyncCheckBox; 
		QPushButton *_startSyncButton;
		QPushButton *_stopSyncButton;
		QLabel *_syncPeriodLabel; 
		QComboBox *_syncPeriodBox;
		QGroupBox *_authGroup;
		QGridLayout *_authLayout;
		QGroupBox *_syncGroup;
		QGridLayout *_syncLayout;
		QGroupBox *_buttonGroup; 
		QGridLayout *_buttonLayout;
		QGroupBox *_progressGroup;
		QGridLayout *_progressLayout;
		QLabel *_statusLabel;
		QLabel *_statusValue;
		QLabel *_progressBarLabel; 
		QProgressBar *_progressBar;
		QVBoxLayout *_mainLayout;

		RemoteDriver::RVFSDriver* _driver;
	};

	class WEBMOUNTER_JMM_EXPORT JmmGalleryView : public QWidget
	{
		Q_OBJECT
	public:
		JmmGalleryView(const Data::PluginSettings& settings, QWidget *parent = 0);
		void changeLang();

	protected:
		void changeEvent ( QEvent * event );

	private:
		QLabel *_nameLabel;
		QLineEdit *_nameEdit;
		QLabel *_passwordLabel;
		QLineEdit *_passwordEdit;
		QPushButton *_startPluginButton;
		QPushButton *_stopPluginButton;
		QRadioButton *_fullSyncRadioButton;
		QRadioButton *_partSyncRadioButton;
		QCheckBox* _autoSyncCheckBox; 
		QPushButton *_startSyncButton;
		QPushButton *_stopSyncButton;
		QLabel *_syncPeriodLabel; 
		QComboBox *_syncPeriodBox;
		QGroupBox *_authGroup;
		QGridLayout *_authLayout;
		QGroupBox *_syncGroup;
		QGridLayout *_syncLayout;
		QGroupBox *_buttonGroup; 
		QGridLayout *_buttonLayout;
		QGroupBox *_progressGroup;
		QGridLayout *_progressLayout;
		QLabel *_statusLabel;
		QLabel *_statusValue;
		QLabel *_progressBarLabel; 
		QProgressBar *_progressBar;
		QVBoxLayout *_mainLayout;

		RemoteDriver::RVFSDriver* _driver;
	};
}
#endif