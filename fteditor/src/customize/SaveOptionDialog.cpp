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
  type_ = type;
  setWindowTitle(type_ == SaveOptionDialog::DisplayList
                     ? tr("Save Display List")
                     : tr("Save Coprocessor Command"));
  useDefaultName_ = true;
  auto label = new QLabel(tr("Save as:"));
  lineEdit_ = new QLineEdit;
  label->setBuddy(lineEdit_);
  auto findButton = new QPushButton(tr("..."));
  findButton->setAutoDefault(false);
  QHBoxLayout *dirHBoxLayout = new QHBoxLayout;
  dirHBoxLayout->addWidget(label);
  dirHBoxLayout->addWidget(lineEdit_);
  dirHBoxLayout->addWidget(findButton);

  // Notation group
  radioBinary_ = new QRadioButton;
  radioBinary_->setText(tr("Binary"));
  radioString_ = new QRadioButton;
  radioString_->setText(tr("Hexadecimal String"));
  auto notationHBoxLayout = new QHBoxLayout;
  notationHBoxLayout->addWidget(radioBinary_);
  notationHBoxLayout->addWidget(radioString_);
  auto notationGroupBox = new QGroupBox(this);
  notationGroupBox->setTitle(tr("Notation"));
  notationGroupBox->setLayout(notationHBoxLayout);

  // Byte order group
  radioLittleEndian_ = new QRadioButton;
  radioLittleEndian_->setText(tr("Little Endian"));
  radioBigEndian_ = new QRadioButton;
  radioBigEndian_->setText(tr("Big Endian"));
  auto byteOrderHBoxLayout = new QHBoxLayout;
  byteOrderHBoxLayout->addWidget(radioLittleEndian_);
  byteOrderHBoxLayout->addWidget(radioBigEndian_);
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
  outputType == SaveOptionDialog::Binary ? radioBinary_->setChecked(true)
                                         : radioString_->setChecked(true);
  byteOrder == SaveOptionDialog::BigEndian
      ? radioBigEndian_->setChecked(true)
      : radioLittleEndian_->setChecked(true);
  autoOpenCheckBox->setChecked(autoOpen);

  connect(autoOpenCheckBox, &QAbstractButton::clicked, this,
          [](bool checked) { autoOpen = checked; });
  connect(radioLittleEndian_, SIGNAL(clicked()), this,
          SLOT(updateDefaultName()));
  connect(radioLittleEndian_, SIGNAL(clicked()), this, SLOT(changeByteOrder()));
  connect(radioBigEndian_, SIGNAL(clicked()), this, SLOT(updateDefaultName()));
  connect(radioBigEndian_, SIGNAL(clicked()), this, SLOT(changeByteOrder()));
  connect(radioString_, SIGNAL(clicked()), this, SLOT(updateDefaultName()));
  connect(radioString_, SIGNAL(clicked()), this, SLOT(changeOutputType()));
  connect(radioBinary_, SIGNAL(clicked()), this, SLOT(updateDefaultName()));
  connect(radioBinary_, SIGNAL(clicked()), this, SLOT(changeOutputType()));

  connect(findButton, SIGNAL(clicked()), this, SLOT(handleFindClicked()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(handleAcceptClicked()));
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(this, SIGNAL(filePathChanged(QString)), lineEdit_,
          SLOT(setText(QString)));

  updateDefaultName();

  // Set default width for QLineEdit
  auto textSize = lineEdit_->fontMetrics().size(0, lineEdit_->text());
  textSize.setWidth(textSize.width() - 10);
  QStyleOptionFrame op;
  op.initFrom(lineEdit_);
  auto newSize =
      lineEdit_->style()->sizeFromContents(QStyle::CT_LineEdit, &op, textSize);
  lineEdit_->setMinimumSize(newSize);
}

void SaveOptionDialog::updateDefaultName() {
  if (!useDefaultName_) return;
  defaultFileName_ = generateDefaultFileName();

  setFilePath(
      QDir::cleanPath(savedDirectory + QDir::separator() + defaultFileName_));
}

const QString &SaveOptionDialog::getFilePath() const { return filePath_; }

void SaveOptionDialog::setFilePath(const QString &newFilePath) {
  filePath_ = newFilePath;
  emit filePathChanged(filePath_);
}

QString SaveOptionDialog::generateDefaultFileName() {
  QString prefix =
      QString(QDir(QDir::currentPath()).dirName())
          .append(type_ == Type::DisplayList ? tr("_dl")
                                             : tr("_copro"));
  prefix = QString(prefix).append(radioBinary_->isChecked() ? tr(".bin")
                                                            : tr(".txt"));
  return prefix;
}

void SaveOptionDialog::changeOutputType() {
  outputType = radioBinary_->isChecked() ? SaveOptionDialog::Binary
                                         : SaveOptionDialog::Text;
}

void SaveOptionDialog::changeByteOrder() {
  byteOrder = radioLittleEndian_->isChecked() ? SaveOptionDialog::LittleEndian
                                              : SaveOptionDialog::BigEndian;
}

void SaveOptionDialog::handleFindClicked() {
  QString filePath = QFileDialog::getSaveFileName(
      this, tr("Save as"), filePath_,
      tr(outputType == OutputType::Binary ? "Binary(*.bin)" : "Text(*.txt)"));
  if (filePath.isEmpty()) return;
  setFilePath(filePath);
  savedDirectory = QFileInfo(filePath).absolutePath();
  useDefaultName_ = false;
}

void SaveOptionDialog::handleAcceptClicked() {
  emit handleAccept(filePath_, outputType, byteOrder, autoOpen);
  accept();
}
