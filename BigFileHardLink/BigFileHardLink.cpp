#include "BigFileHardLink.h"

#include "Windows.h"

#include <QStorageInfo>
#include <QDir>
#include <filesystem>

BigFileHardLink::BigFileHardLink(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	for (auto volume : QStorageInfo::mountedVolumes())
	{
		std::shared_ptr<VolumnData> data = std::make_shared<VolumnData>();
		data->rootPath = volume.rootPath();
		data->usedSize = volume.bytesTotal() - volume.bytesFree();
		m_volumnData.push_back(data);
	}
	m_driveModel.update(&m_volumnData);
	ui.tableDrive->setModel(&m_driveModel);
	ui.treeFile->setModel(&m_bigFileModel);

	connect(this, &BigFileHardLink::scanFinished, this, &BigFileHardLink::finnishScan);
	startTimer(100);
}

void BigFileHardLink::on_btnScan_clicked()
{
	if (m_scaning == false)
	{
		m_scaning = true;
		ui.btnScan->setText(QString::fromLocal8Bit("停止扫描"));
		ui.btnCreateHardlink->setEnabled(false);
		m_tasks.clear();
		m_bigFiles.clear();
		m_validBigFiles.clear();
		m_bigFileModel.setData(m_validBigFiles);
		m_busyThreadCount = 0;
		m_dispRepeatedCount = -1;
		m_repeatedCount = 0;
		for (auto& d : m_volumnData)
		{
			d->scanedSize = 0;
			d->repeateFileUse = 0;
			if (d->checked)
			{
				Task task;
				task.dirPath = d->rootPath;
				task.volumn = d.get();
				m_tasks.push_back(task);
			}
		}
		for (int i = 0; i < ui.spinThreadCount->value(); ++i)
		{
			m_threads.push_back(std::thread(&BigFileHardLink::taskThreadFunc, this));
		}
	}
	else
	{
		finnishScan();
	}
}

void BigFileHardLink::on_btnCreateHardlink_clicked()
{
	for (auto& f : m_validBigFiles)
	{
		if (f.second->checked)
		{
			auto baseFile = f.second->refFiles[0];
			for (int i = 1; i < f.second->refFiles.size(); ++i)
			{
				auto refFile = f.second->refFiles[i];
				QString bakupFilePath = refFile->filePath + ".bakup";
				if (QFile::rename(refFile->filePath, bakupFilePath))
				{
					try
					{
						std::tr2::sys::create_hard_link(baseFile->filePath.toLocal8Bit().toStdString(), refFile->filePath.toLocal8Bit().toStdString());
						if (QFile::exists(refFile->filePath))
						{
							QFile::remove(bakupFilePath);
						}
					}
					catch (std::exception& e)
					{
						QFile::rename(bakupFilePath, refFile->filePath);
					}
				}
			}
		}
	}
}

void BigFileHardLink::finnishScan()
{
	if (m_scaning)
	{
		m_scaning = false;
		ui.btnScan->setText(QString::fromLocal8Bit("开始扫描"));
		ui.btnCreateHardlink->setEnabled(!m_validBigFiles.empty());
		for (auto& t : m_threads)
		{
			if (t.joinable())
			{
				t.join();
			}
		}
		m_busyThreadCount = 0;
		m_threads.clear();
	}
}

void BigFileHardLink::taskThreadFunc()
{
	while (m_scaning)
	{
		if (!m_tasks.empty())
		{
			Task task;
			{
				std::unique_lock<std::mutex> lck(m_mtx);
				if (!m_tasks.empty())
				{
					++m_busyThreadCount;
					task = m_tasks.front();
					m_tasks.pop_front();
					m_curDir = task.dirPath;
				}
			}
			if (!task.dirPath.isEmpty())
			{
				QDir dir(task.dirPath);
				{
					std::list<Task> taskList;
					for (auto& subDir : dir.entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot))
					{
						Task newTask = task;
						newTask.dirPath = subDir.filePath();
						taskList.push_back(newTask);
					}
					if (!taskList.empty())
					{
						std::unique_lock<std::mutex> lck(m_mtx);
						m_tasks.insert(m_tasks.end(), taskList.begin(), taskList.end());
					}
				}
				for (auto& file : dir.entryInfoList(ui.editFileNames->text().split(';'), QDir::Filter::Files | QDir::Filter::NoSymLinks))
				{
					auto fileSize = file.size();
					task.volumn->scanedSize += fileSize;
					if (fileSize > (ui.spinFileSize->value() * 1MB))
					{
						BigFileDataKey key;
						key.drive = task.volumn->rootPath;
						key.fileName = file.fileName();
						key.fileSize = fileSize;

						std::shared_ptr<BigFileData> bigFileData;
						std::shared_ptr<BigFileData::RefFile> refFile = std::make_shared<BigFileData::RefFile>();
						refFile->filePath = file.filePath();
						auto p = refFile->filePath.toStdWString();
						HANDLE hFile = CreateFileW(refFile->filePath.toStdWString().c_str(),
							0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (hFile != INVALID_HANDLE_VALUE)
						{
							BY_HANDLE_FILE_INFORMATION fileInfo;
							if (GetFileInformationByHandle(hFile, &fileInfo))
							{
								refFile->isHardLink = fileInfo.nNumberOfLinks > 1;
							}
							CloseHandle(hFile);
						}
						{
							std::unique_lock<std::mutex> lck(m_mtx);
							if (m_bigFiles.count(key) == 0)
							{
								m_bigFiles[key] = bigFileData = std::make_shared<BigFileData>();
								bigFileData->key = key;
							}
							else
							{
								bigFileData = m_bigFiles.at(key);
							}
							refFile->parent = bigFileData.get();
							bigFileData->refFiles.push_back(refFile);
						}
						if (bigFileData->refFiles.size() > 1)
						{
							if (bigFileData->refFiles.size() == 2 && m_validBigFiles.count(key) == 0)
							{
								std::unique_lock<std::mutex> lck(m_mtx);
								m_validBigFiles[key] = bigFileData;
							}
							if (refFile->isHardLink)
							{
								if (bigFileData->refFiles[0]->isHardLink)
								{
									task.volumn->scanedSize -= fileSize;
								}
								else
								{
									std::unique_lock<std::mutex> lck(m_mtx);
									if (bigFileData->refFiles[0]->isHardLink == false)
									{
										std::swap(*bigFileData->refFiles[0], *refFile);
									}
								}
							}
							else
							{
								task.volumn->repeateFileUse += fileSize;
							}
							++m_repeatedCount;
						}
					}
				}
				--m_busyThreadCount;
			}
		} else if (m_busyThreadCount == 0)
		{
			emit scanFinished();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void BigFileHardLink::timerEvent(QTimerEvent *event)
{
	m_driveModel.update(&m_volumnData);
	std::unique_lock<std::mutex> lck(m_mtx);
	ui.statusBar->showMessage(QString::fromLocal8Bit("工作线程:%1, 当前目录:%2").arg(m_busyThreadCount).arg(m_curDir), 1000);
	if (m_dispRepeatedCount != m_repeatedCount)
	{
		m_bigFileModel.setData(m_validBigFiles);
		m_dispRepeatedCount = m_repeatedCount;
	}
}
