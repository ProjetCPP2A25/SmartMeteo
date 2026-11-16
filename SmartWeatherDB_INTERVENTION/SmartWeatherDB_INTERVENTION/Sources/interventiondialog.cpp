#include "interventiondialog.h"
#include "ui_interventiondialog.h"

#include "intervention.h"
#include "interventionservice.h"

#include <QDateTime>
#include <QMessageBox>
#include <QPushButton>
#include <optional>

namespace {
// Les dates sont enregistrées à la minute près.
QDateTime toMinute(QDateTime d)
{
    d.setTime(QTime(d.time().hour(), d.time().minute()));
    return d;
}
}

InterventionDialog::InterventionDialog(int idIntervention, QWidget *parent)
    : QDialog(parent), ui(new Ui::InterventionDialog), m_id(idIntervention)
{
    ui->setupUi(this);
    setMinimumWidth(620);

    std::optional<Intervention> cur;
    if (m_id > 0)
        cur = InterventionService::get(m_id);

    ui->comboType->addItems(InterventionService::types());
    ui->comboPriorite->addItems(InterventionService::priorites());

    // Statut : nouvelle intervention = « Planifiée » uniquement ; sinon seulement les transitions autorisées.
    if (cur) {
        ui->comboStatut->addItems(InterventionService::transitionsFrom(cur->statut));
    } else {
        ui->comboStatut->addItem(Statut::Planifiee);
        ui->comboStatut->setEnabled(false);
    }

    // Zones actives (+ la zone actuelle en modification)
    for (const ZoneItem &z : InterventionService::zones())
        if (z.active || (cur && cur->idZone == z.id))
            ui->comboZone->addItem(z.nom, z.id);

    // Agent (Affect) et équipement (Utilise) : facultatifs
    ui->comboAgent->addItem(QStringLiteral("— aucun —"), 0);
    for (const IdLabel &a : InterventionService::agents(cur ? cur->idAgent : 0))
        ui->comboAgent->addItem(a.label, a.id);
    ui->comboEquipement->addItem(QStringLiteral("— aucun —"), 0);
    for (const IdLabel &e : InterventionService::equipements(cur ? cur->idEquipement : 0))
        ui->comboEquipement->addItem(e.label, e.id);

    QDateTime now = QDateTime::currentDateTime();
    now.setTime(QTime(now.time().hour(), 0));
    ui->dtDebut->setDateTime(now);
    ui->dtFin->setDateTime(now.addSecs(2 * 3600));
    ui->dtFin->setEnabled(false);
    connect(ui->checkFin, &QCheckBox::toggled, ui->dtFin, &QWidget::setEnabled);

    ui->buttonBox->button(QDialogButtonBox::Save)->setText(QStringLiteral("Enregistrer"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Annuler"));
    ui->buttonBox->button(QDialogButtonBox::Save)->setObjectName(QStringLiteral("primary"));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &InterventionDialog::onSave);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (cur) {
        setWindowTitle(QStringLiteral("Modifier l'intervention #%1").arg(m_id));
        ui->comboType->setCurrentText(cur->type);
        ui->comboPriorite->setCurrentText(cur->priorite);
        ui->comboStatut->setCurrentText(cur->statut);
        ui->comboZone->setCurrentIndex(qMax(0, ui->comboZone->findData(cur->idZone)));
        ui->comboAgent->setCurrentIndex(qMax(0, ui->comboAgent->findData(cur->idAgent)));
        ui->comboEquipement->setCurrentIndex(qMax(0, ui->comboEquipement->findData(cur->idEquipement)));
        ui->dtDebut->setDateTime(cur->debut);
        ui->checkFin->setChecked(cur->fin.isValid());
        if (cur->fin.isValid())
            ui->dtFin->setDateTime(cur->fin);
        ui->editResponsable->setText(cur->responsable);
        ui->editDescription->setPlainText(cur->description);
        ui->editCompteRendu->setPlainText(cur->compteRendu);
    } else {
        setWindowTitle(QStringLiteral("Nouvelle intervention"));
    }
}

InterventionDialog::~InterventionDialog() { delete ui; }

void InterventionDialog::onSave()
{
    Intervention i;
    i.id = m_id;
    i.type = ui->comboType->currentText();
    i.priorite = ui->comboPriorite->currentText();
    i.statut = ui->comboStatut->currentText();
    i.idZone = ui->comboZone->currentData().toInt();
    i.idAgent = ui->comboAgent->currentData().toInt();
    i.idEquipement = ui->comboEquipement->currentData().toInt();
    i.debut = toMinute(ui->dtDebut->dateTime());
    if (ui->checkFin->isChecked())
        i.fin = toMinute(ui->dtFin->dateTime());
    i.responsable = ui->editResponsable->text().trimmed();
    i.description = ui->editDescription->toPlainText().trimmed();
    i.compteRendu = ui->editCompteRendu->toPlainText().trimmed();

    // Une seule écriture dans la base, faite ICI (l'ancienne version l'écrivait aussi dans la page : doublon).
    QString err;
    const bool ok = (m_id > 0) ? InterventionService::update(i, &err) : InterventionService::add(i, &err);
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("Contrôle de saisie"), err);
        return;   // le formulaire reste ouvert : la saisie n'est pas perdue
    }
    m_result = i;
    accept();
}
