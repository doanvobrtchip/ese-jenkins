/********************************************************************************
** Form generated from reading UI file 'device_manage.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVICE_MANAGE_H
#define UI_DEVICE_MANAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DeviceManageDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QListWidget *deviceListWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *btnAddNewDevice;
    QPushButton *btnEditDevice;
    QPushButton *btnRemoveDevice;
    QSpacerItem *verticalSpacer;
    QPushButton *btnClose;

    void setupUi(QDialog *DeviceManageDialog)
    {
        if (DeviceManageDialog->objectName().isEmpty())
            DeviceManageDialog->setObjectName(QString::fromUtf8("DeviceManageDialog"));
        DeviceManageDialog->resize(378, 290);
        verticalLayout_2 = new QVBoxLayout(DeviceManageDialog);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        deviceListWidget = new QListWidget(DeviceManageDialog);
        deviceListWidget->setObjectName(QString::fromUtf8("deviceListWidget"));

        horizontalLayout->addWidget(deviceListWidget);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        btnAddNewDevice = new QPushButton(DeviceManageDialog);
        btnAddNewDevice->setObjectName(QString::fromUtf8("btnAddNewDevice"));
        btnAddNewDevice->setMinimumSize(QSize(100, 0));

        verticalLayout->addWidget(btnAddNewDevice);

        btnEditDevice = new QPushButton(DeviceManageDialog);
        btnEditDevice->setObjectName(QString::fromUtf8("btnEditDevice"));

        verticalLayout->addWidget(btnEditDevice);

        btnRemoveDevice = new QPushButton(DeviceManageDialog);
        btnRemoveDevice->setObjectName(QString::fromUtf8("btnRemoveDevice"));

        verticalLayout->addWidget(btnRemoveDevice);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        btnClose = new QPushButton(DeviceManageDialog);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));

        verticalLayout->addWidget(btnClose);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(DeviceManageDialog);

        QMetaObject::connectSlotsByName(DeviceManageDialog);
    } // setupUi

    void retranslateUi(QDialog *DeviceManageDialog)
    {
        DeviceManageDialog->setWindowTitle(QApplication::translate("DeviceManageDialog", "Manage Device", nullptr));
        btnAddNewDevice->setText(QApplication::translate("DeviceManageDialog", "Add New Device", nullptr));
        btnEditDevice->setText(QApplication::translate("DeviceManageDialog", "Edit Device", nullptr));
        btnRemoveDevice->setText(QApplication::translate("DeviceManageDialog", "Remove Device", nullptr));
        btnClose->setText(QApplication::translate("DeviceManageDialog", "Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeviceManageDialog: public Ui_DeviceManageDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVICE_MANAGE_H
