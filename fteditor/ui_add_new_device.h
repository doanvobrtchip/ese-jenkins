/********************************************************************************
** Form generated from reading UI file 'add_new_device.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADD_NEW_DEVICE_H
#define UI_ADD_NEW_DEVICE_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DeviceAddNewDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTableWidget *DeviceTableWidget;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DeviceAddNewDialog)
    {
        if (DeviceAddNewDialog->objectName().isEmpty())
            DeviceAddNewDialog->setObjectName(QString::fromUtf8("DeviceAddNewDialog"));
        DeviceAddNewDialog->resize(480, 640);
        verticalLayout = new QVBoxLayout(DeviceAddNewDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        DeviceTableWidget = new QTableWidget(DeviceAddNewDialog);
        if (DeviceTableWidget->columnCount() < 2)
            DeviceTableWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        DeviceTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        DeviceTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        DeviceTableWidget->setObjectName(QString::fromUtf8("DeviceTableWidget"));
        DeviceTableWidget->setAlternatingRowColors(true);
        DeviceTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        DeviceTableWidget->horizontalHeader()->setVisible(false);
        DeviceTableWidget->horizontalHeader()->setCascadingSectionResizes(false);
        DeviceTableWidget->horizontalHeader()->setStretchLastSection(true);
        DeviceTableWidget->verticalHeader()->setVisible(false);

        verticalLayout->addWidget(DeviceTableWidget);

        buttonBox = new QDialogButtonBox(DeviceAddNewDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DeviceAddNewDialog);
        QObject::connect(buttonBox, SIGNAL(rejected()), DeviceAddNewDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(DeviceAddNewDialog);
    } // setupUi

    void retranslateUi(QDialog *DeviceAddNewDialog)
    {
        DeviceAddNewDialog->setWindowTitle(QApplication::translate("DeviceAddNewDialog", "Add New Device", nullptr));
        QTableWidgetItem *___qtablewidgetitem = DeviceTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("DeviceAddNewDialog", "New Column", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = DeviceTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("DeviceAddNewDialog", "New Column", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeviceAddNewDialog: public Ui_DeviceAddNewDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADD_NEW_DEVICE_H
