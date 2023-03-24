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

class QHexView : public QAbstractScrollArea {
  Q_OBJECT
 public:
  class DataStorage {
   public:
    virtual ~DataStorage(){};
    virtual QByteArray getData(int position, int length) = 0;
    virtual int size() = 0;
  };

  class DataStorageArray : public DataStorage {
   public:
    DataStorageArray(const QByteArray &arr);
    virtual QByteArray getData(int position, int length);
    virtual int size();

   private:
    QByteArray m_data;
  };

  class DataStorageFile : public DataStorage {
   public:
    DataStorageFile(const QString &fileName);
    virtual QByteArray getData(int position, int length);
    virtual int size();

   private:
    QFile m_file;
  };

  struct ContentArea {
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
  bool showFromAddress(int address);

 public:
  void updateUint();
  bool checkVisible();
  DataStorage *data();
  void ensureVisible();
  void resetSelection();
  int StartAddress() const;
  void setCursorPos(int pos);
  void resetSelection(int pos);
  bool isValidPress(QMouseEvent *event);
  void setBytesPerLine(int bytesPerLine);
  ContentInfo *detectCurrentContentInfo();
  void setStartAddress(int newStartAddress);
  void addContentArea(ContentInfo *contentInfo);
  void setUseContentArea(bool newUseContentArea);
  void setSelectedOffset(int offset, int length);
  void removeContentArea(ContentInfo *contentInfo);
  void setSelectedAddress(int address, int length);

 protected:
  void wheelEvent(QWheelEvent *e);
  void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
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

  int m_selectEnd;
  int m_selectType;
  int m_selectInit;
  int m_selectBegin;

  int m_cursorPos;
  int m_bytesPerLine;
  int m_startAddress;
  bool m_useContentArea;

  QList<ContentArea *> m_contentAreaList;
  QMenu *m_contextMenu;

  int rowCount() const;
  void updatePositions();
  int lineHeight() const;
  QSize fullSize() const;
  int cursorPos(const QPointF &position);
  void setSelection(int pos, bool shouldEmitUpdate = false);

  QList<QColor> m_colours = {
      QColor(0x66, 0x00, 0x66), QColor(0x00, 0x80, 0x80),
      QColor(0xcc, 0x33, 0x00), QColor(0x00, 0x66, 0x66),
      QColor(0x99, 0x00, 0x4d), QColor(0x00, 0x80, 0x00)};
  enum SelectType { None, Hex, Ascii };

 signals:
  void bytesPerLineChanged(int bytesPerLine);
  void uintChanged(QString valueStr, uint value);
  void currentInfoChanged(FTEDITOR::ContentInfo *currentInfo);
};

}  // namespace FTEDITOR
#endif
