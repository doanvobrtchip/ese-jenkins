/*!
 * @file UndoLineEdit.h
 * @date 11/21/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef UNDOLINEEDIT_H
#define UNDOLINEEDIT_H

#include <QUndoCommand>

class QLineEdit;

class UndoLineEdit : public QUndoCommand
{
public:
	UndoLineEdit(int id, QLineEdit *lineEdit, QString oldValue, bool *working);
	~UndoLineEdit() = default;
	
	virtual void undo();
	virtual void redo();
	virtual int id() const;
	virtual bool mergeWith(const QUndoCommand *command);
	
private:
	int m_id;
	QLineEdit *m_lineEdit;
	QString m_newValue;
	QString m_oldValue;
	bool *m_working;
};

#endif // UNDOLINEEDIT_H
