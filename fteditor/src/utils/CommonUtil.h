/*!
 * @file CommonUtil.h
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef COMMONUTIL_H
#define COMMONUTIL_H

class QTreeWidget;
class QString;

class CommonUtil
{
public:
	static void copy(const QTreeWidget *widget);
	static bool existProject(QString path);
};

#endif // COMMONUTIL_H
