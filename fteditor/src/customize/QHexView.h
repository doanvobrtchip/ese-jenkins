#ifndef Q_HEX_VIEWER_H_
#define Q_HEX_VIEWER_H_

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>
#include <QMap>
#include <QMutex>

class QMenu;

namespace FTEDITOR {
struct ContentInfo;

class QHexView : public QAbstractScrollArea
{
	Q_OBJECT
public:
	class DataStorage
	{
	public:
		virtual ~DataStorage() {};
		virtual QByteArray getData(int position, int length) = 0;
		virtual int size() = 0;
	};

	class DataStorageArray : public DataStorage
	{
	public:
		DataStorageArray(const QByteArray &arr);
		virtual QByteArray getData(int position, int length);
		virtual int size();

	private:
		QByteArray m_data;
	};

	class DataStorageFile : public DataStorage
	{
	public:
		DataStorageFile(const QString &fileName);
		virtual QByteArray getData(int position, int length);
		virtual int size();

	private:
		QFile m_file;
	};

	struct ContentArea
	{
		ContentArea(ContentInfo *contentInfo);
		ContentInfo *contentInfo;
		QColor color;
		int start();
		int end();
	};

	QHexView(QWidget *parent = 0);
	~QHexView();

public slots:
	void setData(QHexView::DataStorage *pData);
	void clear();
	bool showFromOffset(int offset);

public:
	void setSelected(int offset, int length);
	DataStorage *data();
	void resetSelection();
	void addContentArea(ContentInfo *contentInfo);
	void removeContentArea(ContentInfo *contentInfo);
	ContentInfo *detectCurrentContentInfo();
	void setBytesPerLine(int bytesPerLine);
	void setCursorPos(int pos);
	void resetSelection(int pos);
	void ensureVisible();
	void updateUint();
	bool checkVisible();
	bool isValidMouseEvent(QMouseEvent *event);

	void setUseContentArea(bool newUseContentArea);

	int StartAddress() const;
	void setStartAddress(int newStartAddress);

protected:
	void paintEvent(QPaintEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *e);
private slots:
	void onCopy();

private:
	QMutex m_dataMtx;
	DataStorage *m_pdata;
	int m_posAddr;
	int m_posHex;
	int m_posAscii;
	int m_charWidth;
	int m_charHeight;

	int m_selectBegin;
	int m_selectEnd;
	int m_selectInit;
	int m_cursorPos;
	int m_bytesPerLine;
	bool m_UseContentArea;
	int m_StartAddress;

	QSize fullSize() const;
	void updatePositions();
	void setSelection(int pos, bool shouldEmitUpdate = false);
	int cursorPos(const QPointF &position);
	int lineHeight() const;
	int rowCount() const;

	QList<QColor> colours = { QColor(0x66, 0x00, 0x66), QColor(0x00, 0x80, 0x80),
		QColor(0xcc, 0x33, 0x00), QColor(0x00, 0x66, 0x66),
		QColor(0x99, 0x00, 0x4d), QColor(0x00, 0x80, 0x00) };

	QList<ContentArea *> m_contentAreaList;
	QMenu *m_ContextMenu;

signals:
	void currentInfoChanged(ContentInfo *currentInfo);
	void bytesPerLineChanged(int bytesPerLine);
	void uintChanged(uint value);
};

} // namespace FTEDITOR
#endif
