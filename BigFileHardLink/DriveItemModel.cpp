#include "DriveItemModel.h"

DriveItemModel::DriveItemModel(QObject *parent)
	: QAbstractTableModel(parent)
{
}

DriveItemModel::~DriveItemModel()
{
}

void DriveItemModel::update(std::vector<std::shared_ptr<VolumnData>>* datas)
{
	if (m_volumnData != nullptr)
	{
		emit dataChanged(index(0, ScanRatio), index(m_volumnData->size() - 1, RepeateFileSize), {Qt::DisplayRole});
	}
	m_volumnData = datas;
}

Q_INVOKABLE QVariant DriveItemModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch ((Column)section)
		{
		case Column::Drive:
			return QString::fromLocal8Bit("盘符");
		case Column::VolumnSize:
			return QString::fromLocal8Bit("已用大小");
		case Column::ScanRatio:
			return QString::fromLocal8Bit("已扫描");
		case Column::RepeateFileSize:
			return QString::fromLocal8Bit("重复文件占用");
		default:
			break;
		}
	}
	return QVariant();
}

Q_INVOKABLE int DriveItemModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return m_volumnData->size();
}

Q_INVOKABLE int DriveItemModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return ColumnCount;
}

Q_INVOKABLE QVariant DriveItemModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
	if (index.isValid())
	{
		if (role == Qt::DisplayRole)
		{
			const VolumnData& volumnData = *m_volumnData->at(index.row());
			switch ((Column)index.column())
			{
			case Column::Drive:
				return volumnData.rootPath;
			case Column::VolumnSize:
				return QString("%1GB").arg((double)volumnData.usedSize / 1GB, 0, 'f', 1);
			case Column::ScanRatio:
				return QString("%1%").arg(std::min(1., (double)volumnData.scanedSize / volumnData.usedSize) * 100, 0, 'f', 1);
			case Column::RepeateFileSize:
				if (volumnData.repeateFileUse > 1GB)
				{
					return QString("%1GB").arg((double)volumnData.repeateFileUse / 1GB, 0, 'f', 1);
				}
				else
				{
					return QString("%1MB").arg((double)volumnData.repeateFileUse / 1MB, 0, 'f', 1);
				}
			default:
				break;
			}
		}
		else if (role == Qt::CheckStateRole)
		{
			const VolumnData& volumnData = *m_volumnData->at(index.row());
			if (index.column() == Column::Drive)
			{
				return volumnData.checked ? Qt::Checked : Qt::Unchecked;
			}
		}
	}
	return QVariant();
}

Q_INVOKABLE bool DriveItemModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
	if (index.isValid())
	{
		if (role == Qt::CheckStateRole)
		{
			if (index.column() == Column::Drive)
			{
				VolumnData& volumnData = *m_volumnData->at(index.row());
				volumnData.checked = value.toBool();
				emit dataChanged(index, index);
				return true;
			}
		}
	}
	return false;
}

Qt::ItemFlags DriveItemModel::flags(const QModelIndex &index) const
{
	auto flags = QAbstractTableModel::flags(index);
	flags.setFlag(Qt::ItemIsUserCheckable, true);
	return flags;
}
