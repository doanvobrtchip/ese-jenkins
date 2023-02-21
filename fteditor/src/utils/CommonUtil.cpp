/*!
 * @file CommonUtilonutil.cpp
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "CommonUtil.h"

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QApplication>
#include <QClipboard>

void CommonUtil::copy(const QTreeWidget *widget)
{
	QList<QTreeWidgetItem *> selectedItems = widget->selectedItems();
	QClipboard *clip = QApplication::clipboard();
	QString copy_text("");

	// get header
	int len = widget->headerItem()->columnCount();
	int i = 0;

	for (; i < len; ++i)
	{
		copy_text += widget->headerItem()->text(i) + '\t';
	}
	copy_text.replace(copy_text.length() - 1, 1, '\n');

	foreach (QTreeWidgetItem *item, selectedItems)
	{
		for (i = 0; i < len; ++i)
		{
			copy_text += item->text(i) + '\t';
		}
		copy_text.replace(copy_text.length() - 1, 1, '\n');
	}

	clip->setText(copy_text);
}
