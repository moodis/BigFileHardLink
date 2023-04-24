#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BigFileHardLink.h"
#include "DriveItemModel.h"
#include "BigFileItemModel.h"
#include <thread>
#include <mutex>

class BigFileHardLink : public QMainWindow
{
    Q_OBJECT

public:
    BigFileHardLink(QWidget *parent = Q_NULLPTR);

	private slots:
	void on_btnScan_clicked();
	void on_btnCreateHardlink_clicked();
signals:
	void scanFinished();
private:
	void finnishScan();
	void taskThreadFunc();
	virtual void timerEvent(QTimerEvent *event) override;
private:
	struct Task 
	{
		QString dirPath;
		VolumnData* volumn;
	};

    Ui::BigFileHardLinkClass ui;
	DriveItemModel m_driveModel;
	BigFileItemModel m_bigFileModel;
	std::vector<std::shared_ptr<VolumnData>> m_volumnData;
	std::map<BigFileDataKey, std::shared_ptr<BigFileData>> m_bigFiles;
	std::map<BigFileDataKey, std::shared_ptr<BigFileData>> m_validBigFiles;

	QString m_curDir;

	std::list<Task> m_tasks;
	std::mutex m_mtx;
	bool m_scaning = false;
	std::vector<std::thread> m_threads;
	std::atomic_int m_busyThreadCount = 0, m_repeatedCount = 0;

	int m_dispRepeatedCount = 0;
};
