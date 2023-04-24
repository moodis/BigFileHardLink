#include "BigFileItemModel.h"
#include <QDir>

BigFileItemModel::BigFileItemModel(QObject *parent)
	: QAbstractItemModel(parent)
{
}

BigFileItemModel::~BigFileItemModel()
{
}

Q_INVOKABLE QModelIndex BigFileItemModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
	if (!parent.isValid())
	{
		if (row < m_bigFiles.size())
		{
			return createIndex(row, column, m_bigFiles[row].get());
		}
	}
	else
	{
		IItem* iItem = (IItem*)parent.internalPointer();
		assert(iItem->itemType == ItemFileData);
		if (iItem->itemType == ItemFileData)
		{//BigFileData
			BigFileData* fileData = (BigFileData*)parent.internalPointer();
			return createIndex(row, column, fileData->refFiles[row].get());
		}
	}
	return QModelIndex();
}

Q_INVOKABLE QModelIndex BigFileItemModel::parent(const QModelIndex &child) const
{
	IItem* iItem = (IItem*)child.internalPointer();
	if (iItem->itemType == ItemFileData)
	{//BigFileData
	}
	else if (iItem->itemType == ItemRefFile)
	{//RefFile
		BigFileData::RefFile* refFile = (BigFileData::RefFile*)child.internalPointer();
		return index(refFile->parent->row, 0);
	}
	return QModelIndex();
}

Q_INVOKABLE int BigFileItemModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	if (!parent.isValid())
	{
		return m_bigFiles.size();
	}
	else
	{
		IItem* iItem = (IItem*)parent.internalPointer();
		if (iItem->itemType == ItemFileData)
		{//BigFileData
			BigFileData* fileData = (BigFileData*)parent.internalPointer();
			if (parent.column() == 0)
			{
				return fileData->refFiles.size();
			}
		}
		return 0;
	}
}

Q_INVOKABLE int BigFileItemModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	if (!parent.isValid())
	{
		return ColumnCount;
	}
	else
	{
		return 1;
	}
}

Q_INVOKABLE QVariant BigFileItemModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
	if (index.isValid())
	{
		if (role == Qt::DisplayRole)
		{
			IItem* iItem = (IItem*)index.internalPointer();
			if (iItem->itemType == ItemFileData)
			{//BigFileData
				BigFileData* fileData = (BigFileData*)index.internalPointer();
				switch (index.column())
				{
				case BigFileItemModel::ColumnFileName:
					return fileData->key.fileName;
				case BigFileItemModel::ColumnDrive:
					return fileData->key.drive;
				case BigFileItemModel::ColumnFileSize:
					if (fileData->key.fileSize > 1GB)
					{
						return QString("%1GB").arg((double)fileData->key.fileSize / 1GB, 0, 'f', 1);
					}
					else
					{
						return QString("%1MB").arg((double)fileData->key.fileSize / 1MB, 0, 'f', 1);
					}
				case BigFileItemModel::ColumnRefFileCount:
					return fileData->refFiles.size();
				default:
					break;
				}
			}
			else if (iItem->itemType == ItemRefFile)
			{//RefFile
				if (index.column() == 0)
				{
					BigFileData::RefFile* refFile = (BigFileData::RefFile*)index.internalPointer();
					return (refFile->isHardLink ? "*" : "") + refFile->filePath;
				}
			}
		}
		else if (role == Qt::CheckStateRole)
		{
			IItem* iItem = (IItem*)index.internalPointer();
			if (iItem->itemType == ItemFileData)
			{//BigFileData
				BigFileData* fileData = (BigFileData*)index.internalPointer();
				switch (index.column())
				{
				case BigFileItemModel::ColumnFileName:
					return fileData->checked? Qt::Checked : Qt::Unchecked;
				}
			}
		}
	}
	return QVariant();
}

Q_INVOKABLE bool BigFileItemModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
	if (index.isValid())
	{
		if (role == Qt::CheckStateRole)
		{
			IItem* iItem = (IItem*)index.internalPointer();
			if (iItem->itemType == ItemFileData)
			{//BigFileData
				if (index.column() == ColumnFileName)
				{
					BigFileData* fileData = (BigFileData*)index.internalPointer();
					fileData->checked = value.toBool();
					emit dataChanged(index, index);
					return true;
				}
			}
		}
	}
	return false;
}

void BigFileItemModel::setData(const std::map<BigFileDataKey, std::shared_ptr<BigFileData>>& datas)
{
	beginResetModel();
	m_bigFiles.clear();
	m_bigFiles.reserve(datas.size());
	for (auto& f : datas)
	{
		m_bigFiles.push_back(f.second);
	}
	int row = 0;
	for (auto& f : m_bigFiles)
	{
		f->itemType = ItemFileData;
		f->row = row++;
		int subRow = 0;
		for (auto& ref : f->refFiles)
		{
			ref->itemType = ItemRefFile;
			ref->row = subRow++;
		}
	}
	endResetModel();
}

Q_INVOKABLE QVariant BigFileItemModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case ColumnFile::ColumnFileName:
			return QString::fromLocal8Bit("文件名");
		case ColumnFile::ColumnDrive:
			return QString::fromLocal8Bit("所在盘");
		case ColumnFile::ColumnFileSize:
			return QString::fromLocal8Bit("文件大小");
		case ColumnFile::ColumnRefFileCount:
			return QString::fromLocal8Bit("引用次数");
		default:
			break;
		}
	}
	return QVariant();
}

Q_INVOKABLE Qt::ItemFlags BigFileItemModel::flags(const QModelIndex &index) const
{
	auto flags = QAbstractItemModel::flags(index);
	flags.setFlag(Qt::ItemIsUserCheckable, true);
	return flags;
}
