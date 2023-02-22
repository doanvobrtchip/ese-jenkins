/*!
 * @file RamDL.cpp
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "RamDL.h"

#include <QDockWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QtEndian>
// STL includes
#include <stdio.h>

#include <QTreeWidgetItem>
#include <iomanip>
#include <sstream>
// Emulator includes
#include <bt8xxemu_diag.h>

#include "inspector.h"
#include "main_window.h"
#include "utils/CommonUtil.h"
#include "utils/ConvertUtil.h"
#include "utils/LoggerUtil.h"

namespace FTEDITOR {
extern BT8XXEMU_Emulator *g_Emulator;

RamDL::RamDL(Inspector *parent)
    : QDockWidget(parent->mainWindow())
    , m_DLCMDCount(0)
    , m_FirstDisplayCMD(-1)
    , m_Inspector(parent)
{
	setFocusPolicy(Qt::StrongFocus);
	m_HandleUsage = new bool[FTED_NUM_HANDLES];
	m_DisplayList = new QTreeWidget(parent);
	m_DisplayList->setColumnCount(3);
	m_DisplayList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_DisplayList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_DisplayList->setContextMenuPolicy(Qt::CustomContextMenu);
	m_DisplayList->installEventFilter(this);
	m_DisplayList->viewport()->installEventFilter(this);
	
	m_CopyAct = new QAction(parent);
	m_CopyAct->setText(tr("Copy"));
	m_ContextMenu = new QMenu(parent);
	m_ContextMenu->addAction(m_CopyAct);

	QStringList dlHeaders;
	dlHeaders.push_back(tr(""));
	dlHeaders.push_back(tr("Raw"));
	dlHeaders.push_back(tr("Text"));
	m_DisplayList->setHeaderLabels(dlHeaders);

	QString raw = ConvertUtil::asRaw(0);
	QString text = ConvertUtil::asText(0);
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		std::stringstream idx;
		idx << i;
		m_DisplayListCopy[i] = 0;
		m_DisplayListUpdate[i] = false;
		m_DisplayListItems[i] = new QTreeWidgetItem(m_DisplayList);
		m_DisplayListItems[i]->setText(0, idx.str().c_str());
		m_DisplayListItems[i]->setText(1, raw);
		m_DisplayListItems[i]->setText(2, text);
	}

	m_DisplayListItems[0]->setText(0, "9999");
	m_DisplayListItems[0]->setText(1, "0x000000000");
	for (int i = 0; i < 2; ++i)
	{
		m_DisplayList->resizeColumnToContents(i);
	}
	m_DisplayListItems[0]->setText(0, "0");
	m_DisplayListItems[0]->setText(1, raw);

	updateData();
	updateView();
	connect(m_CopyAct, &QAction::triggered, this, &RamDL::onCopy);
	connect(m_DisplayList, &QTreeWidget::customContextMenuRequested, this,
	    &RamDL::onPrepareContextMenu);
	connect(m_Inspector, &Inspector::updateData, this, &RamDL::updateData);
	connect(m_Inspector, &Inspector::updateView, this, &RamDL::updateView);
}

void RamDL::updateData()
{
	if (!g_Emulator) return;

	m_DLCMDCount = 0;
	m_FirstDisplayCMD = -1;
	for (int handle = 0; handle < FTED_NUM_HANDLES; ++handle)
		m_HandleUsage[handle] = false;

	const uint32_t *dl = BT8XXEMU_getDisplayList(g_Emulator);
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		if (m_DisplayListCopy[i] != dl[i])
		{
			m_DisplayListCopy[i] = dl[i];
			m_DisplayListUpdate[i] = true;
		}
		if ((m_DisplayListCopy[i] >> 24) == FTEDITOR_DL_BITMAP_HANDLE)
		{
			uint32_t handle = (m_DisplayListCopy[i] & 0x1F);
			// printf("BITMAP_HANDLE: %i\n", handle);
			if (handle < FTED_NUM_HANDLES)
			{
				m_HandleUsage[handle] = true;
			}
		}

		if (m_DisplayListCopy[i] > 0)
		{
			m_DLCMDCount++;
		}
		else if (m_FirstDisplayCMD == -1)
		{
			m_FirstDisplayCMD = i;
		}
	}
}

void RamDL::updateView()
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		if (m_DisplayListUpdate[i])
		{
			m_DisplayListUpdate[i] = false;
			m_DisplayListItems[i]->setText(1,
			    ConvertUtil::asRaw(m_DisplayListCopy[i]));
			m_DisplayListItems[i]->setText(2,
			    ConvertUtil::asText(m_DisplayListCopy[i]));
		}
	}
}

QByteArray RamDL::getDLBinary(bool isBigEndian)
{
	int columnCount = m_DisplayList->headerItem()->columnCount();
	int currRow = 0, currColumn = 0;
	uint32_t rawValue = 0;
	QTreeWidgetItem *item = 0;
	bool ok;
	bool isDisplayCmd = false;
	int displayCmdPos = 0;
	QString currText;
	QByteArray result;

	typedef union DataConvert
	{
		uint8_t bytes[4];
		uint32_t data;
	} DataConvert;
	DataConvert dataConvert;

	for (currRow = 0; currRow < m_DisplayList->topLevelItemCount(); currRow++)
	{
		item = m_DisplayList->topLevelItem(currRow);
		for (currColumn = 0; currColumn < columnCount; ++currColumn)
		{
			currText = item->text(currColumn);
			if (!currText.startsWith("0x")) continue;

			rawValue = currText.toUInt(&ok, 16);
			if (currText == "0x00000000")
			{
				if (isDisplayCmd && displayCmdPos == (currRow - 1)) return result;
				isDisplayCmd = true;
				displayCmdPos = currRow;
			}
			else if (isBigEndian && ok)
			{
				rawValue = qToBigEndian(rawValue);
			}

			dataConvert.data = rawValue;
			for (int i = 3; i >= 0; --i)
			{
				result.append(dataConvert.bytes[i]);
			}
		}
	}
	return result;
}

QString RamDL::getDLContent(bool isBigEndian)
{
	QString text("");
	int len = m_DisplayList->headerItem()->columnCount();
	int i = 0, j = 0;
	uint32_t lit = 0, big = 0;
	QTreeWidgetItem *item = 0;
	bool ok;
	bool isDisplayCmd = false;
	int pos = 0;
	QString iText("");

	for (i = 0; i < m_DisplayList->topLevelItemCount(); i++)
	{
		item = m_DisplayList->topLevelItem(i);
		for (j = 1; j < len; ++j)
		{
			iText = item->text(j);

			if (iText == "0x00000000")
			{
				if (isDisplayCmd && pos == (i - 1))
				{
					return text;
				}

				isDisplayCmd = true;
				pos = i;
			}

			if (iText.startsWith("0x"))
			{
				if (isBigEndian)
				{
					lit = iText.toUInt(&ok, 16);
					big = 0;
					if (ok)
					{
						big = qToBigEndian(lit);
					}
					text += QString("0x%1").arg(big, 8, 16, QChar('0')) + "\t// ";
				}
				else
				{
					text += iText + "\t// ";
				}
			}
			else
			{
				text += iText + '\t';
			}
		}
		text.replace(text.length() - 1, 1, '\n');
	}

	return text;
}

QTreeWidget *RamDL::DisplayList() const { return m_DisplayList; }

bool *RamDL::handleUsage() const { return m_HandleUsage; }

void RamDL::onPrepareContextMenu(const QPoint &pos)
{
	QTreeWidget *treeWidget = dynamic_cast<QTreeWidget *>(sender());

	m_ContextMenu->exec(treeWidget->mapToGlobal(pos));
}

int RamDL::firstDisplayCMD() const { return m_FirstDisplayCMD; }

bool RamDL::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_DisplayList && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->matches(QKeySequence::Copy))
		{
			onCopy();
			return true;
		}
	}

	if ((watched == m_DisplayList || watched == m_DisplayList->viewport())
	    && event->type() == QEvent::Wheel)
	{
		auto e = static_cast<QWheelEvent *>(event);
		if (e->modifiers() & Qt::ControlModifier)
		{
			int angle = e->angleDelta().y();
			auto ptSize = m_DisplayList->font().pointSize();
			if (angle > 0 && ptSize < 180)
			{
				m_DisplayList->setFont(QFont("Segoe UI", ptSize + 2));
			}
			else if (angle <= 0 && ptSize > 5)
			{
				m_DisplayList->setFont(QFont("Segoe UI", ptSize - 2));
			}
			return true;
		}
	}
	return QWidget::eventFilter(watched, event);
}

int RamDL::dlCMDCount() const { return m_DLCMDCount; }

void RamDL::onCopy() { CommonUtil::copy(m_DisplayList); }

} // namespace FTEDITOR
