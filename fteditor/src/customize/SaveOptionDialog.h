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
  static bool autoOpen;
  static ByteOrder byteOrder;
  static OutputType outputType;
  static QString savedDirectory;

  Type m_type;
  QString m_filePath;
  bool m_useDefaultName;
  QLineEdit *m_lineEdit;
  QString m_defaultFileName;
  QRadioButton *m_radioString;
  QRadioButton *m_radioBinary;
  QRadioButton *m_radioBigEndian;
  QRadioButton *m_radioLittleEndian;

 public slots:
  void changeByteOrder();
  void changeOutputType();
  void handleFindClicked();
  void updateDefaultName();
  void handleAcceptClicked();

 signals:
  void filePathChanged(QString newPath);
  void handleAccept(QString filePath, int outputType, int byteOrder,
                    bool autoOpen);
};

#endif  // SAVEOPTIONDIALOG_H
