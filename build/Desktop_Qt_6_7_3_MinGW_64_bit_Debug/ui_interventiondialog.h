/********************************************************************************
** Form generated from reading UI file 'interventiondialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INTERVENTIONDIALOG_H
#define UI_INTERVENTIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_InterventionDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelTitre;
    QFormLayout *formLayout;
    QLabel *lblType;
    QComboBox *comboType;
    QLabel *lblZone;
    QComboBox *comboZone;
    QLabel *lblPriorite;
    QComboBox *comboPriorite;
    QLabel *lblStatut;
    QComboBox *comboStatut;
    QLabel *lblDebut;
    QDateTimeEdit *dtDebut;
    QCheckBox *checkFin;
    QDateTimeEdit *dtFin;
    QLabel *lblResp;
    QLineEdit *editResponsable;
    QLabel *lblEmploi;
    QComboBox *comboEmploi;
    QLabel *lblDesc;
    QPlainTextEdit *editDescription;
    QLabel *lblCR;
    QPlainTextEdit *editCompteRendu;
    QLabel *labelAide;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *InterventionDialog)
    {
        if (InterventionDialog->objectName().isEmpty())
            InterventionDialog->setObjectName("InterventionDialog");
        InterventionDialog->resize(620, 640);
        verticalLayout = new QVBoxLayout(InterventionDialog);
        verticalLayout->setSpacing(12);
        verticalLayout->setObjectName("verticalLayout");
        labelTitre = new QLabel(InterventionDialog);
        labelTitre->setObjectName("labelTitre");
        labelTitre->setProperty("class", QVariant(QString::fromUtf8("sectionTitle")));

        verticalLayout->addWidget(labelTitre);

        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        formLayout->setHorizontalSpacing(14);
        formLayout->setVerticalSpacing(10);
        lblType = new QLabel(InterventionDialog);
        lblType->setObjectName("lblType");

        formLayout->setWidget(0, QFormLayout::LabelRole, lblType);

        comboType = new QComboBox(InterventionDialog);
        comboType->setObjectName("comboType");

        formLayout->setWidget(0, QFormLayout::FieldRole, comboType);

        lblZone = new QLabel(InterventionDialog);
        lblZone->setObjectName("lblZone");

        formLayout->setWidget(1, QFormLayout::LabelRole, lblZone);

        comboZone = new QComboBox(InterventionDialog);
        comboZone->setObjectName("comboZone");

        formLayout->setWidget(1, QFormLayout::FieldRole, comboZone);

        lblPriorite = new QLabel(InterventionDialog);
        lblPriorite->setObjectName("lblPriorite");

        formLayout->setWidget(2, QFormLayout::LabelRole, lblPriorite);

        comboPriorite = new QComboBox(InterventionDialog);
        comboPriorite->setObjectName("comboPriorite");

        formLayout->setWidget(2, QFormLayout::FieldRole, comboPriorite);

        lblStatut = new QLabel(InterventionDialog);
        lblStatut->setObjectName("lblStatut");

        formLayout->setWidget(3, QFormLayout::LabelRole, lblStatut);

        comboStatut = new QComboBox(InterventionDialog);
        comboStatut->setObjectName("comboStatut");

        formLayout->setWidget(3, QFormLayout::FieldRole, comboStatut);

        lblDebut = new QLabel(InterventionDialog);
        lblDebut->setObjectName("lblDebut");

        formLayout->setWidget(4, QFormLayout::LabelRole, lblDebut);

        dtDebut = new QDateTimeEdit(InterventionDialog);
        dtDebut->setObjectName("dtDebut");
        dtDebut->setCalendarPopup(true);

        formLayout->setWidget(4, QFormLayout::FieldRole, dtDebut);

        checkFin = new QCheckBox(InterventionDialog);
        checkFin->setObjectName("checkFin");

        formLayout->setWidget(5, QFormLayout::LabelRole, checkFin);

        dtFin = new QDateTimeEdit(InterventionDialog);
        dtFin->setObjectName("dtFin");
        dtFin->setCalendarPopup(true);

        formLayout->setWidget(5, QFormLayout::FieldRole, dtFin);

        lblResp = new QLabel(InterventionDialog);
        lblResp->setObjectName("lblResp");

        formLayout->setWidget(6, QFormLayout::LabelRole, lblResp);

        editResponsable = new QLineEdit(InterventionDialog);
        editResponsable->setObjectName("editResponsable");

        formLayout->setWidget(6, QFormLayout::FieldRole, editResponsable);

        lblEmploi = new QLabel(InterventionDialog);
        lblEmploi->setObjectName("lblEmploi");

        formLayout->setWidget(7, QFormLayout::LabelRole, lblEmploi);

        comboEmploi = new QComboBox(InterventionDialog);
        comboEmploi->setObjectName("comboEmploi");

        formLayout->setWidget(7, QFormLayout::FieldRole, comboEmploi);

        lblDesc = new QLabel(InterventionDialog);
        lblDesc->setObjectName("lblDesc");

        formLayout->setWidget(8, QFormLayout::LabelRole, lblDesc);

        editDescription = new QPlainTextEdit(InterventionDialog);
        editDescription->setObjectName("editDescription");
        editDescription->setMaximumSize(QSize(16777215, 80));

        formLayout->setWidget(8, QFormLayout::FieldRole, editDescription);

        lblCR = new QLabel(InterventionDialog);
        lblCR->setObjectName("lblCR");

        formLayout->setWidget(9, QFormLayout::LabelRole, lblCR);

        editCompteRendu = new QPlainTextEdit(InterventionDialog);
        editCompteRendu->setObjectName("editCompteRendu");
        editCompteRendu->setMaximumSize(QSize(16777215, 100));

        formLayout->setWidget(9, QFormLayout::FieldRole, editCompteRendu);


        verticalLayout->addLayout(formLayout);

        labelAide = new QLabel(InterventionDialog);
        labelAide->setObjectName("labelAide");
        labelAide->setWordWrap(true);
        labelAide->setProperty("class", QVariant(QString::fromUtf8("caption")));

        verticalLayout->addWidget(labelAide);

        buttonBox = new QDialogButtonBox(InterventionDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(InterventionDialog);

        QMetaObject::connectSlotsByName(InterventionDialog);
    } // setupUi

    void retranslateUi(QDialog *InterventionDialog)
    {
        InterventionDialog->setWindowTitle(QCoreApplication::translate("InterventionDialog", "Intervention", nullptr));
        labelTitre->setText(QCoreApplication::translate("InterventionDialog", "D\303\251tails de l'intervention", nullptr));
        lblType->setText(QCoreApplication::translate("InterventionDialog", "Type *", nullptr));
        lblZone->setText(QCoreApplication::translate("InterventionDialog", "Zone *", nullptr));
        lblPriorite->setText(QCoreApplication::translate("InterventionDialog", "Priorit\303\251 *", nullptr));
        lblStatut->setText(QCoreApplication::translate("InterventionDialog", "Statut *", nullptr));
        lblDebut->setText(QCoreApplication::translate("InterventionDialog", "Date de d\303\251but *", nullptr));
        dtDebut->setDisplayFormat(QCoreApplication::translate("InterventionDialog", "dd/MM/yyyy HH:mm", nullptr));
        checkFin->setText(QCoreApplication::translate("InterventionDialog", "Date de fin", nullptr));
        dtFin->setDisplayFormat(QCoreApplication::translate("InterventionDialog", "dd/MM/yyyy HH:mm", nullptr));
        lblResp->setText(QCoreApplication::translate("InterventionDialog", "Responsable", nullptr));
        editResponsable->setPlaceholderText(QCoreApplication::translate("InterventionDialog", "Nom du responsable de la mission", nullptr));
        lblEmploi->setText(QCoreApplication::translate("InterventionDialog", "Emploi li\303\251", nullptr));
        lblDesc->setText(QCoreApplication::translate("InterventionDialog", "Description", nullptr));
        lblCR->setText(QCoreApplication::translate("InterventionDialog", "Compte-rendu", nullptr));
        editCompteRendu->setPlaceholderText(QCoreApplication::translate("InterventionDialog", "Obligatoire pour cl\303\264turer (20 caract\303\250res minimum)", nullptr));
        labelAide->setText(QCoreApplication::translate("InterventionDialog", "* champs obligatoires \342\200\224 la zone doit \303\252tre active ; une intervention \302\253 En cours \302\273 exige un responsable ou un emploi li\303\251.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InterventionDialog: public Ui_InterventionDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTERVENTIONDIALOG_H
