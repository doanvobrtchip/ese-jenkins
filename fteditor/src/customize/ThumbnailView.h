/*!
 * @file ThumbnailView.h
 * @date 6/2/2023
 * @author Liem Do <liem.do@brtchip.com>
 */

#ifndef THUMBNAILVIEW_H
#define THUMBNAILVIEW_H

#include <QWidget>
#include <QGridLayout>

class QPropertyAnimation;
class QFileInfo;

namespace FTEDITOR {
class ExampleDialog;
class ThumbnailView;

class GridLayout : public QGridLayout {
 public:
  GridLayout(QWidget *parent = 0);
  void clear();
};

class Thumbnail : public QWidget {
  Q_OBJECT
  Q_PROPERTY(qreal overlayHeight READ overlayHeight WRITE setOverlayHeight
                 NOTIFY overlayHeightChanged)
  
 signals:
  void overlayHeightChanged();
  void itemPressed(QString path);

 public:
  Thumbnail(QString path = QString());
  int overlayHeight() const;
  void setOverlayHeight(int newOverlayHeight);
  const QString &projectName() const;
  void setProjectName(const QString &newPrjName);

 protected:
  void enterEvent(QEnterEvent *event);
  void leaveEvent(QEvent *event);
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);

 private:
  int m_overlayHeight;
  QString m_path;
  bool m_isEntered;
  QString m_description;
  QString m_prjName;
  QString m_thumbnailPath;
  QPropertyAnimation* m_animation;
};

class ThumbnailView : public QWidget {
  Q_OBJECT

 public:
  ThumbnailView(ExampleDialog *parent, QString path = QString());
  void addThumbnail(QString path, int row, int column);
  void updateViewContent(QStringList path);
  
 public slots:
  void handleItemPressed(const QString &path);
  void onSeach(const QString &text);

 signals:
  void itemPressed(QString path);

 private:
  ExampleDialog *m_exampleDlg;
  QList<QFileInfo> m_fileInfoList;
  GridLayout *m_gridLyt;
};

}  // namespace FTEDITOR
#endif  // THUMBNAILVIEW_H
