#include "interventionwidget.h"
#include "interventiondialog.h"
#include "interventionservice.h"
#include "statschartwidget.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMarginsF>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QStringConverter>
#include <QTabBar>
#include <QTabWidget>
#include <QTableView>
#include <QTableWidget>
#include <QTextDocument>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>

namespace {

enum Col { ColId, ColType, ColZone, ColPriorite, ColStatut, ColDebut, ColFin, ColResp, ColAgent, ColEquip, ColAlerte, ColCount };

const QString kDateFmt = QStringLiteral("dd/MM/yyyy HH:mm");

QColor prioriteColor(const QString &p)
{
    if (p == Priorite::Critique) return QColor("#E05A5B");
    if (p == Priorite::Haute)    return QColor("#E49740");
    if (p == Priorite::Moyenne)  return QColor("#D8B65C");
    return QColor("#3FC98F");
}

QColor statutColor(const QString &s)
{
    if (s == Statut::Terminee) return QColor("#3FC98F");
    if (s == Statut::EnCours)  return QColor("#19C6B7");
    if (s == Statut::Annulee)  return QColor("#8A9BA5");
    return QColor("#6CB4E4");   // Planifiée (couleur claire : lisible sur fond sombre)
}

QStandardItem *makeItem(const QString &text, const QVariant &sortKey)
{
    auto *it = new QStandardItem(text);
    it->setEditable(false);
    it->setData(sortKey, Qt::UserRole);
    return it;
}

QString fmtHeures(double h)
{
    const int minutes = qRound(h * 60.0);
    return QStringLiteral("%1 h %2 min").arg(minutes / 60).arg(minutes % 60, 2, 10, QLatin1Char('0'));
}

QString csvCell(QString s)
{
    s.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    s.replace(QLatin1Char('\n'), QLatin1Char(' '));
    return QLatin1Char('"') + s + QLatin1Char('"');
}

bool askYesNo(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::No, parent);
    box.setButtonText(QMessageBox::Yes, QStringLiteral("Oui"));
    box.setButtonText(QMessageBox::No, QStringLiteral("Non"));
    box.setDefaultButton(QMessageBox::No);
    return box.exec() == QMessageBox::Yes;
}

// Carte « indicateur » : grande valeur colorée + légende.
QFrame *makeCard(const QString &objectName, const QString &caption, const QString &color, QLabel *&valueLabel)
{
    auto *frame = new QFrame;
    frame->setObjectName(objectName);
    auto *lay = new QVBoxLayout(frame);
    lay->setContentsMargins(14, 12, 14, 12);
    lay->setSpacing(2);
    valueLabel = new QLabel(QStringLiteral("0"));
    valueLabel->setObjectName(QStringLiteral("kpiValue"));
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet(QStringLiteral("color: %1;").arg(color));
    auto *cap = new QLabel(caption);
    cap->setObjectName(QStringLiteral("kpiCaption"));
    cap->setAlignment(Qt::AlignCenter);
    lay->addWidget(valueLabel);
    lay->addWidget(cap);
    return frame;
}

QDateEdit *makeDateEdit(const QDate &d)
{
    auto *e = new QDateEdit(d);
    e->setCalendarPopup(true);
    e->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    e->setMinimumWidth(130);   // évite que la date soit coupée (« 16/08/202 »)
    return e;
}

} // namespace

InterventionWidget::InterventionWidget(QWidget *parent) : QWidget(parent)
{
    buildUi();
    refresh();
}

// ------------------------------------------------------------------ interface

void InterventionWidget::buildUi()
{
    auto button = [this](const QString &text, const char *objName, auto slot) {
        auto *b = new QPushButton(text);
        if (objName && *objName) b->setObjectName(QString::fromLatin1(objName));
        b->setCursor(Qt::PointingHandCursor);
        connect(b, &QPushButton::clicked, this, slot);
        return b;
    };

    // ----- Titre + indicateurs -----
    auto *title = new QLabel(tr("Gestion des interventions"));
    title->setObjectName(QStringLiteral("title"));

    auto *kpiRow = new QHBoxLayout;
    kpiRow->setSpacing(12);
    kpiRow->addWidget(makeCard(QStringLiteral("kpi"), tr("Interventions"), QStringLiteral("#FFFFFF"), m_kTotal));
    kpiRow->addWidget(makeCard(QStringLiteral("kpi"), tr("En cours"), QStringLiteral("#19C6B7"), m_kEnCours));
    kpiRow->addWidget(makeCard(QStringLiteral("kpi"), tr("En retard"), QStringLiteral("#E49740"), m_kRetard));
    kpiRow->addWidget(makeCard(QStringLiteral("kpi"), tr("Critiques actives"), QStringLiteral("#E05A5B"), m_kCritiques));

    // ================= Onglet 1 : liste =================
    m_search = new QLineEdit;
    m_search->setPlaceholderText(tr("Rechercher (n°, type, zone, responsable, agent, statut…)"));
    m_search->setClearButtonEnabled(true);

    m_fStatut = new QComboBox;
    m_fStatut->addItem(tr("Tous les statuts"));
    m_fStatut->addItems(InterventionService::statuts());
    m_fPriorite = new QComboBox;
    m_fPriorite->addItem(tr("Toutes les priorités"));
    m_fPriorite->addItems(InterventionService::priorites());
    m_fType = new QComboBox;
    m_fType->addItem(tr("Tous les types"));
    m_fType->addItems(InterventionService::types());
    m_fZone = new QComboBox;
    m_fZone->addItem(tr("Toutes les zones"), 0);
    for (const ZoneItem &z : InterventionService::zones())
        m_fZone->addItem(z.nom, z.id);

    m_fPeriode = new QCheckBox(tr("Période du"));
    m_fDu = makeDateEdit(QDate::currentDate().addMonths(-3));
    m_fAu = makeDateEdit(QDate::currentDate().addMonths(1));

    auto *filters1 = new QHBoxLayout;
    filters1->addWidget(m_search, 3);
    filters1->addWidget(m_fStatut, 1);
    filters1->addWidget(m_fPriorite, 1);
    auto *filters2 = new QHBoxLayout;
    filters2->addWidget(m_fType, 1);
    filters2->addWidget(m_fZone, 1);
    filters2->addWidget(m_fPeriode);
    filters2->addWidget(m_fDu);
    filters2->addWidget(new QLabel(tr("au")));
    filters2->addWidget(m_fAu);

    m_model = new QStandardItemModel(0, ColCount, this);
    m_model->setHorizontalHeaderLabels({tr("N°"), tr("Type"), tr("Zone"), tr("Priorité"), tr("Statut"), tr("Début"),
                                        tr("Fin"), tr("Responsable"), tr("Agent"), tr("Équipement"), tr("Alerte")});
    m_model->setSortRole(Qt::UserRole);

    m_table = new QTableView;
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(ColDebut, Qt::DescendingOrder);
    connect(m_table, &QTableView::doubleClicked, this, &InterventionWidget::onEdit);

    auto *crud = new QHBoxLayout;
    crud->addWidget(button(tr("Ajouter"), "ok", &InterventionWidget::onAdd));
    crud->addWidget(button(tr("Modifier"), "", &InterventionWidget::onEdit));
    crud->addWidget(button(tr("Clôturer"), "ok", &InterventionWidget::onClose));
    crud->addWidget(button(tr("Annuler l'intervention"), "warn", &InterventionWidget::onCancel));
    crud->addWidget(button(tr("Supprimer"), "danger", &InterventionWidget::onDelete));
    crud->addStretch();
    crud->addWidget(button(tr("Export PDF"), "", &InterventionWidget::exportPdf));
    crud->addWidget(button(tr("Export CSV"), "", &InterventionWidget::exportCsv));

    auto *listPage = new QWidget;
    auto *listLay = new QVBoxLayout(listPage);
    listLay->setContentsMargins(14, 14, 14, 14);
    listLay->setSpacing(10);
    listLay->addLayout(filters1);
    listLay->addLayout(filters2);
    listLay->addWidget(m_table, 1);
    listLay->addLayout(crud);

    // ================= Onglet 2 : statistiques (style moderne) =================
    auto *pillRow = new QHBoxLayout;
    pillRow->setSpacing(8);
    m_dimGroup = new QButtonGroup(this);
    m_dimGroup->setExclusive(true);
    const QVector<QPair<QString, QString>> dims = {
        {tr("Statut"), QStringLiteral("statut")}, {tr("Priorité"), QStringLiteral("priorite")},
        {tr("Type"), QStringLiteral("type")},     {tr("Zone"), QStringLiteral("zone")},
        {tr("Mois"), QStringLiteral("mois")},     {tr("Agent"), QStringLiteral("agent")}};
    bool first = true;
    for (const auto &d : dims) {
        auto *b = new QPushButton(d.first);
        b->setObjectName(QStringLiteral("pill"));
        b->setCheckable(true);
        b->setChecked(first);
        b->setCursor(Qt::PointingHandCursor);
        b->setProperty("dim", d.second);
        m_dimGroup->addButton(b);
        pillRow->addWidget(b);
        first = false;
    }
    pillRow->addStretch();
    m_sPeriode = new QCheckBox(tr("Période du"));
    m_sDu = makeDateEdit(QDate::currentDate().addMonths(-3));
    m_sAu = makeDateEdit(QDate::currentDate().addMonths(1));
    pillRow->addWidget(m_sPeriode);
    pillRow->addWidget(m_sDu);
    pillRow->addWidget(new QLabel(tr("au")));
    pillRow->addWidget(m_sAu);

    auto *cardRow = new QHBoxLayout;
    cardRow->setSpacing(12);
    cardRow->addWidget(makeCard(QStringLiteral("statCard"), tr("Interventions"), QStringLiteral("#FFFFFF"), m_cTotal));
    cardRow->addWidget(makeCard(QStringLiteral("statCard"), tr("Taux de clôture"), QStringLiteral("#3FC98F"), m_cRate));
    cardRow->addWidget(makeCard(QStringLiteral("statCard"), tr("En retard"), QStringLiteral("#E49740"), m_cLate));
    cardRow->addWidget(makeCard(QStringLiteral("statCard"), tr("Durée moyenne"), QStringLiteral("#19C6B7"), m_cDuree));

    m_chart = new StatsChartWidget;
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(m_chart);
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; border: none; }"
                                         "QScrollArea > QWidget > QWidget { background: transparent; }"));

    auto *chartCard = new QFrame;
    chartCard->setObjectName(QStringLiteral("card"));
    auto *chartLay = new QVBoxLayout(chartCard);
    chartLay->setContentsMargins(4, 4, 4, 4);
    chartLay->addWidget(scroll);

    auto *statsPage = new QWidget;
    auto *statsLay = new QVBoxLayout(statsPage);
    statsLay->setContentsMargins(14, 14, 14, 14);
    statsLay->setSpacing(12);
    statsLay->addLayout(pillRow);
    statsLay->addLayout(cardRow);
    statsLay->addWidget(chartCard, 1);

    // ================= Onglets + métiers à côté des onglets =================
    auto *tabs = new QTabWidget;
    tabs->tabBar()->setExpanding(false);
    tabs->tabBar()->setCursor(Qt::PointingHandCursor);
    tabs->addTab(listPage, tr("Liste des interventions"));
    tabs->addTab(statsPage, tr("Statistiques"));

    auto *metiers = new QWidget;
    auto *metiersLay = new QHBoxLayout(metiers);
    metiersLay->setContentsMargins(0, 0, 0, 6);
    metiersLay->setSpacing(8);
    metiersLay->addWidget(button(tr("Estimer la durée"), "metier", &InterventionWidget::onEstimate));
    metiersLay->addWidget(button(tr("Détecter les conflits"), "metier", &InterventionWidget::onConflicts));
    tabs->setCornerWidget(metiers, Qt::TopRightCorner);

    m_msg = new QLabel;
    m_msg->setObjectName(QStringLiteral("message"));

    auto *root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->addWidget(title);
    root->addLayout(kpiRow);
    root->addWidget(tabs, 1);
    root->addWidget(m_msg);

    // ----- Connexions (après construction : évite les rafraîchissements prématurés) -----
    connect(m_search, &QLineEdit::textChanged, this, &InterventionWidget::refresh);
    for (auto *c : {m_fStatut, m_fPriorite, m_fType, m_fZone})
        connect(c, &QComboBox::currentIndexChanged, this, &InterventionWidget::refresh);
    connect(m_fPeriode, &QCheckBox::toggled, this, &InterventionWidget::refresh);
    connect(m_fDu, &QDateEdit::dateChanged, this, &InterventionWidget::refresh);
    connect(m_fAu, &QDateEdit::dateChanged, this, &InterventionWidget::refresh);

    connect(m_dimGroup, &QButtonGroup::buttonClicked, this, &InterventionWidget::refreshStats);
    connect(m_sPeriode, &QCheckBox::toggled, this, &InterventionWidget::refreshStats);
    connect(m_sDu, &QDateEdit::dateChanged, this, &InterventionWidget::refreshStats);
    connect(m_sAu, &QDateEdit::dateChanged, this, &InterventionWidget::refreshStats);
}

InterventionFilter InterventionWidget::currentFilter() const
{
    InterventionFilter f;
    f.texte = m_search->text();
    if (m_fStatut->currentIndex() > 0)   f.statut = m_fStatut->currentText();
    if (m_fPriorite->currentIndex() > 0) f.priorite = m_fPriorite->currentText();
    if (m_fType->currentIndex() > 0)     f.type = m_fType->currentText();
    f.idZone = m_fZone->currentData().toInt();
    f.periode = m_fPeriode->isChecked();
    f.du = m_fDu->date();
    f.au = m_fAu->date();
    return f;
}

int InterventionWidget::selectedId()
{
    const QModelIndexList rows = m_table->selectionModel()->selectedRows(ColId);
    if (rows.isEmpty()) {
        QMessageBox::information(this, tr("Sélection requise"), tr("Sélectionnez d'abord une intervention dans la liste."));
        return 0;
    }
    return rows.first().data(Qt::DisplayRole).toInt();
}

void InterventionWidget::notify(const QString &text)
{
    m_msg->setText(text);
    QTimer::singleShot(6000, m_msg, [this] { m_msg->clear(); });
}

// ------------------------------------------------------------------ données

void InterventionWidget::refresh()
{
    QString err;
    m_shown = InterventionService::list(currentFilter(), &err);
    if (!err.isEmpty())
        QMessageBox::critical(this, tr("Erreur base de données"), err);

    populateTable(m_shown);

    // indicateurs (toujours calculés sur l'ensemble des interventions, sans filtre)
    const QVector<Intervention> all = InterventionService::list();
    const QDateTime now = QDateTime::currentDateTime();
    int enCours = 0, retard = 0, critiques = 0;
    for (const Intervention &i : all) {
        if (i.statut == Statut::EnCours) {
            ++enCours;
            if (i.fin.isValid() && i.fin < now) ++retard;
        }
        const bool active = i.statut == Statut::Planifiee || i.statut == Statut::EnCours;
        if (active && i.priorite == Priorite::Critique) ++critiques;
    }
    m_kTotal->setText(QString::number(all.size()));
    m_kEnCours->setText(QString::number(enCours));
    m_kRetard->setText(QString::number(retard));
    m_kCritiques->setText(QString::number(critiques));

    refreshStats();
}

void InterventionWidget::populateTable(const QVector<Intervention> &rows)
{
    auto *header = m_table->horizontalHeader();
    const int sortSection = header->sortIndicatorSection();
    const Qt::SortOrder sortOrder = header->sortIndicatorOrder();
    const QDateTime now = QDateTime::currentDateTime();

    m_table->setSortingEnabled(false);
    m_model->setRowCount(0);

    QFont bold = m_table->font();
    bold.setBold(true);

    for (const Intervention &i : rows) {
        auto *id = makeItem(QString::number(i.id), i.id);
        id->setData(i.id, Qt::DisplayRole);

        auto *prio = makeItem(i.priorite, int(InterventionService::priorites().indexOf(i.priorite)));
        prio->setForeground(prioriteColor(i.priorite));
        prio->setFont(bold);

        auto *statut = makeItem(i.statut, i.statut);
        statut->setForeground(statutColor(i.statut));
        statut->setFont(bold);

        const bool late = i.statut == Statut::EnCours && i.fin.isValid() && i.fin < now;
        auto *alerte = makeItem(late ? tr("En retard") : QString(), late ? 1 : 0);
        alerte->setForeground(QColor("#E05A5B"));
        alerte->setFont(bold);

        const QString dash = QStringLiteral("—");
        m_model->appendRow({id,
                            makeItem(i.type, i.type),
                            makeItem(i.zoneNom, i.zoneNom),
                            prio,
                            statut,
                            makeItem(i.debut.toString(kDateFmt), i.debut.toString(Qt::ISODate)),
                            makeItem(i.fin.isValid() ? i.fin.toString(kDateFmt) : dash, i.fin.toString(Qt::ISODate)),
                            makeItem(i.responsable.isEmpty() ? dash : i.responsable, i.responsable),
                            makeItem(i.agentNom.isEmpty() ? dash : i.agentNom, i.agentNom),
                            makeItem(i.equipementNom.isEmpty() ? dash : i.equipementNom, i.equipementNom),
                            alerte});
    }

    m_table->setSortingEnabled(true);
    m_table->sortByColumn(sortSection, sortOrder);
}

void InterventionWidget::refreshStats()
{
    if (!m_chart || !m_dimGroup || !m_dimGroup->checkedButton()) return;

    InterventionFilter f;
    f.periode = m_sPeriode->isChecked();
    f.du = m_sDu->date();
    f.au = m_sAu->date();

    const QString dimKey = m_dimGroup->checkedButton()->property("dim").toString();
    const QString dimLabel = m_dimGroup->checkedButton()->text().toLower();

    const Counts counts = InterventionService::repartition(dimKey, f);
    const StatsSummary s = InterventionService::resume(f);

    m_cTotal->setText(QString::number(s.total));
    m_cRate->setText(QStringLiteral("%1 %").arg(s.total > 0 ? qRound(100.0 * s.terminees / s.total) : 0));
    m_cLate->setText(QString::number(s.enRetard));
    m_cDuree->setText(s.dureeMoyenneH < 0 ? QStringLiteral("—") : fmtHeures(s.dureeMoyenneH));

    const QString periode = f.periode
        ? tr("Du %1 au %2").arg(f.du.toString(QStringLiteral("dd/MM/yyyy")), f.au.toString(QStringLiteral("dd/MM/yyyy")))
        : tr("Toutes les dates");
    m_chart->setData(counts, tr("Répartition par %1").arg(dimLabel),
                     tr("%1 · %2 intervention(s)").arg(periode).arg(s.total));
}

// ------------------------------------------------------------------ CRUD

void InterventionWidget::onAdd()
{
    InterventionDialog dlg(0, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    notify(tr("Intervention #%1 créée.").arg(dlg.result().id));   // le formulaire a déjà enregistré
    refresh();
}

void InterventionWidget::onEdit()
{
    const int id = selectedId();
    if (!id) return;
    InterventionDialog dlg(id, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    notify(tr("Intervention #%1 mise à jour.").arg(id));
    refresh();
}

void InterventionWidget::onDelete()
{
    const int id = selectedId();
    if (!id) return;
    if (!askYesNo(this, tr("Confirmation"), tr("Supprimer définitivement l'intervention #%1 ?").arg(id)))
        return;
    QString err;
    if (!InterventionService::remove(id, &err)) {
        QMessageBox::warning(this, tr("Suppression refusée"), err);
        return;
    }
    notify(tr("Intervention #%1 supprimée.").arg(id));
    refresh();
}

void InterventionWidget::onCancel()
{
    const int id = selectedId();
    if (!id) return;
    if (!askYesNo(this, tr("Confirmation"), tr("Annuler l'intervention #%1 ?").arg(id)))
        return;
    QString err;
    if (!InterventionService::cancel(id, &err)) {
        QMessageBox::warning(this, tr("Annulation impossible"), err);
        return;
    }
    notify(tr("Intervention #%1 annulée.").arg(id));
    refresh();
}

void InterventionWidget::onClose()
{
    const int id = selectedId();
    if (!id) return;
    const auto cur = InterventionService::get(id);
    if (!cur) return;
    if (cur->statut != Statut::EnCours) {
        QMessageBox::warning(this, tr("Clôture impossible"), tr("Seule une intervention « En cours » peut être clôturée."));
        return;
    }

    QString compteRendu = cur->compteRendu;
    QDateTime fin = QDateTime::currentDateTime();
    fin.setTime(QTime(fin.time().hour(), fin.time().minute()));

    while (true) {
        QDialog dlg(this);
        dlg.setWindowTitle(tr("Clôturer l'intervention #%1").arg(id));
        dlg.setMinimumWidth(480);
        auto *cr = new QPlainTextEdit(compteRendu);
        cr->setPlaceholderText(tr("Compte-rendu (20 caractères minimum)"));
        auto *dte = new QDateTimeEdit(fin);
        dte->setCalendarPopup(true);
        dte->setDisplayFormat(QStringLiteral("dd/MM/yyyy HH:mm"));
        auto *form = new QFormLayout;
        form->addRow(tr("Fin réelle"), dte);
        form->addRow(tr("Compte-rendu *"), cr);
        auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        bb->button(QDialogButtonBox::Ok)->setText(tr("Clôturer"));
        bb->button(QDialogButtonBox::Cancel)->setText(tr("Annuler"));
        connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        auto *lay = new QVBoxLayout(&dlg);
        lay->addLayout(form);
        lay->addWidget(bb);

        if (dlg.exec() != QDialog::Accepted)
            return;
        compteRendu = cr->toPlainText().trimmed();
        fin = dte->dateTime();

        QString err;
        if (InterventionService::cloturer(id, compteRendu, fin, &err)) {
            notify(tr("Intervention #%1 clôturée.").arg(id));
            refresh();
            return;
        }
        QMessageBox::warning(this, tr("Clôture refusée"), err);   // puis on rouvre le formulaire
    }
}

// ------------------------------------------------------------------ métiers

// Métier 1 : estimer la durée d'une intervention d'après l'historique des interventions terminées.
void InterventionWidget::onEstimate()
{
    const int id = selectedId();
    if (!id) return;
    const auto cur = InterventionService::get(id);
    if (!cur) return;

    const DurationEstimate d = InterventionService::estimerDuree(cur->type, cur->idZone);
    if (d.heures < 0) {
        QMessageBox::information(this, tr("Estimation de durée"),
                                 tr("Pas assez de données : aucune intervention « %1 » terminée pour estimer la durée.").arg(cur->type));
        return;
    }

    const QDateTime finEstimee = cur->debut.addSecs(qint64(d.heures * 3600.0));
    QString text = tr("Durée estimée : %1\n\nBase de calcul : %2 (%3 intervention(s)).\n")
                       .arg(fmtHeures(d.heures), d.base).arg(d.echantillon);
    if (cur->fin.isValid())
        text += tr("Durée actuellement prévue : %1\n").arg(fmtHeures(cur->debut.secsTo(cur->fin) / 3600.0));
    text += tr("Fin estimée : %1").arg(finEstimee.toString(kDateFmt));

    const bool modifiable = cur->statut == Statut::Planifiee || cur->statut == Statut::EnCours;
    if (!modifiable) {
        QMessageBox::information(this, tr("Estimation de durée"), text);
        return;
    }
    if (!askYesNo(this, tr("Estimation de durée"),
                  text + tr("\n\nAppliquer cette date de fin à l'intervention #%1 ?").arg(id)))
        return;

    QString err;
    if (!InterventionService::appliquerFinEstimee(id, finEstimee, &err)) {
        QMessageBox::warning(this, tr("Estimation de durée"), err);
        return;
    }
    notify(tr("Date de fin de l'intervention #%1 fixée au %2.").arg(id).arg(finEstimee.toString(kDateFmt)));
    refresh();
}

// Métier 2 : détecter les chevauchements d'horaires (même agent ou même équipement).
void InterventionWidget::onConflicts()
{
    const QVector<Conflict> conflicts = InterventionService::detecterConflits();
    if (conflicts.isEmpty()) {
        QMessageBox::information(this, tr("Conflits de planning"),
                                 tr("Aucun conflit : aucune intervention active ne se chevauche avec un même agent ou un même équipement."));
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Conflits de planning"));
    dlg.resize(820, 380);
    auto *info = new QLabel(tr("%1 conflit(s) détecté(s) entre interventions actives (Planifiée / En cours) :").arg(conflicts.size()));
    auto *table = new QTableWidget(conflicts.size(), 4);
    table->setHorizontalHeaderLabels({tr("Intervention A"), tr("Intervention B"), tr("Cause"), tr("Chevauchement dès")});
    table->verticalHeader()->hide();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->horizontalHeader()->setStretchLastSection(true);
    for (int r = 0; r < conflicts.size(); ++r) {
        const Conflict &c = conflicts[r];
        table->setItem(r, 0, new QTableWidgetItem(QStringLiteral("#%1 — %2").arg(c.idA).arg(c.typeA)));
        table->setItem(r, 1, new QTableWidgetItem(QStringLiteral("#%1 — %2").arg(c.idB).arg(c.typeB)));
        table->setItem(r, 2, new QTableWidgetItem(c.cause));
        table->setItem(r, 3, new QTableWidgetItem(qMax(c.debutA, c.debutB).toString(kDateFmt)));
    }
    table->resizeColumnsToContents();
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    bb->button(QDialogButtonBox::Close)->setText(tr("Fermer"));
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    auto *lay = new QVBoxLayout(&dlg);
    lay->addWidget(info);
    lay->addWidget(table, 1);
    lay->addWidget(bb);
    dlg.exec();
}

// ------------------------------------------------------------------ exports

void InterventionWidget::exportCsv()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Exporter en CSV"),
                                                      QStringLiteral("interventions.csv"), tr("Fichier CSV (*.csv)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export"), tr("Impossible d'écrire le fichier : %1").arg(f.errorString()));
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);   // accents corrects dans Excel
    out << "N°;Type;Zone;Priorité;Statut;Début;Fin;Responsable;Agent;Équipement;Description;Compte-rendu\n";
    for (const Intervention &i : std::as_const(m_shown)) {
        out << i.id << ';' << csvCell(i.type) << ';' << csvCell(i.zoneNom) << ';' << csvCell(i.priorite) << ';'
            << csvCell(i.statut) << ';' << i.debut.toString(kDateFmt) << ';' << i.fin.toString(kDateFmt) << ';'
            << csvCell(i.responsable) << ';' << csvCell(i.agentNom) << ';' << csvCell(i.equipementNom) << ';'
            << csvCell(i.description) << ';' << csvCell(i.compteRendu) << '\n';
    }
    notify(tr("%1 intervention(s) exportée(s) en CSV.").arg(m_shown.size()));
}

void InterventionWidget::exportPdf()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Exporter en PDF"),
                                                      QStringLiteral("interventions.pdf"), tr("Document PDF (*.pdf)"));
    if (path.isEmpty()) return;

    QString html = QStringLiteral(
        "<h2 style='color:#162B3E'>Smart Weather Management — Interventions</h2>"
        "<p>Édité le %1 — %2 intervention(s)</p>"
        "<table border='1' cellspacing='0' cellpadding='4' width='100%'>"
        "<tr style='background-color:#162B3E;color:white'>"
        "<th>N°</th><th>Type</th><th>Zone</th><th>Priorité</th><th>Statut</th>"
        "<th>Début</th><th>Fin</th><th>Responsable</th><th>Agent</th></tr>")
        .arg(QDateTime::currentDateTime().toString(kDateFmt)).arg(m_shown.size());
    for (const Intervention &i : std::as_const(m_shown)) {
        html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td><td>%8</td><td>%9</td></tr>")
                    .arg(i.id)
                    .arg(i.type.toHtmlEscaped(), i.zoneNom.toHtmlEscaped(), i.priorite.toHtmlEscaped(), i.statut.toHtmlEscaped(),
                         i.debut.toString(kDateFmt), i.fin.isValid() ? i.fin.toString(kDateFmt) : QStringLiteral("—"),
                         i.responsable.toHtmlEscaped(), i.agentNom.toHtmlEscaped());
    }
    html += QStringLiteral("</table>");

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&writer);
    notify(tr("%1 intervention(s) exportée(s) en PDF.").arg(m_shown.size()));
}
