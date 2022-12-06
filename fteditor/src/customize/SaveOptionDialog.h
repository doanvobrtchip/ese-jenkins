/*!
 * @file SaveOptionDialog.h
 * @date 11/14/2022
 * @author Liem Do <liem.do@brtchip.com>
 */

#ifndef SAVEOPTIONDIALOG_H
#define SAVEOPTIONDIALOG_H

#include <QDialog>

class QRadioButton;
class QLineEdit;

class SaveOptionDialog : public QDialog {
  Q_OBJECT

 public:
  enum OutputType { Binary, Text };
  enum ByteOrder { LittleEndian, BigEndian };
  enum Type { DisplayList, CoprocessorCommand };

  SaveOptionDialog(SaveOptionDialog::Type type, QWidget *parent = nullptr);

  const QString &getDefaultFileName() const;
  const QString &getFilePath() const;
  void setFilePath(const QString &newFilePath);
  QString generateDefaultFileName();

 private:
  static QString savedDirectory;
  static OutputType outputType;
  static ByteOrder byteOrder;
  static bool autoOpen;

  QRadioButton *radioBinary_;
  QRadioButton *radioString_;
  QRadioButton *radioLittleEndian_;
  QRadioButton *radioBigEndian_;
  QLineEdit *lineEdit_;
  QString defaultFileName_;
  QString filePath_;
  bool useDefaultName_;
  Type type_;

 public slots:
  void changeOutputType();
  void changeByteOrder();
  void handleFindClicked();
  void handleAcceptClicked();
  void updateDefaultName();

 signals:
  void filePathChanged(QString newPath);
  void handleAccept(QString filePath, int outputType, int byteOrder,
                    bool autoOpen);
};

#endif  // SAVEOPTIONDIALOG_H
