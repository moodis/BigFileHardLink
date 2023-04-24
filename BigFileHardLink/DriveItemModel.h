#pragma once

#include "FileSizeDefine.h"
#include <QAbstractTableModel>
#include <atomic>
#include <memory>

struct VolumnData
{
	QString rootPath;
	qint64 usedSize = 1;
	std::atomic_int64_t scanedSize = 0, repeateFileUse = 0;
	bool checked = true;
};

class DriveItemModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	DriveItemModel(QObject *parent = nullptr);
	~DriveItemModel();
	void update(std::vector<std::shared_ptr<VolumnData>>* datas);

	Q_INVOKABLE virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
	enum Column
	{
		Drive,
		VolumnSize,
		ScanRatio,
		RepeateFileSize,
		ColumnCount
	};
private:
	std::vector<std::shared_ptr<VolumnData>>* m_volumnData = nullptr;
};
