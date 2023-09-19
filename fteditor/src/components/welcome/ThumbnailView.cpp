/*!
 * @file ThumbnailView.cpp
 * @date 6/2/2023
 * @author Liem Do <liem.do@brtchip.com>
 */

#include "ThumbnailView.h"

#include <QDir>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScrollArea>

#include "ExampleDialog.h"
#include "define/ValueDefine.h"
#include "utils/ImageUtil.h"

namespace FTEDITOR {

GridLayout::GridLayout(QWidget *parent) : QGridLayout(parent) {}

void GridLayout::clear() {
  for (int r = 0; r < rowCount(); ++r) {
    for (int c = 0; c < columnCount(); ++c) {
      QLayoutItem *child;
      if ((child = itemAtPosition(r, c)) != nullptr) {
        delete child->widget();
      }
    }
  }
}

Thumbnail::Thumbnail(QString path)
    : m_overlayHeight(0),
      m_isEntered(false),
      m_description(tr("No description.")) {
  setFont(QFont("Segoe UI", 10));
  setOverlayHeight(0);

  QFileInfo f(path);
  m_prjName = f.completeBaseName();
  m_path = f.absoluteFilePath();

  m_animation = new QPropertyAnimation(this, "overlayHeight");
  m_animation->setDuration(250);
  m_animation->setEasingCurve(QEasingCurve::OutCubic);

  QString absPath = f.absolutePath();
  QString thumbnailPath = absPath + "/overview/" + m_prjName + ".png";
  QFile file(thumbnailPath);
  m_thumbnailPath = file.exists() ? thumbnailPath : ":/icons/black.png";

  QString introFilePath = absPath + "/overview/overview.json";
  QFile fileIntro(introFilePath);
  if (fileIntro.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString val = fileIntro.readAll();
    fileIntro.close();
    auto doc = QJsonDocument::fromJson(val.toUtf8());
    auto objArray = doc.array();
    for (auto jsonValue : objArray) {
      auto obj = jsonValue.toObject();
      if (obj["project_name"].toString() == m_prjName) {
        m_description = obj["description"].toString();
      }
    }
  }
  connect(this, &Thumbnail::overlayHeightChanged, this, [this]() { update(); });
}

void Thumbnail::enterEvent(QEnterEvent *event) {
  m_animation->stop();
  m_isEntered = true;
  m_animation->setEndValue(height());
  m_animation->start();
  update();
  QWidget::enterEvent(event);
}

void Thumbnail::leaveEvent(QEvent *event) {
  m_animation->stop();
  m_isEntered = false;
  m_overlayHeight = 100;
  update();
  QWidget::leaveEvent(event);
}

void Thumbnail::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);
  QPainter p(this);
  p.setPen(Qt::blue);

  QColor color(0xff, 0xff, 0xff, 0xE7);
  int curX = 0, curY = 0;
  p.fillRect(curX, curY, width(), height(), color);

  const int imgHeight = 170;
  curX = 5, curY = 5;
  if (m_isEntered) {
    auto effect = new QGraphicsBlurEffect;
    effect->setBlurRadius(12);
    p.drawImage(
        QRect(curX, curY, width() - curX * 2, imgHeight),
        ImageUtil::applyEffectToImage(QImage(m_thumbnailPath), effect, 1));
  } else {
    p.drawImage(QRect(curX, curY, width() - curX * 2, imgHeight),
                QImage(m_thumbnailPath));
  }
  curX += 10;
  if (m_isEntered) {
    p.fillRect(0, height() - m_overlayHeight, width(), m_overlayHeight, color);
    p.setPen(Qt::black);
    p.drawText(QRect(curX, curY + 10, width() - curX * 2, imgHeight - curY * 2),
               m_description);
  }

  p.setPen(Qt::black);
  curY = curY + imgHeight + 5;
  p.drawLine(10, curY, width() - 10, curY);

  curY += 23;
  QFont f("Segoe UI", font().pointSize() + 1);
  p.setFont(f);
  p.drawText(curX, curY, m_prjName);
  p.end();
}

void Thumbnail::mousePressEvent(QMouseEvent *event) {
  emit itemPressed(m_path);
}

const QString &Thumbnail::projectName() const { return m_prjName; }

void Thumbnail::setProjectName(const QString &newPrjName) {
  m_prjName = newPrjName;
}

int Thumbnail::overlayHeight() const { return m_overlayHeight; }

void Thumbnail::setOverlayHeight(int newOverlayHeight) {
  if (newOverlayHeight == m_overlayHeight) return;
  m_overlayHeight = newOverlayHeight;
  emit overlayHeightChanged();
}

ThumbnailView::ThumbnailView(ExampleDialog *parent, QString path)
    : m_exampleDlg(parent), m_fileInfoList(QList<QFileInfo>()) {
  m_gridLyt = new GridLayout;
  m_gridLyt->setSizeConstraint(QLayout::SetFixedSize);

  auto mainLyt = new QHBoxLayout;
  auto addProject = [&, this](QFileInfo item) {
    if (item.isFile() && item.suffix() == PROJECT_EXTENSION) {
      m_fileInfoList.append(item);
    }
  };

  QDir dir(path);
  auto lv1InfoList =
      dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

  for (auto &lv1Item : lv1InfoList) {
    if (!lv1Item.isDir()) {
      addProject(lv1Item);
      continue;
    }
    QDir itemDir(lv1Item.absoluteFilePath());
    auto lv2InfoList =
        itemDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (auto &lv2Item : lv2InfoList) {
      addProject(lv2Item);
    }
  }

  auto scrollWidget = new QWidget;
  scrollWidget->setLayout(m_gridLyt);
  auto scrollArea = new QScrollArea;
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(scrollWidget);
  mainLyt->addWidget(scrollArea);

  setLayout(mainLyt);
}

void ThumbnailView::addThumbnail(QString path, int row, int column) {
  auto thumb = new Thumbnail(path);
  thumb->setFixedWidth(230);
  thumb->setFixedHeight(220);
  connect(thumb, &Thumbnail::itemPressed, m_exampleDlg,
          &ExampleDialog::onItemPressed);
  m_gridLyt->addWidget(thumb, row, column);
}

void ThumbnailView::updateViewContent(QStringList paths) {
  m_gridLyt->clear();
  const int MAX_COLUMN = 4;
  int row = 0, column = 0;
  for (auto &path : paths) {
    addThumbnail(path, row, column);
    ++column;
    if (column % MAX_COLUMN == 0) {
      ++row;
      column = 0;
    }
  }
}

QStringList ThumbnailView::search(const QString &text) {
  QStringList result;
  QRegularExpression regEx(text, QRegularExpression::CaseInsensitiveOption);
  if (!regEx.isValid()) return result;
  QStringList nameList;
  QMap<QString, QString> mapPathAndName;
  for (auto &file : m_fileInfoList) {
    mapPathAndName.insert(file.absoluteFilePath(), file.fileName());
    nameList.append(file.fileName());
  }
  nameList = nameList.filter(regEx);
  for (auto &path : nameList) {
    result.append(mapPathAndName.keys(path));
  }
  return result;
}

}  // namespace FTEDITOR
