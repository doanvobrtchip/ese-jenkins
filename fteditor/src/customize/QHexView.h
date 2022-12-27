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
    virtual QByteArray getData(std::size_t position, std::size_t length) = 0;
    virtual std::size_t size() = 0;
  };

  class DataStorageArray : public DataStorage {
   public:
    DataStorageArray(const QByteArray &arr);
    virtual QByteArray getData(std::size_t position, std::size_t length);
    virtual std::size_t size();

   private:
    QByteArray m_data;
  };

  class DataStorageFile : public DataStorage {
   public:
    DataStorageFile(const QString &fileName);
    virtual QByteArray getData(std::size_t position, std::size_t length);
    virtual std::size_t size();

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
  void showFromOffset(std::size_t offset);

 public:
  void setSelected(std::size_t offset, std::size_t length);
  DataStorage *data();
  void resetSelection();
  void addContentArea(ContentInfo *contentInfo);
  void removeContentArea(ContentInfo *contentInfo);
  ContentInfo *detectCurrentContentInfo();
  void setBytesPerLine(int bytesPerLine);
  void setCursorPos(int pos);
  void resetSelection(std::size_t pos);
  void ensureVisible();
  void updateUint();
  bool checkVisible();
  bool isValidMouseEvent(QMouseEvent *event);

 protected:
  void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
 private slots:
  void onCopy();

 private:
  QMutex m_dataMtx;
  DataStorage *m_pdata;
  std::size_t m_posAddr;
  std::size_t m_posHex;
  std::size_t m_posAscii;
  std::size_t m_charWidth;
  std::size_t m_charHeight;

  std::size_t m_selectBegin;
  std::size_t m_selectEnd;
  std::size_t m_selectInit;
  std::size_t m_cursorPos;
  std::size_t m_bytesPerLine;

  QSize fullSize() const;
  void updatePositions();
  void setSelection(std::size_t pos, bool shouldEmitUpdate = false);

  int cursorPos(const QPointF &position);
  const std::size_t startY() const;
  QList<QColor> colours = {QColor(0x66, 0x00, 0x66), QColor(0x00, 0x80, 0x80),
                           QColor(0xcc, 0x33, 0x00), QColor(0x00, 0x66, 0x66),
                           QColor(0x99, 0x00, 0x4d), QColor(0x00, 0x80, 0x00)};

  QList<ContentArea *> m_contentAreaList;
  QMenu *m_ContextMenu;

 signals:
  void currentInfoChanged(ContentInfo *currentInfo);
  void bytesPerLineChanged(int bytesPerLine);
  void uintChanged(uint value);
};

}  // namespace FTEDITOR
#endif
