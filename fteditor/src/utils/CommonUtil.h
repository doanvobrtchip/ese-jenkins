/*!
 * @file CommonUtil.h
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef COMMONUTIL_H
#define COMMONUTIL_H

class QTreeWidget;
class QString;
class QPlainTextEdit;
class QJsonArray;
class QTextDocument;

class CommonUtil {
 public:
  static void copy(const QTreeWidget *widget);
  static bool existProject(QString path);
  static void documentFromJsonArray(QPlainTextEdit *textEditor,
                                    const QJsonArray &arr);
  static QJsonArray documentToJsonArray(const QTextDocument *textDocument,
                                        bool coprocessor, bool exportScript);
};

#endif  // COMMONUTIL_H
