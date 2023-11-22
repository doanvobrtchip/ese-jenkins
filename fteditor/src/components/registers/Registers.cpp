/*!
 * @file RegistersComponent.cpp
 * @date 8/15/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#include "Registers.h"

#include <dl_parser.h>

#include <QAbstractSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>

#include "code_editor.h"
#include "constant_common.h"
#include "constant_mapping.h"
#include "customize/UintSpinBox.h"
#include "customize/UndoLineEdit.h"
#include "customize/UndoSpinBox.h"
#include "define/RegDefine.h"
#include "define/ValueDefine.h"
#include "device_manager.h"
#include "dl_editor.h"
#include "inspector/Inspector.h"
#include "inspector/RamReg.h"
#include "main_window.h"
#include "utils/CommonUtil.h"
#include "utils/ConvertUtil.h"
#include "utils/LoggerUtil.h"

namespace FTEDITOR {
Registers::Registers(MainWindow *parent)
    : QWidget(parent),
      m_mainWindow(parent),
      m_undoStack(m_mainWindow->undoStack()),
      m_playCtrl(),
      m_lbCurrPlayCtrl(NULL),
      m_undoRedoWorking(false),
      m_latestHSize(screenWidthDefault(FTEDITOR_CURRENT_DEVICE)),
      m_latestVSize(screenHeightDefault(FTEDITOR_CURRENT_DEVICE)),
      m_latestRotate(REG_ROTATE_DEFAULT),
      m_latestHSF(REG_HSF_HSIZE_DEFAULT),
      m_latestFreq(QString::number(REG_FREQUENCY_DEFAULT)) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  // Display Size
  {
    QGroupBox *sizeGroup = new QGroupBox;
    sizeGroup->setTitle(tr("Display Size"));
    QVBoxLayout *sizeLayout = new QVBoxLayout();

    m_hSize = new QSpinBox;
    m_hSize->setMinimum(1);
    m_hSize->setMaximum(screenWidthMaximum(FTEDITOR_CURRENT_DEVICE));
    connect(m_hSize, &QSpinBox::valueChanged, this, &Registers::onHSizeChanged);
    QHBoxLayout *hsizeLayout = new QHBoxLayout();
    QLabel *hsizeLabel = new QLabel;
    hsizeLabel->setText(tr("Horizontal"));
    hsizeLayout->addWidget(hsizeLabel);
    hsizeLayout->addWidget(m_hSize);
    sizeLayout->addLayout(hsizeLayout);

    m_vSize = new QSpinBox;
    m_vSize->setMinimum(1);
    m_vSize->setMaximum(screenHeightMaximum(FTEDITOR_CURRENT_DEVICE));
    connect(m_vSize, &QSpinBox::valueChanged, this, &Registers::onVSizeChanged);
    QHBoxLayout *vsizeLayout = new QHBoxLayout();
    QLabel *vsizeLabel = new QLabel;
    vsizeLabel->setText(tr("Vertical"));
    vsizeLayout->addWidget(vsizeLabel);
    vsizeLayout->addWidget(m_vSize);
    sizeLayout->addLayout(vsizeLayout);

    sizeGroup->setLayout(sizeLayout);
    layout->addWidget(sizeGroup);
  }

  // Rotate
  {
    QGroupBox *group = new QGroupBox;
    group->setTitle(tr("Rotate"));
    QVBoxLayout *groupLayout = new QVBoxLayout;

    m_rotate = new QSpinBox;
    m_rotate->setMinimum(REG_ROTATE_MIN);
    m_rotate->setMaximum(REG_ROTATE_MAX);
    connect(m_rotate, &QSpinBox::valueChanged, this,
            &Registers::onRotateChanged);
    auto hboxLayout = new QHBoxLayout;
    auto label = new QLabel;
    label->setText(tr("REG_ROTATE"));
    hboxLayout->addWidget(label);
    hboxLayout->addWidget(m_rotate);
    groupLayout->addLayout(hboxLayout);

    group->setLayout(groupLayout);
    layout->addWidget(group);
  }

  // Macro
  {
    QGroupBox *macroGroup = new QGroupBox;
    macroGroup->setTitle(tr("Macro"));
    QVBoxLayout *macroLayout = new QVBoxLayout();
    macroLayout->setContentsMargins(0, 0, 0, 0);

    m_macro = new DlEditor(m_mainWindow);
    m_macro->setPropertiesEditor(m_mainWindow->propertiesEditor());
    m_macro->setUndoStack(m_undoStack);
    m_macro->setModeMacro();
    macroLayout->addWidget(m_macro);

    macroGroup->setLayout(macroLayout);
    macroGroup->setMaximumHeight(80);
    layout->addWidget(macroGroup);

    connect(m_macro->codeEditor(), &CodeEditor::textChanged, this,
            [this]() { emit contentChanged(); });
  }

  // Control
  {
    QGroupBox *ctrlGroup = new QGroupBox;
    ctrlGroup->setTitle(tr("Control"));
    QVBoxLayout *ctrlLayout = new QVBoxLayout;
    QLabel *lbReqPlayCtrl = new QLabel;
    lbReqPlayCtrl->setText(tr("REG_PLAY_CONTROL"));
    lbReqPlayCtrl->setMinimumWidth(30);
    m_lbCurrPlayCtrl =
        new QLabel(QString("<b>(%1)</b>")
                       .arg(ConvertUtil::uintToHex(REG_PLAY_CONTROL_EXIT, 2)));

    QVBoxLayout *vBoxStrPlayTrl = new QVBoxLayout;
    vBoxStrPlayTrl->setSpacing(0);
    vBoxStrPlayTrl->setAlignment(Qt::AlignVCenter);
    vBoxStrPlayTrl->addWidget(lbReqPlayCtrl);
    vBoxStrPlayTrl->addWidget(m_lbCurrPlayCtrl);

    QHBoxLayout *hBoxExit = new QHBoxLayout;
    hBoxExit->setSpacing(2);
    QLabel *lbExit = new QLabel(tr("Exit:"));
    QPushButton *btnExit =
        new QPushButton(ConvertUtil::uintToHex(REG_PLAY_CONTROL_EXIT, 2));
    hBoxExit->setAlignment(Qt::AlignRight);
    hBoxExit->addWidget(lbExit);
    hBoxExit->addWidget(btnExit);

    QHBoxLayout *hBoxPause = new QHBoxLayout;
    hBoxPause->setSpacing(2);
    QLabel *lbPause = new QLabel(tr("Pause:"));
    QPushButton *btnPause =
        new QPushButton(ConvertUtil::uintToHex(REG_PLAY_CONTROL_PAUSE, 2));
    hBoxPause->setAlignment(Qt::AlignRight);
    hBoxPause->addWidget(lbPause);
    hBoxPause->addWidget(btnPause);

    QHBoxLayout *hBoxPlay = new QHBoxLayout;
    hBoxPlay->setSpacing(2);
    QLabel *lbPlay = new QLabel(tr("Play:"));
    QPushButton *btnPlay =
        new QPushButton(ConvertUtil::uintToHex(REG_PLAY_CONTROL_PLAY, 2));
    hBoxPlay->setAlignment(Qt::AlignRight);
    hBoxPlay->addWidget(lbPlay);
    hBoxPlay->addWidget(btnPlay);

    QVBoxLayout *vBoxReqPlayCtrl = new QVBoxLayout;
    vBoxReqPlayCtrl->addLayout(hBoxExit);
    vBoxReqPlayCtrl->addLayout(hBoxPause);
    vBoxReqPlayCtrl->addLayout(hBoxPlay);

    QHBoxLayout *hBoxReqPlayCtrl = new QHBoxLayout;
    hBoxReqPlayCtrl->addLayout(vBoxStrPlayTrl);
    hBoxReqPlayCtrl->addLayout(vBoxReqPlayCtrl);

    ctrlLayout->addLayout(hBoxReqPlayCtrl);
    ctrlGroup->setLayout(ctrlLayout);
    layout->addWidget(ctrlGroup);

    connect(btnExit, &QPushButton::clicked, this,
            [this]() { setPlayCtrl(REG_PLAY_CONTROL_EXIT); });
    connect(btnPause, &QPushButton::clicked, this,
            [this]() { setPlayCtrl(REG_PLAY_CONTROL_PAUSE); });
    connect(btnPlay, &QPushButton::clicked, this,
            [this]() { setPlayCtrl(REG_PLAY_CONTROL_PLAY); });

    connect(m_mainWindow, &MainWindow::deviceChanged, this, [ctrlGroup]() {
      ctrlGroup->setVisible(FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815);
    });
  }

  // HSF
  {
    auto hsfGroup = new QGroupBox;
    hsfGroup->setTitle(tr("HSF"));
    hsfGroup->setToolTip(tr("Set by CMD_HSF"));

    auto hsfLabel = new QLabel;
    hsfLabel->setText(tr("Width of HSF"));
    m_hsf = new QSpinBox;
    m_hsf->setMinimum(0);
    m_hsf->setValue(REG_HSF_HSIZE_DEFAULT);

    auto hsfLayout = new QHBoxLayout;
    hsfLayout->addWidget(hsfLabel);
    hsfLayout->addWidget(m_hsf);

    hsfGroup->setLayout(hsfLayout);
    layout->addWidget(hsfGroup);

    connect(m_hSize, &QSpinBox::valueChanged, this, [this](int i) {
      if (i < m_hsf->value()) m_hsf->setValue(0);
      m_hsf->setMaximum(i);
    });
    connect(m_hsf, &QSpinBox::valueChanged, this, &Registers::onHSFChanged);
    connect(m_mainWindow, &MainWindow::deviceChanged, this, [hsfGroup, this]() {
      bool visible = FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT817;
      hsfGroup->setVisible(visible);
      if (visible) m_hsf->setValue(0);
    });
  }

  // Frequency
  {
    auto freqGroup = new QGroupBox;
    freqGroup->setTitle(tr("Frequency"));

    auto freqLabel = new QLabel;
    freqLabel->setText(tr("REG_FREQUENCY"));
    m_freq = new UintSpinBox(REG_FREQUENCY_DEFAULT);

    auto freqLayout = new QHBoxLayout;
    freqLayout->addWidget(freqLabel);
    freqLayout->addWidget(m_freq);

    freqGroup->setLayout(freqLayout);
    layout->addWidget(freqGroup);

    connect(m_freq, &UintSpinBox::changeFinished, this,
            &Registers::onFreqChanged);
    connect(m_freq, &UintSpinBox::hasAcceptableValue, this,
            [this](uint32_t value) { emit frequencyChanged(value); });
  }

  layout->addStretch();

  connect(m_mainWindow, &MainWindow::clearEvent, this,
          &Registers::onClearEvent);
  connect(m_mainWindow, &MainWindow::displaySizeChanged, this,
          &Registers::onDisplaySizeChanged);

  ComponentBase::finishedSetup(this, m_mainWindow);
}

void Registers::setupConnections(QObject *obj) {
  if (auto dm = m_mainWindow->deviceManager();
      dm && (obj == dm || obj == nullptr)) {
    connect(dm, &DeviceManager::displaySizeChanged, this,
            &Registers::onDisplaySizeChanged);
  }

  if (auto insp = m_mainWindow->inspector();
      insp && (obj == insp || obj == nullptr)) {
    connect(insp->ramReg(), &RamReg::regPlayControlChanged, this,
            [this](uint32_t value) {
              m_lbCurrPlayCtrl->setText(
                  QString("<b>(%1)</b>").arg(ConvertUtil::uintToHex(value, 2)));
            });
    connect(insp->ramReg(), &RamReg::regFrequencyChanged, this,
            [this](uint32_t value) { m_freq->setText(value); });
  }
}

void Registers::setPlayCtrl(int newPlayCtrl) {
  m_playCtrl = newPlayCtrl;
  emit playCtrlChanged(newPlayCtrl);
}

void Registers::onHSizeChanged(int newValue) {
  emit contentChanged();
  emit hSizeChanged(newValue);
  if (m_undoRedoWorking) return;
  m_undoStack->push(
      new UndoSpinBox(41517686, m_hSize, m_latestHSize, &m_undoRedoWorking));
  m_latestHSize = newValue;
}

void Registers::onVSizeChanged(int newValue) {
  emit contentChanged();
  emit vSizeChanged(newValue);
  if (m_undoRedoWorking) return;
  m_undoStack->push(
      new UndoSpinBox(78984351, m_vSize, m_latestVSize, &m_undoRedoWorking));
  m_latestVSize = newValue;
}

void Registers::onRotateChanged(int newValue) {
  emit contentChanged();
  emit rotateChanged(newValue);
  if (m_undoRedoWorking) return;
  m_undoStack->push(
      new UndoSpinBox(78994352, m_rotate, m_latestRotate, &m_undoRedoWorking));
  m_latestRotate = newValue;
}

void Registers::onHSFChanged(int newValue) {
  emit contentChanged();
  if (m_undoRedoWorking) return;
  m_undoStack->push(
      new UndoSpinBox(78994353, m_hsf, m_latestHSF, &m_undoRedoWorking));
  m_latestHSF = newValue;

  // Inject CMD_HSF in the current list
  DlParsed pa;
  pa.ValidId = true;
  pa.IdLeft = 0xFFFFFF00;
  pa.IdRight = CMD_HSF & 0xFF;
  pa.ExpectedStringParameter = false;
  pa.VarArgCount = 0;
  pa.Parameter[0].I = newValue;
  pa.ExpectedParameterCount = 1;

  auto cmdEditor = m_mainWindow->cmdEditor();
  int index = cmdEditor->codeEditor()->blockCount();
  cmdEditor->setDLSharedItem(DlParser::compile(FTEDITOR_CURRENT_DEVICE, pa),
                             index);
  cmdEditor->setDLParsedItem(pa, index);
  cmdEditor->setDisplayListModified(true);
}

void Registers::onFreqChanged(const QString &newValue) {
  emit contentChanged();
  if (m_undoRedoWorking) return;
  m_undoStack->push(new UndoLineEdit(78994354, m_freq->getLineEdit(),
                                     m_latestFreq, &m_undoRedoWorking));
  m_latestFreq = newValue;
}

void Registers::onDisplaySizeChanged(int hSize, int vSize) {
  if (m_hSize->value() == hSize && m_vSize->value() == vSize) return;
  m_undoStack->beginMacro("Change resolution");
  setHSize(hSize);
  setVSize(vSize);
  m_undoStack->endMacro();
}

int Registers::latestHSF() const { return m_latestHSF; }

QSpinBox *Registers::rotate() const { return m_rotate; }

QSpinBox *Registers::hSize() const { return m_hSize; }

QSpinBox *Registers::vSize() const { return m_vSize; }

void Registers::setHSize(int newValue) {
  if (m_hSize->value() == newValue) return;
  m_hSize->setValue(newValue);
}

void Registers::setVSize(int newValue) {
  if (m_vSize->value() == newValue) return;
  m_vSize->setValue(newValue);
}

void Registers::setRotate(int newValue) {
  if (m_rotate->value() == newValue) return;
  m_rotate->setValue(newValue);
}

void Registers::setHSF(int newValue) {
  if (m_hsf->value() == newValue) return;
  m_hsf->setValue(newValue);
}

void Registers::onClearEvent() {
  m_hSize->setValue(screenWidthDefault(FTEDITOR_CURRENT_DEVICE));
  m_vSize->setValue(screenHeightDefault(FTEDITOR_CURRENT_DEVICE));
  m_rotate->setValue(REG_ROTATE_DEFAULT);
  m_hsf->setValue(REG_HSF_HSIZE_DEFAULT);
  m_freq->setText(REG_FREQUENCY_DEFAULT);
  m_macro->clear();

  // Reset Macro
  {
    DlParsed pa;
    pa.ValidId = true;
    pa.IdLeft = 0;
    pa.IdRight = FTEDITOR_DL_NOP;
    pa.ExpectedStringParameter = false;
    pa.VarArgCount = 0;
    m_macro->replaceLine(0, pa);
    m_macro->replaceLine(1, pa);
  }
}

DlEditor *Registers::macro() const { return m_macro; }

QJsonObject Registers::toJson(bool exportScript) {
  QJsonObject reg;
  reg[REG_HSIZE_KEY] = m_hSize->value();
  reg[REG_VSIZE_KEY] = m_vSize->value();
  reg[REG_ROTATE_KEY] = m_rotate->value();
  reg[REG_HSF_KEY] = m_hsf->value();
  reg[REG_MACRO_KEY] = CommonUtil::documentToJsonArray(
      m_macro->codeEditor()->document(), false, exportScript);
  return reg;
}

void Registers::fromJson(QJsonObject obj) {
  auto reg = obj[REG_KEY].toObject();
  m_hSize->setValue(reg[REG_HSIZE_KEY].toVariant().toInt());
  m_vSize->setValue(reg[REG_VSIZE_KEY].toVariant().toInt());
  m_rotate->setValue(reg.contains(REG_ROTATE_KEY)
                         ? reg[REG_ROTATE_KEY].toVariant().toInt()
                         : REG_ROTATE_DEFAULT);
  m_hsf->setValue(reg.contains(REG_HSF_KEY)
                      ? reg[REG_HSF_KEY].toVariant().toInt()
                      : REG_HSF_HSIZE_DEFAULT);
  m_macro->codeEditor()->clear();
  CommonUtil::documentFromJsonArray(m_macro->codeEditor(),
                                    reg[REG_MACRO_KEY].toArray());
}

}  // namespace FTEDITOR
