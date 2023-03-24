/*!
 * @file SaveOptionDialog.cpp
 * @date 11/14/2022
 * @author Liem Do <liem.do@brtchip.com>
 */

#include "SaveOptionDialog.h"

#include <QtWidgets>

QString SaveOptionDialog::savedDirectory =
    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
SaveOptionDialog::OutputType SaveOptionDialog::outputType = OutputType::Binary;
SaveOptionDialog::ByteOrder SaveOptionDialog::byteOrder =
    ByteOrder::LittleEndian;
bool SaveOptionDialog::autoOpen = true;

SaveOptionDialog::SaveOptionDialog(SaveOptionDialog::Type type,
                                   QWidget *parent) {
  m_type = type;
  setWindowTitle(m_type == SaveOptionDialog::DisplayList
                     ? tr("Save Display List")
                     : tr("Save Coprocessor Command"));
  m_useDefaultName = true;
  auto label = new QLabel(tr("Save as:"));
  m_lineEdit = new QLineEdit;
  label->setBuddy(m_lineEdit);
  auto findButton = new QPushButton(tr("..."));
  findButton->setAutoDefault(false);
  QHBoxLayout *dirHBoxLayout = new QHBoxLayout;
  dirHBoxLayout->addWidget(label);
  dirHBoxLayout->addWidget(m_lineEdit);
  dirHBoxLayout->addWidget(findButton);

  // Notation group
  m_radioBinary = new QRadioButton;
  m_radioBinary->setText(tr("Binary"));
  m_radioString = new QRadioButton;
  m_radioString->setText(tr("Hexadecimal String"));
  auto notationHBoxLayout = new QHBoxLayout;
  notationHBoxLayout->addWidget(m_radioBinary);
  notationHBoxLayout->addWidget(m_radioString);
  auto notationGroupBox = new QGroupBox(this);
  notationGroupBox->setTitle(tr("Notation"));
  notationGroupBox->setLayout(notationHBoxLayout);

  // Byte order group
  m_radioLittleEndian = new QRadioButton;
  m_radioLittleEndian->setText(tr("Little Endian"));
  m_radioBigEndian = new QRadioButton;
  m_radioBigEndian->setText(tr("Big Endian"));
  auto byteOrderHBoxLayout = new QHBoxLayout;
  byteOrderHBoxLayout->addWidget(m_radioLittleEndian);
  byteOrderHBoxLayout->addWidget(m_radioBigEndian);
  auto byteOrderGroupBox = new QGroupBox;
  byteOrderGroupBox->setTitle(tr("Byte Order"));
  byteOrderGroupBox->setLayout(byteOrderHBoxLayout);

  // Auto open
  auto autoOpenLayout = new QHBoxLayout;
  auto autoOpenCheckBox = new QCheckBox;
  autoOpenCheckBox->setText(tr("Automatically open file"));
  autoOpenLayout->addWidget(autoOpenCheckBox);

  // Button group
  auto acceptButton = new QPushButton(tr("OK"));
  acceptButton->setDefault(true);
  auto rejectButton = new QPushButton(tr("Cancel"));
  auto buttonBox = new QDialogButtonBox(Qt::Horizontal);
  buttonBox->addButton(acceptButton, QDialogButtonBox::AcceptRole);
  buttonBox->addButton(rejectButton, QDialogButtonBox::RejectRole);

  // General
  auto verticalLayout = new QVBoxLayout;
  verticalLayout->addLayout(dirHBoxLayout);
  verticalLayout->addWidget(notationGroupBox);
  verticalLayout->addWidget(byteOrderGroupBox);
  verticalLayout->addLayout(autoOpenLayout);
  verticalLayout->addWidget(buttonBox);

  setLayout(verticalLayout);
  setWindowIcon(QIcon(":/icons/eve-puzzle-16.png"));
  outputType == SaveOptionDialog::Binary ? m_radioBinary->setChecked(true)
                                         : m_radioString->setChecked(true);
  byteOrder == SaveOptionDialog::BigEndian
      ? m_radioBigEndian->setChecked(true)
      : m_radioLittleEndian->setChecked(true);
  autoOpenCheckBox->setChecked(autoOpen);

  connect(autoOpenCheckBox, &QAbstractButton::clicked, this,
          [](bool checked) { autoOpen = checked; });
  connect(m_radioLittleEndian, SIGNAL(clicked()), this,
          SLOT(updateDefaultName()));
  connect(m_radioLittleEndian, SIGNAL(clicked()), this,
          SLOT(changeByteOrder()));
  connect(m_radioBigEndian, SIGNAL(clicked()), this, SLOT(updateDefaultName()));
  connect(m_radioBigEndian, SIGNAL(clicked()), this, SLOT(changeByteOrder()));
  connect(m_radioString, SIGNAL(clicked()), this, SLOT(updateDefaultName()));
  connect(m_radioString, SIGNAL(clicked()), this, SLOT(changeOutputType()));
  connect(m_radioBinary, SIGNAL(clicked()), this, SLOT(updateDefaultName()));
  connect(m_radioBinary, SIGNAL(clicked()), this, SLOT(changeOutputType()));

  connect(findButton, SIGNAL(clicked()), this, SLOT(handleFindClicked()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(handleAcceptClicked()));
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(this, SIGNAL(filePathChanged(QString)), m_lineEdit,
          SLOT(setText(QString)));

  updateDefaultName();

  // Set default width for QLineEdit
  auto textSize = m_lineEdit->fontMetrics().size(0, m_lineEdit->text());
  textSize.setWidth(textSize.width() - 10);
  QStyleOptionFrame op;
  op.initFrom(m_lineEdit);
  auto newSize =
      m_lineEdit->style()->sizeFromContents(QStyle::CT_LineEdit, &op, textSize);
  m_lineEdit->setMinimumSize(newSize);
}

void SaveOptionDialog::updateDefaultName() {
  if (!m_useDefaultName) return;
  m_defaultFileName = generateDefaultFileName();

  setFilePath(
      QDir::cleanPath(savedDirectory + QDir::separator() + m_defaultFileName));
}

const QString &SaveOptionDialog::getFilePath() const { return m_filePath; }

void SaveOptionDialog::setFilePath(const QString &newFilePath) {
  m_filePath = newFilePath;
  emit filePathChanged(m_filePath);
}

QString SaveOptionDialog::generateDefaultFileName() {
  QString prefix =
      QString(QDir(QDir::currentPath()).dirName())
          .append(m_type == Type::DisplayList ? tr("_dl") : tr("_copro"));
  prefix = QString(prefix).append(m_radioBinary->isChecked() ? tr(".bin")
                                                             : tr(".txt"));
  return prefix;
}

void SaveOptionDialog::changeOutputType() {
  outputType = m_radioBinary->isChecked() ? SaveOptionDialog::Binary
                                          : SaveOptionDialog::Text;
}

void SaveOptionDialog::changeByteOrder() {
  byteOrder = m_radioLittleEndian->isChecked() ? SaveOptionDialog::LittleEndian
                                               : SaveOptionDialog::BigEndian;
}

void SaveOptionDialog::handleFindClicked() {
  QString filePath = QFileDialog::getSaveFileName(
      this, tr("Save as"), m_filePath,
      tr(outputType == OutputType::Binary ? "Binary(*.bin)" : "Text(*.txt)"));
  if (filePath.isEmpty()) return;
  setFilePath(filePath);
  savedDirectory = QFileInfo(filePath).absolutePath();
  m_useDefaultName = false;
}

void SaveOptionDialog::handleAcceptClicked() {
  emit handleAccept(m_filePath, outputType, byteOrder, autoOpen);
  accept();
}
