/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QFrame *sidebar;
    QVBoxLayout *verticalLayout_sidebar;
    QLabel *logoLabel;
    QPushButton *btn_accueil;
    QPushButton *btn_emploi;
    QPushButton *btn_zones;
    QPushButton *btn_interventions;
    QPushButton *btn_equipements;
    QPushButton *btn_fournisseurs;
    QSpacerItem *verticalSpacer;
    QWidget *rightContainer;
    QVBoxLayout *verticalLayout_right;
    QFrame *header;
    QHBoxLayout *horizontalLayout_header;
    QLabel *lbl_weather_icon;
    QLabel *lbl_weather_temp;
    QLabel *lbl_humidity;
    QLabel *lbl_wind;
    QLabel *lbl_precip;
    QSpacerItem *horizontalSpacer;
    QWidget *clockContainer;
    QVBoxLayout *verticalLayout_clock;
    QLabel *lbl_date;
    QLabel *lbl_time;
    QLabel *lbl_notifications;
    QLabel *lbl_user_info;
    QStackedWidget *m_stack;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1440, 880);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        sidebar = new QFrame(centralwidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(230, 0));
        sidebar->setMaximumSize(QSize(230, 16777215));
        verticalLayout_sidebar = new QVBoxLayout(sidebar);
        verticalLayout_sidebar->setSpacing(6);
        verticalLayout_sidebar->setObjectName("verticalLayout_sidebar");
        verticalLayout_sidebar->setContentsMargins(14, 18, 14, 18);
        logoLabel = new QLabel(sidebar);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_sidebar->addWidget(logoLabel);

        btn_accueil = new QPushButton(sidebar);
        btn_accueil->setObjectName("btn_accueil");
        btn_accueil->setCheckable(true);

        verticalLayout_sidebar->addWidget(btn_accueil);

        btn_emploi = new QPushButton(sidebar);
        btn_emploi->setObjectName("btn_emploi");
        btn_emploi->setCheckable(true);

        verticalLayout_sidebar->addWidget(btn_emploi);

        btn_zones = new QPushButton(sidebar);
        btn_zones->setObjectName("btn_zones");
        btn_zones->setCheckable(true);

        verticalLayout_sidebar->addWidget(btn_zones);

        btn_interventions = new QPushButton(sidebar);
        btn_interventions->setObjectName("btn_interventions");
        btn_interventions->setCheckable(true);

        verticalLayout_sidebar->addWidget(btn_interventions);

        btn_equipements = new QPushButton(sidebar);
        btn_equipements->setObjectName("btn_equipements");
        btn_equipements->setCheckable(true);

        verticalLayout_sidebar->addWidget(btn_equipements);

        btn_fournisseurs = new QPushButton(sidebar);
        btn_fournisseurs->setObjectName("btn_fournisseurs");
        btn_fournisseurs->setCheckable(true);

        verticalLayout_sidebar->addWidget(btn_fournisseurs);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_sidebar->addItem(verticalSpacer);


        horizontalLayout->addWidget(sidebar);

        rightContainer = new QWidget(centralwidget);
        rightContainer->setObjectName("rightContainer");
        verticalLayout_right = new QVBoxLayout(rightContainer);
        verticalLayout_right->setSpacing(14);
        verticalLayout_right->setObjectName("verticalLayout_right");
        verticalLayout_right->setContentsMargins(18, 16, 18, 16);
        header = new QFrame(rightContainer);
        header->setObjectName("header");
        horizontalLayout_header = new QHBoxLayout(header);
        horizontalLayout_header->setSpacing(26);
        horizontalLayout_header->setObjectName("horizontalLayout_header");
        horizontalLayout_header->setContentsMargins(20, 10, 20, 10);
        lbl_weather_icon = new QLabel(header);
        lbl_weather_icon->setObjectName("lbl_weather_icon");

        horizontalLayout_header->addWidget(lbl_weather_icon);

        lbl_weather_temp = new QLabel(header);
        lbl_weather_temp->setObjectName("lbl_weather_temp");

        horizontalLayout_header->addWidget(lbl_weather_temp);

        lbl_humidity = new QLabel(header);
        lbl_humidity->setObjectName("lbl_humidity");

        horizontalLayout_header->addWidget(lbl_humidity);

        lbl_wind = new QLabel(header);
        lbl_wind->setObjectName("lbl_wind");

        horizontalLayout_header->addWidget(lbl_wind);

        lbl_precip = new QLabel(header);
        lbl_precip->setObjectName("lbl_precip");

        horizontalLayout_header->addWidget(lbl_precip);

        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_header->addItem(horizontalSpacer);

        clockContainer = new QWidget(header);
        clockContainer->setObjectName("clockContainer");
        verticalLayout_clock = new QVBoxLayout(clockContainer);
        verticalLayout_clock->setSpacing(0);
        verticalLayout_clock->setObjectName("verticalLayout_clock");
        verticalLayout_clock->setContentsMargins(0, 0, 0, 0);
        lbl_date = new QLabel(clockContainer);
        lbl_date->setObjectName("lbl_date");

        verticalLayout_clock->addWidget(lbl_date);

        lbl_time = new QLabel(clockContainer);
        lbl_time->setObjectName("lbl_time");

        verticalLayout_clock->addWidget(lbl_time);


        horizontalLayout_header->addWidget(clockContainer);

        lbl_notifications = new QLabel(header);
        lbl_notifications->setObjectName("lbl_notifications");

        horizontalLayout_header->addWidget(lbl_notifications);

        lbl_user_info = new QLabel(header);
        lbl_user_info->setObjectName("lbl_user_info");

        horizontalLayout_header->addWidget(lbl_user_info);


        verticalLayout_right->addWidget(header);

        m_stack = new QStackedWidget(rightContainer);
        m_stack->setObjectName("m_stack");
        m_stack->setGeometry(QRect(0, 0, 1210, 800));

        verticalLayout_right->addWidget(m_stack);


        horizontalLayout->addWidget(rightContainer);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Smart Weather Management \342\200\224 Application Int\303\251gr\303\251e", nullptr));
        logoLabel->setText(QCoreApplication::translate("MainWindow", "Logo", nullptr));
        btn_accueil->setText(QCoreApplication::translate("MainWindow", "\360\237\217\240   Accueil", nullptr));
        btn_emploi->setText(QCoreApplication::translate("MainWindow", "\360\237\223\205   Gestion d'emploi", nullptr));
        btn_zones->setText(QCoreApplication::translate("MainWindow", "\360\237\223\241   Stations & Zones", nullptr));
        btn_interventions->setText(QCoreApplication::translate("MainWindow", "\342\232\240   Interventions", nullptr));
        btn_equipements->setText(QCoreApplication::translate("MainWindow", "\360\237\247\260   \303\211quipements", nullptr));
        btn_fournisseurs->setText(QCoreApplication::translate("MainWindow", "\360\237\223\246   Fournisseurs", nullptr));
        lbl_weather_icon->setText(QCoreApplication::translate("MainWindow", "\342\233\205", nullptr));
        lbl_weather_temp->setText(QCoreApplication::translate("MainWindow", "<b style='font-size:20px'>22\302\260C</b><br><span style='color:#8fb3c2'>Partiellement nuageux \342\200\224 Tunis</span>", nullptr));
        lbl_humidity->setText(QCoreApplication::translate("MainWindow", "\360\237\222\247 Humidit\303\251<br><b>68 %</b>", nullptr));
        lbl_wind->setText(QCoreApplication::translate("MainWindow", "\360\237\214\254 Vent<br><b>18 km/h</b>", nullptr));
        lbl_precip->setText(QCoreApplication::translate("MainWindow", "\360\237\214\247 Pr\303\251cipitations<br><b>0 mm</b>", nullptr));
        lbl_date->setText(QCoreApplication::translate("MainWindow", "date", nullptr));
        lbl_time->setText(QCoreApplication::translate("MainWindow", "time", nullptr));
        lbl_notifications->setText(QCoreApplication::translate("MainWindow", "\360\237\224\224", nullptr));
        lbl_user_info->setText(QCoreApplication::translate("MainWindow", "<b>\303\211quipe SmartWeather</b><br><span style='color:#19c6b7; font-size:11px'>Adem \342\200\242 Aweb \342\200\242 Bader \342\200\242 Yasmine</span>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
