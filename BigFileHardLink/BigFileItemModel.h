#pragma once

#include "FileSizeDefine.h"
#include <QAbstractItemModel>
#include <map>
#include <memory>

struct BigFileDataKey
{
	QString drive;
	QString fileName;
	qint64 fileSize;
	bool operator < (const BigFileDataKey& other) const
	{
#define MyComp(x) if (x != other.x) return x < other.x
#define MyComp2(x) if (x != other.x) return x > other.x
		MyComp(drive);
		MyComp2(fileSize);
		MyComp(fileName);
		return false;
#undef MyComp
	}
};

struct IItem 
{
	int itemType;
	int row;
};

struct BigFileData : public IItem
{
	BigFileDataKey key;
	bool checked = true;
	struct RefFile : public IItem
	{
		BigFileData* parent = nullptr;
		QString filePath;
		bool isHardLink = false;
	};
	std::vector<std::shared_ptr<RefFile>> refFiles;
};

class BigFileItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	BigFileItemModel(QObject *parent = nullptr);
	~BigFileItemModel();

	Q_INVOKABLE virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QModelIndex parent(const QModelIndex &child) const override;
	Q_INVOKABLE virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	Q_INVOKABLE virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	void setData(const std::map<BigFileDataKey, std::shared_ptr<BigFileData>>& datas);


	Q_INVOKABLE virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	enum ItemType
	{
		ItemFileData,
		ItemRefFile,
	};
	enum ColumnFile
	{
		ColumnFileName,
		ColumnDrive,
		ColumnFileSize,
		ColumnRefFileCount,
		ColumnCount
	};
private:
	std::vector<std::shared_ptr<BigFileData>> m_bigFiles;
};
