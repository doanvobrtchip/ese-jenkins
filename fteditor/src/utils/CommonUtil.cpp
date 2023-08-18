/*!
 * @file CommonUtilonutil.cpp
 * @date 2/14/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "CommonUtil.h"

#include <define/ValueDefine.h>

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QJsonArray>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocument>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "dl_editor.h"

void CommonUtil::copy(const QTreeWidget *widget) {
  QList<QTreeWidgetItem *> selectedItems = widget->selectedItems();
  QClipboard *clip = QApplication::clipboard();
  QString copy_text("");

  // get header
  int len = widget->headerItem()->columnCount();
  int i = 0;

  for (; i < len; ++i) {
    copy_text += widget->headerItem()->text(i) + '\t';
  }
  copy_text.replace(copy_text.length() - 1, 1, '\n');

  foreach (QTreeWidgetItem *item, selectedItems) {
    for (i = 0; i < len; ++i) {
      copy_text += item->text(i) + '\t';
    }
    copy_text.replace(copy_text.length() - 1, 1, '\n');
  }

  clip->setText(copy_text);
}

// Check 2 levels of the path to find a ESE project
bool CommonUtil::existProject(QString path) {
  auto listLv1 =
      QDir(path).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

  for (auto &iLv1 : listLv1) {
    if (iLv1.isFile()) {
      if (iLv1.suffix() == PROJECT_EXTENSION) return true;
      continue;
    }
    auto listLv2 = QDir(iLv1.absoluteFilePath())
                       .entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (auto &iLv2 : listLv2) {
      if (iLv2.suffix() == PROJECT_EXTENSION) return true;
      continue;
    }
  }
  return false;
}

void CommonUtil::documentFromJsonArray(QPlainTextEdit *textEditor,
                                       const QJsonArray &arr) {
  bool firstLine = true;
  for (int i = 0; i < arr.size(); ++i) {
    if (firstLine)
      firstLine = false;
    else
      textEditor->textCursor().insertText("\n");
    textEditor->textCursor().insertText(arr[i].toString());
  }
}

QJsonArray CommonUtil::documentToJsonArray(const QTextDocument *textDocument,
                                           bool coprocessor,
                                           bool exportScript) {
  QJsonArray result;
  for (int i = 0; i < textDocument->blockCount(); ++i) {
    QString line = textDocument->findBlockByNumber(i).text();
    if (exportScript) {
      FTEDITOR::DlParsed parsed;
      FTEDITOR::DlParser::parse(FTEDITOR_CURRENT_DEVICE, parsed, line,
                                coprocessor);
      if (!parsed.ValidId)
        line = "";
      else
        line = FTEDITOR::DlParser::toString(FTEDITOR_CURRENT_DEVICE, parsed);
    }
    result.push_back(line);
  }
  return result;
}
