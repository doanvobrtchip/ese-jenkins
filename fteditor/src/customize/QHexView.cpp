#include "QHexView.h"

#include <utils/LoggerUtil.h>

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSize>
#include <QtEndian>
#include <stdexcept>

#include "content_manager.h"

const int MIN_HEXCHARS_IN_LINE = 8;
const int GAP_ADR_HEX = 5;
const int GAP_HEX_ASCII = 16;
const int MIN_BYTES_PER_LINE = 2;
const int ADR_LENGTH = 8;

namespace FTEDITOR {

QHexView::QHexView(QWidget *parent)
    : QAbstractScrollArea(parent), m_pdata(NULL) {
  setFont(QFont("Courier", 10));

  m_charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
  m_charHeight = fontMetrics().height();

  m_posAddr = 0;
  m_posHex = ADR_LENGTH * m_charWidth + GAP_ADR_HEX;
  m_posAscii = m_posHex + MIN_HEXCHARS_IN_LINE * m_charWidth + GAP_HEX_ASCII;
  m_bytesPerLine = MIN_BYTES_PER_LINE;

  setMinimumWidth(m_posAscii + (MIN_BYTES_PER_LINE * m_charWidth));

  setFocusPolicy(Qt::StrongFocus);
  m_cursorPos = 0;
  resetSelection(0);

  QAction *actCopy = new QAction;
  actCopy->setText(tr("Copy"));
  m_ContextMenu = new QMenu(this);
  m_ContextMenu->addAction(actCopy);
  connect(actCopy, &QAction::triggered, this, &QHexView::onCopy);
  connect(this, &QWidget::customContextMenuRequested, this,
          [this](const QPoint &pos) {
            m_ContextMenu->exec(this->mapToGlobal(pos));
          });
}

QHexView::~QHexView() {
  if (m_pdata) delete m_pdata;
}

void QHexView::setData(QHexView::DataStorage *pData) {
  QMutexLocker lock(&m_dataMtx);

  if (m_pdata) delete m_pdata;
  m_pdata = pData;
}

QHexView::DataStorage *QHexView::data() { return m_pdata; }

void QHexView::showFromOffset(std::size_t offset) {
  QMutexLocker lock(&m_dataMtx);

  if (m_pdata && offset < m_pdata->size()) {
    updatePositions();
    setCursorPos(offset * 2);

    if (!checkVisible()) {
      int cursorY = m_cursorPos / (2 * m_bytesPerLine);
      if (cursorY > 0) cursorY--;

      verticalScrollBar()->setValue(cursorY);
      viewport()->update();
    }
  }
}

void QHexView::clear() {
  QMutexLocker lock(&m_dataMtx);

  if (m_pdata) {
    delete m_pdata;
    m_pdata = NULL;
  }
  verticalScrollBar()->setValue(0);
  viewport()->update();
}

QSize QHexView::fullSize() const {
  if (!m_pdata) return QSize(0, 0);

  std::size_t width = m_posAscii + (m_bytesPerLine * m_charWidth);
  std::size_t height = m_pdata->size() / m_bytesPerLine;
  if (m_pdata->size() % m_bytesPerLine) height++;

  height *= m_charHeight;

  return QSize(width, height);
}

void QHexView::updatePositions() {
  m_charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
  m_charHeight = fontMetrics().height();

  int serviceSymbolsWidth =
      ADR_LENGTH * m_charWidth + GAP_ADR_HEX + GAP_HEX_ASCII + 11;

  setBytesPerLine((width() - serviceSymbolsWidth) /
                  (4 * m_charWidth + 1));  // 3 bytes for hex + 1 bytes for char
  if (m_bytesPerLine <= 0) setBytesPerLine(1);

  m_posAddr = 0;
  m_posHex = ADR_LENGTH * m_charWidth + GAP_ADR_HEX;
  m_posAscii =
      m_posHex + (m_bytesPerLine * 3 - 1) * m_charWidth + GAP_HEX_ASCII;

  QSize areaSize = viewport()->size();
  QSize widgetSize = fullSize();
  verticalScrollBar()->setPageStep(areaSize.height() / m_charHeight);
  verticalScrollBar()->setRange(
      0, (widgetSize.height() - areaSize.height()) / m_charHeight + 2);
}

void QHexView::paintEvent(QPaintEvent *event) {
  QMutexLocker lock(&m_dataMtx);
  if (!m_pdata) return;
  QPainter painter(viewport());

  updatePositions();

  QSize areaSize = viewport()->size();
  int firstLineIdx = verticalScrollBar()->value();

  int lastLineIdx = firstLineIdx + areaSize.height() / m_charHeight;
  if ((unsigned int)lastLineIdx > m_pdata->size() / m_bytesPerLine) {
    lastLineIdx = m_pdata->size() / m_bytesPerLine;
    if (m_pdata->size() % m_bytesPerLine) lastLineIdx++;
  }

  painter.fillRect(event->rect(), this->palette().color(QPalette::Base));

  int linePos = m_posAscii - (GAP_HEX_ASCII / 2);
  // Draw address
  QColor nonContentColor = QColor(0xcc, 0xe6, 0xff);
  int yPosStart = m_charHeight;
  bool isValidFocus = false;

  // Draw focus
  if (hasFocus()) {
    int xFocus = (m_cursorPos % (2 * m_bytesPerLine));
    int yFocus = m_cursorPos / (2 * m_bytesPerLine);
    yFocus -= firstLineIdx;
    int cursorX = (((xFocus / 2) * 3) + (xFocus % 2)) * m_charWidth + m_posHex;
    int cursorY = yFocus * m_charHeight + m_charHeight + 3;
    if (cursorY >= yPosStart && m_selectBegin != m_selectEnd)
      isValidFocus = true;

    if (isValidFocus) {
      QColor bgColor = QColor(0xd1, 0x1a, 0xff, 0xFF);
      painter.fillRect(cursorX - 0.3 * m_charWidth, cursorY, 2.8 * m_charWidth,
                       m_charHeight - 1, bgColor);

      // Ascii
      int cursorXAscii = (xFocus / 2) * (m_charWidth + 1) + m_posAscii;
      painter.fillRect(cursorXAscii, cursorY, m_charWidth, m_charHeight,
                       bgColor);
    }
  }

  // Draw header
  for (int i = 0, xPos = m_posHex, yPos = yPosStart; i < m_bytesPerLine;
       ++i, xPos += 3 * m_charWidth) {
    if (!hasFocus() || i != ((m_cursorPos / 2) % m_bytesPerLine) ||
        (hasFocus() && !isValidFocus)) {
      painter.fillRect(
          QRect(xPos - 0.3 * m_charWidth, yPos - 0.9 * m_charHeight,
                2.8 * m_charWidth, m_charHeight + 1),
          nonContentColor);
    }
    QString headerText = QString("%1").arg(i, 2, 16, QChar('0'));
    painter.drawText(xPos, yPos, headerText);
  }

  painter.setPen(Qt::gray);

  painter.drawLine(linePos, event->rect().top(), linePos, height());

  painter.setPen(Qt::black);

  yPosStart = startY();

  auto data = m_pdata->getData(firstLineIdx * m_bytesPerLine,
                               (lastLineIdx - firstLineIdx) * m_bytesPerLine);

  int index = 0;
  for (int lineIdx = firstLineIdx, y = yPosStart; lineIdx < lastLineIdx;
       lineIdx += 1, y += m_charHeight) {
    painter.setPen(QColor(0, 0, 0));
    if (!hasFocus() || lineIdx != ((m_cursorPos / 2) / m_bytesPerLine) ||
        (hasFocus() && !isValidFocus)) {
      painter.fillRect(QRect(m_posAddr, y - 0.8 * m_charHeight,
                             m_posHex - 0.8 * m_charWidth, m_charHeight - 1),
                       nonContentColor);
    }
    QString address =
        QString("%1").arg(lineIdx * m_bytesPerLine, ADR_LENGTH, 16, QChar('0'));
    painter.drawText(m_posAddr, y, address);

    for (int x = m_posHex, i = 0, xAscii = m_posAscii;
         i < m_bytesPerLine &&
         ((lineIdx - firstLineIdx) * m_bytesPerLine + i) < data.size();
         i++, x += 3 * m_charWidth, xAscii += m_charWidth + 1) {
      // Default transparency and color of the text
      painter.setPen(QColor(0, 0, 0, 128));
      auto pos = (lineIdx * m_bytesPerLine + i) * 2;
      index = (lineIdx - firstLineIdx) * m_bytesPerLine + i;

      // Draw background for value if selected
      if (pos >= m_selectBegin && pos < m_selectEnd) {
        // Background hex
        painter.fillRect(QRect(x - 0.3 * m_charWidth, y - 0.8 * m_charHeight,
                               2.8 * m_charWidth, m_charHeight - 1),
                         QColor(0xb3, 0xff, 0xd9, 0xAF));

        // Background char
        painter.fillRect(
            QRect(xAscii, y - 0.8 * m_charHeight, m_charWidth, m_charHeight),
            QColor(0xb3, 0xff, 0xd9, 0xAF));
      }

      // Set color for memory area of content item
      int globalIndex = lineIdx * m_bytesPerLine + i;
      ContentArea *area;
      for (int listIdx = m_contentAreaList.count() - 1; listIdx >= 0;
           --listIdx) {
        area = m_contentAreaList.at(listIdx);
        if (area->contentInfo->MemoryLoaded && globalIndex >= area->start() &&
            globalIndex <= area->end()) {
          painter.setPen(area->color);
          break;
        }
      }

      // Draw hex value
      QString val = QString::number((data.at(index) & 0xF0) >> 4, 16);
      painter.drawText(x, y, val);

      val = QString::number((data.at(index) & 0xF), 16);
      painter.drawText(x + m_charWidth, y, val);

      // Draw ascii chars
      char ch = data[(lineIdx - firstLineIdx) * (uint)m_bytesPerLine + i];
      auto unsignedCh = static_cast<uint8_t>(ch);
      if (unsignedCh < 0x20 || (unsignedCh > 0x7E && unsignedCh < 0xA0))
        ch = '.';
      painter.drawText(xAscii, y, QString(ch));
    }
  }
}

void QHexView::keyPressEvent(QKeyEvent *event) {
  QMutexLocker lock(&m_dataMtx);

  bool setVisible = false;

  /*****************************************************************************/
  /* Cursor movements */
  /*****************************************************************************/
  if (event->matches(QKeySequence::MoveToNextChar)) {
    setCursorPos(m_cursorPos + 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToPreviousChar)) {
    setCursorPos(m_cursorPos - 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToEndOfLine)) {
    setCursorPos(2 * m_bytesPerLine * (m_cursorPos / (m_bytesPerLine * 2) + 1) -
                 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToStartOfLine)) {
    setCursorPos(m_cursorPos - (m_cursorPos % (m_bytesPerLine * 2)));
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToPreviousLine)) {
    setCursorPos(m_cursorPos - m_bytesPerLine * 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToNextLine)) {
    setCursorPos(m_cursorPos + m_bytesPerLine * 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToNextPage)) {
    setCursorPos(m_cursorPos + (viewport()->height() / m_charHeight - 1) * 2 *
                                   m_bytesPerLine);
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToPreviousPage)) {
    int pos = m_cursorPos -
              (viewport()->height() / m_charHeight - 1) * 2 * m_bytesPerLine;
    if (pos < 0 && m_cursorPos != 0) {
      pos = m_cursorPos % (m_bytesPerLine * 2);
    }
    setCursorPos(pos);
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToEndOfDocument)) {
    if (m_pdata) setCursorPos(m_pdata->size() * 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::MoveToStartOfDocument)) {
    setCursorPos(0);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  /*****************************************************************************/
  /* Select commands */
  /*****************************************************************************/
  if (event->matches(QKeySequence::SelectAll)) {
    resetSelection(0);
    if (m_pdata) setSelection(2 * m_pdata->size() + 1);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectNextChar)) {
    std::size_t pos = m_cursorPos + 2;
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectPreviousChar)) {
    std::size_t pos = m_cursorPos - 2;
    setSelection(pos);
    setCursorPos(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectEndOfLine)) {
    std::size_t pos = m_cursorPos - (m_cursorPos % (2 * m_bytesPerLine)) +
                      (2 * m_bytesPerLine) - 2;

    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectStartOfLine)) {
    std::size_t pos = m_cursorPos - (m_cursorPos % (2 * m_bytesPerLine));
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectPreviousLine)) {
    std::size_t pos = m_cursorPos - (2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectNextLine)) {
    std::size_t pos = m_cursorPos + (2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectNextPage)) {
    std::size_t pos =
        m_cursorPos +
        (((viewport()->height() / m_charHeight) - 1) * 2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectPreviousPage)) {
    int pos = m_cursorPos -
              (viewport()->height() / m_charHeight - 1) * 2 * m_bytesPerLine;
    if (pos < 0 && m_cursorPos != 0) {
      pos = m_cursorPos % (m_bytesPerLine * 2);
    }
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectEndOfDocument)) {
    std::size_t pos = 0;
    if (m_pdata) pos = m_pdata->size() * 2;
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }
  if (event->matches(QKeySequence::SelectStartOfDocument)) {
    std::size_t pos = 0;
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::Copy)) {
    onCopy();
  }

  if (setVisible) ensureVisible();
  viewport()->update();
}

void QHexView::mouseMoveEvent(QMouseEvent *event) {
  if (!isValidMouseEvent(event)) return;
  int actPos = cursorPos(event->position());

  QMutexLocker lock(&m_dataMtx);
  setCursorPos(actPos);
  setSelection(actPos);

  ensureVisible();
  viewport()->update();
}

void QHexView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::RightButton) {
    emit this->customContextMenuRequested(event->position().toPoint());
    return;
  }
  if (!isValidMouseEvent(event)) return;
  int cPos = cursorPos(event->position());

  if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) &&
      event->button() == Qt::LeftButton)
    setSelection(cPos, true);
  else
    resetSelection(cPos);

  if (cPos != std::numeric_limits<std::size_t>::max()) {
    QMutexLocker lock(&m_dataMtx);

    setCursorPos(cPos);
  }
  updateUint();
  viewport()->update();
}

void QHexView::onCopy() {
  if (m_pdata) {
    QString res;
    int idx = 0;
    int copyOffset = 0;

    QByteArray data = m_pdata->getData(m_selectBegin / 2,
                                       (m_selectEnd - m_selectBegin) / 2 + 1);
    if (m_selectBegin % 2) {
      res += QString::number((data.at((idx + 1) / 2) & 0xF), 16);
      res += " ";
      idx++;
      copyOffset = 1;
    }

    int selectedSize = m_selectEnd - m_selectBegin;
    for (; idx < selectedSize; idx += 2) {
      if (data.size() > (copyOffset + idx) / 2) {
        QString val =
            QString::number((data.at((copyOffset + idx) / 2) & 0xF0) >> 4, 16);
        if (idx + 1 < selectedSize) {
          val += QString::number((data.at((copyOffset + idx) / 2) & 0xF), 16);
          val += " ";
        }

        res += val;

        if ((idx / 2) % m_bytesPerLine == (m_bytesPerLine - 1)) {
          if (res.at(res.size() - 1) == ' ') {
            res.remove(res.size() - 1, 1);
          }
          res += "\n";
        }
      }
    }

    if (res.size() > 0 &&
        (res.at(res.size() - 1) == ' ' || res.at(res.size() - 1) == '\n')) {
      res.remove(res.size() - 1, 1);
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(res);
  }
}

int QHexView::cursorPos(const QPointF &posF) {
  auto position = posF.toPoint();
  int pos = 0;

  if (((std::size_t)position.x() >= m_posHex) &&
      ((std::size_t)position.x() <
       (m_posHex + (m_bytesPerLine * 3 - 1) * m_charWidth))) {
    int x = (position.x() - m_posHex) / (int)m_charWidth;
    x = (x / 3) * 2;

    int firstLineIdx = verticalScrollBar()->value();
    int y = (position.y() / (int)m_charHeight - 1) * 2 * m_bytesPerLine;
    int calPos = x + y + firstLineIdx * m_bytesPerLine * 2;
    pos = calPos > 0 ? calPos : x;
  }
  return pos;
}

const size_t QHexView::startY() const { return 2 * m_charHeight; }

void QHexView::addContentArea(ContentInfo *contentInfo) {
  auto contentArea = new ContentArea(contentInfo);
  contentArea->color = colours[m_contentAreaList.size() % colours.size()];
  debugLog(QString("Add new content - Name: %1 | Color: %2")
               .arg(contentInfo->DestName, contentArea->color.name()));
  m_contentAreaList.append(contentArea);
}

void QHexView::removeContentArea(ContentInfo *contentInfo) {
  for (auto &area : m_contentAreaList) {
    if (area->contentInfo == contentInfo) {
      delete area;
      m_contentAreaList.removeOne(area);
      return;
    }
  }
}

ContentInfo *QHexView::detectCurrentContentInfo() {
  auto beginIdx = m_selectBegin / 2;
  auto endIdx = m_selectEnd / 2 - 1;
  for (int i = m_contentAreaList.count() - 1; i >= 0; --i) {
    auto area = m_contentAreaList.at(i);
    if ((beginIdx >= area->start() && beginIdx <= area->end()) &&
        (endIdx >= area->start() && endIdx <= area->end())) {
      return area->contentInfo;
    }
  }
  return nullptr;
}

void QHexView::setBytesPerLine(int bytesPerLine) {
  if (m_bytesPerLine == bytesPerLine) return;
  m_bytesPerLine = bytesPerLine;
  emit bytesPerLineChanged(m_bytesPerLine);
}

void QHexView::resetSelection() {
  m_selectBegin = m_selectInit;
  m_selectEnd = m_selectInit;
}

void QHexView::resetSelection(std::size_t pos) {
  if (pos == std::numeric_limits<std::size_t>::max()) pos = 0;

  m_selectInit = pos;
  m_selectBegin = pos;
  m_selectEnd = m_selectBegin + 2;

  auto contentInfo = detectCurrentContentInfo();
  emit currentInfoChanged(contentInfo);
  // Ensure not to lose the current cusor because updating empty current content
  // item
  if (!contentInfo) {
    m_selectInit = pos;
    m_selectBegin = pos;
    m_selectEnd = m_selectBegin + 2;
  }
}

void QHexView::setSelection(std::size_t pos, bool shouldEmitUpdate) {
  if (pos == std::numeric_limits<std::size_t>::max()) pos = 0;

  if ((std::size_t)pos >= m_selectInit) {
    m_selectEnd = pos;
    m_selectBegin = m_selectInit;
  } else {
    m_selectBegin = pos;
    m_selectEnd = m_selectInit;
  }
  m_selectEnd += 2;

  if (shouldEmitUpdate) {
    auto contentInfo = detectCurrentContentInfo();
    emit currentInfoChanged(contentInfo);
  }
}

void QHexView::setSelected(std::size_t offset, std::size_t length) {
  m_selectInit = m_selectBegin = offset * 2;
  m_selectEnd = m_selectBegin + length * 2;
  viewport()->update();
}

void QHexView::setCursorPos(int position) {
  int maxPos = 0;
  if (m_pdata) {
    maxPos = (m_pdata->size() - 1) * 2;
  }
  if (position < 0) return;

  if (position > maxPos) position = maxPos;

  m_cursorPos = position;
}

void QHexView::ensureVisible() {
  QSize areaSize = viewport()->size();

  int firstLineIdx = verticalScrollBar()->value();
  int lastLineIdx = firstLineIdx + areaSize.height() / m_charHeight - 2;

  int cursorY = m_cursorPos / (2 * m_bytesPerLine);
  if (cursorY < firstLineIdx) {
    verticalScrollBar()->setValue(cursorY);
  } else if (cursorY > lastLineIdx) {
    int newValue = cursorY - (areaSize.height() - startY()) / m_charHeight;
    verticalScrollBar()->setValue(newValue);
  }
}

void QHexView::updateUint() {
  int byteNum = 4;
  QByteArray data = m_pdata->getData(m_selectBegin / 2, byteNum);
  if (data.size() != byteNum) return;
  QString resultStr = "0x";
  for (auto &byte : data) {
    uint8_t u = static_cast<uint8_t>(byte);
    resultStr += QString("%1").arg(u, 2, 16, QChar('0'));
  }
  bool ok;
  uint resultUint = 0;
  resultUint = resultStr.toUInt(&ok, 16);
  if (ok) {
    resultUint = qToBigEndian(resultUint);
    emit uintChanged(resultUint);
  }
  debugLog(
      QString("ResultStr: %1 | BigEndian: %2").arg(resultStr).arg(resultUint));
}

bool QHexView::checkVisible() {
  QSize areaSize = viewport()->size();

  int firstLineIdx = verticalScrollBar()->value();
  int lastLineIdx = firstLineIdx + areaSize.height() / m_charHeight - 2;

  int cursorY = m_cursorPos / (2 * m_bytesPerLine);
  if (cursorY < firstLineIdx || cursorY > lastLineIdx) {
    return false;
  }
  return true;
}

bool QHexView::isValidMouseEvent(QMouseEvent *event) {
  if (!hasFocus()) return false;
  int y =
      (event->position().toPoint().y() / m_charHeight - 1) * 2 * m_bytesPerLine;
  if (y < 0 || event->position().toPoint().x() < m_posHex ||
      event->position().toPoint().x() > m_posAscii - GAP_HEX_ASCII) {
    return false;
  };
  return true;
}

QHexView::DataStorageArray::DataStorageArray(const QByteArray &arr) {
  m_data = arr;
}

QByteArray QHexView::DataStorageArray::getData(std::size_t position,
                                               std::size_t length) {
  return m_data.mid(position, length);
}

std::size_t QHexView::DataStorageArray::size() { return m_data.size(); }

QHexView::DataStorageFile::DataStorageFile(const QString &fileName)
    : m_file(fileName) {
  m_file.open(QIODevice::ReadOnly);
  if (!m_file.isOpen())
    throw std::runtime_error(std::string("Failed to open file `") +
                             fileName.toStdString() + "`");
}

QByteArray QHexView::DataStorageFile::getData(std::size_t position,
                                              std::size_t length) {
  m_file.seek(position);
  return m_file.read(length);
}

std::size_t QHexView::DataStorageFile::size() { return m_file.size(); }

QHexView::ContentArea::ContentArea(ContentInfo *contentInfo) {
  this->contentInfo = contentInfo;
}

int QHexView::ContentArea::start() { return contentInfo->MemoryAddress; }

int QHexView::ContentArea::end() {
  return contentInfo->MemoryAddress + contentInfo->CachedMemorySize - 1;
}
}  // namespace FTEDITOR
