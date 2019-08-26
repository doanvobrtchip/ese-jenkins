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
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DeviceManageDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_3;
    QListWidget *deviceListWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *btnAddNewDevice;
    QPushButton *btnEditDevice;
    QPushButton *btnCloneDevice;
    QPushButton *btnRemoveDevice;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_2;
    QListWidget *BuildInDeviceListWidget;
    QVBoxLayout *verticalLayout_2;
    QPushButton *btnExamineButton;
    QPushButton *btnBuildInCloneDevice;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnClose;

    void setupUi(QDialog *DeviceManageDialog)
    {
        if (DeviceManageDialog->objectName().isEmpty())
            DeviceManageDialog->setObjectName(QString::fromUtf8("DeviceManageDialog"));
        DeviceManageDialog->resize(525, 580);
        verticalLayout_3 = new QVBoxLayout(DeviceManageDialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox = new QGroupBox(DeviceManageDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout_3 = new QHBoxLayout(groupBox);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        deviceListWidget = new QListWidget(groupBox);
        deviceListWidget->setObjectName(QString::fromUtf8("deviceListWidget"));

        horizontalLayout_3->addWidget(deviceListWidget);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        btnAddNewDevice = new QPushButton(groupBox);
        btnAddNewDevice->setObjectName(QString::fromUtf8("btnAddNewDevice"));
        btnAddNewDevice->setMinimumSize(QSize(100, 0));

        verticalLayout->addWidget(btnAddNewDevice);

        btnEditDevice = new QPushButton(groupBox);
        btnEditDevice->setObjectName(QString::fromUtf8("btnEditDevice"));

        verticalLayout->addWidget(btnEditDevice);

        btnCloneDevice = new QPushButton(groupBox);
        btnCloneDevice->setObjectName(QString::fromUtf8("btnCloneDevice"));

        verticalLayout->addWidget(btnCloneDevice);

        btnRemoveDevice = new QPushButton(groupBox);
        btnRemoveDevice->setObjectName(QString::fromUtf8("btnRemoveDevice"));

        verticalLayout->addWidget(btnRemoveDevice);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout_3->addLayout(verticalLayout);


        verticalLayout_3->addWidget(groupBox);

        groupBox_2 = new QGroupBox(DeviceManageDialog);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        BuildInDeviceListWidget = new QListWidget(groupBox_2);
        BuildInDeviceListWidget->setObjectName(QString::fromUtf8("BuildInDeviceListWidget"));

        horizontalLayout_2->addWidget(BuildInDeviceListWidget);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        btnExamineButton = new QPushButton(groupBox_2);
        btnExamineButton->setObjectName(QString::fromUtf8("btnExamineButton"));
        btnExamineButton->setMinimumSize(QSize(100, 0));

        verticalLayout_2->addWidget(btnExamineButton);

        btnBuildInCloneDevice = new QPushButton(groupBox_2);
        btnBuildInCloneDevice->setObjectName(QString::fromUtf8("btnBuildInCloneDevice"));

        verticalLayout_2->addWidget(btnBuildInCloneDevice);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);


        horizontalLayout_2->addLayout(verticalLayout_2);


        verticalLayout_3->addWidget(groupBox_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        btnClose = new QPushButton(DeviceManageDialog);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));

        horizontalLayout->addWidget(btnClose);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(DeviceManageDialog);

        QMetaObject::connectSlotsByName(DeviceManageDialog);
    } // setupUi

    void retranslateUi(QDialog *DeviceManageDialog)
    {
        DeviceManageDialog->setWindowTitle(QApplication::translate("DeviceManageDialog", "Manage Device", nullptr));
        groupBox->setTitle(QApplication::translate("DeviceManageDialog", "Custom Device", nullptr));
        btnAddNewDevice->setText(QApplication::translate("DeviceManageDialog", "Add New Device", nullptr));
        btnEditDevice->setText(QApplication::translate("DeviceManageDialog", "Edit Device", nullptr));
        btnCloneDevice->setText(QApplication::translate("DeviceManageDialog", "Clone Device", nullptr));
        btnRemoveDevice->setText(QApplication::translate("DeviceManageDialog", "Remove Device", nullptr));
        groupBox_2->setTitle(QApplication::translate("DeviceManageDialog", "Built-in Device", nullptr));
        btnExamineButton->setText(QApplication::translate("DeviceManageDialog", "Examine Device", nullptr));
        btnBuildInCloneDevice->setText(QApplication::translate("DeviceManageDialog", "Clone Device", nullptr));
        btnClose->setText(QApplication::translate("DeviceManageDialog", "Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeviceManageDialog: public Ui_DeviceManageDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVICE_MANAGE_H
