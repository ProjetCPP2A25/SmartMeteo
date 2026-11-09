#include "affectationintelligentewidget.h"

#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

AffectationIntelligenteWidget::AffectationIntelligenteWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    auto *intro = new QFrame(this);
    intro->setObjectName("card");
    auto *introLayout = new QVBoxLayout(intro);
    introLayout->setContentsMargins(16, 14, 16, 14);

    auto *title = new QLabel(
        QStringLiteral("Affectation intelligente des agents"), intro);
    title->setObjectName("cardTitle");

    auto *subtitle = new QLabel(
        QStringLiteral("Analyse la disponibilité, la zone et la charge de travail "
                       "avant d'affecter un agent à une intervention."), intro);
    subtitle->setObjectName("muted");
    subtitle->setWordWrap(true);
    introLayout->addWidget(title);
    introLayout->addWidget(subtitle);
    root->addWidget(intro);

    auto *selectionCard = new QFrame(this);
    selectionCard->setObjectName("card");
    auto *selectionLayout = new QVBoxLayout(selectionCard);
    selectionLayout->setContentsMargins(16, 14, 16, 14);
    selectionLayout->setSpacing(8);

    auto *selectionTitle = new QLabel(
        QStringLiteral("1. Choisir une intervention"), selectionCard);
    selectionTitle->setObjectName("cardTitle");
    selectionLayout->addWidget(selectionTitle);

    auto *selectionRow = new QHBoxLayout;
    m_interventionCombo = new QComboBox(selectionCard);
    m_interventionCombo->setMinimumWidth(300);
    m_interventionCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *btnAnalyser = new QPushButton(
        QStringLiteral("Analyser les disponibilités"), selectionCard);
    btnAnalyser->setObjectName("primary");
    selectionRow->addWidget(m_interventionCombo, 1);
    selectionRow->addWidget(btnAnalyser);
    selectionLayout->addLayout(selectionRow);
    root->addWidget(selectionCard);

    auto *resultsCard = new QFrame(this);
    resultsCard->setObjectName("card");
    auto *resultsLayout = new QVBoxLayout(resultsCard);
    resultsLayout->setContentsMargins(16, 14, 16, 14);
    resultsLayout->setSpacing(8);

    auto *resultsTitle = new QLabel(
        QStringLiteral("2. Agents recommandés"), resultsCard);
    resultsTitle->setObjectName("cardTitle");
    resultsLayout->addWidget(resultsTitle);

    m_info = new QLabel(
        QStringLiteral("Choisis une intervention puis analyse les disponibilités."),
        resultsCard);
    m_info->setObjectName("muted");
    m_info->setWordWrap(true);
    resultsLayout->addWidget(m_info);

    m_agentsModel = new QStandardItemModel(0, 5, this);
    m_agentsModel->setHorizontalHeaderLabels({
        QStringLiteral("Agent"), QStringLiteral("Rôle"),
        QStringLiteral("Zone"), QStringLiteral("Interventions actives"),
        QStringLiteral("Correspondance zone")
    });

    m_agentsTable = new QTableView(resultsCard);
    m_agentsTable->setModel(m_agentsModel);
    m_agentsTable->setAlternatingRowColors(true);
    m_agentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_agentsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_agentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_agentsTable->setShowGrid(false);
    m_agentsTable->verticalHeader()->hide();
    m_agentsTable->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    for (int col = 1; col < 5; ++col)
        m_agentsTable->horizontalHeader()->setSectionResizeMode(
            col, QHeaderView::ResizeToContents);
    m_agentsTable->setMinimumHeight(250);
    resultsLayout->addWidget(m_agentsTable, 1);

    auto *actions = new QHBoxLayout;
    actions->addStretch(1);
    m_btnAffecter = new QPushButton(
        QStringLiteral("Affecter l'agent sélectionné"), resultsCard);
    m_btnAffecter->setObjectName("primary");
    m_btnAffecter->setEnabled(false);
    actions->addWidget(m_btnAffecter);
    resultsLayout->addLayout(actions);
    root->addWidget(resultsCard, 1);

    connect(btnAnalyser, &QPushButton::clicked,
            this, &AffectationIntelligenteWidget::analyserDisponibilites);
    connect(m_btnAffecter, &QPushButton::clicked,
            this, &AffectationIntelligenteWidget::affecterAgentSelectionne);
    connect(m_agentsTable->selectionModel(),
            &QItemSelectionModel::selectionChanged, this, [this]() {
        m_btnAffecter->setEnabled(
            !m_agentsTable->selectionModel()->selectedRows().isEmpty());
    });
    connect(m_interventionCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        m_agentsModel->removeRows(0, m_agentsModel->rowCount());
        m_btnAffecter->setEnabled(false);
        m_info->setText(QStringLiteral(
            "Intervention sélectionnée. Clique sur « Analyser les disponibilités »."));
    });

    chargerInterventions();
}

void AffectationIntelligenteWidget::chargerInterventions()
{
    m_interventionCombo->clear();
    const QSqlDatabase db = QSqlDatabase::database();

    if (!db.isValid() || !db.isOpen()) {
        m_interventionCombo->addItem(
            QStringLiteral("Connexion Oracle indisponible"), -1);
        m_interventionCombo->setEnabled(false);
        m_info->setText(QStringLiteral(
            "Impossible de charger les interventions : vérifie la connexion Oracle."));
        return;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT i.ID_INTERVENTION, i.TYPE, i.STATUT, i.DATE_DEBUT,
               z.NOM, i.ID_ZONE
        FROM INTERVENTION i
        JOIN ZONE z ON z.ID_ZONE = i.ID_ZONE
        WHERE i.STATUT IN ('Planifiée', 'En cours')
        ORDER BY i.DATE_DEBUT, i.ID_INTERVENTION
    )");

    if (!query.exec()) {
        m_interventionCombo->addItem(QStringLiteral("Erreur de lecture Oracle"), -1);
        m_interventionCombo->setEnabled(false);
        m_info->setText(QStringLiteral("Erreur Oracle : %1")
                            .arg(query.lastError().text()));
        return;
    }

    while (query.next()) {
        const int id = query.value(0).toInt();
        const QString label = QStringLiteral(
            "#%1 — %2 | %3 | %4 | %5")
            .arg(id)
            .arg(query.value(1).toString())
            .arg(query.value(4).toString())
            .arg(query.value(2).toString())
            .arg(query.value(3).toDateTime().toString("dd/MM/yyyy HH:mm"));
        m_interventionCombo->addItem(label, id);
    }

    if (m_interventionCombo->count() == 0) {
        m_interventionCombo->addItem(
            QStringLiteral("Aucune intervention planifiée ou en cours"), -1);
        m_interventionCombo->setEnabled(false);
    } else {
        m_interventionCombo->setEnabled(true);
    }
}

void AffectationIntelligenteWidget::analyserDisponibilites()
{
    m_agentsModel->removeRows(0, m_agentsModel->rowCount());
    m_btnAffecter->setEnabled(false);

    const int idIntervention = m_interventionCombo->currentData().toInt();
    if (idIntervention <= 0) {
        QMessageBox::information(this, QStringLiteral("Affectation intelligente"),
                                 QStringLiteral("Choisis une intervention valide."));
        return;
    }

    const QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery intervention(db);
    intervention.prepare(R"(
        SELECT ID_ZONE, DATE_DEBUT, DATE_FIN, STATUT
        FROM INTERVENTION
        WHERE ID_INTERVENTION = :id
    )");
    intervention.bindValue(":id", idIntervention);

    if (!intervention.exec() || !intervention.next()) {
        m_info->setText(QStringLiteral("Impossible de lire l'intervention : %1")
                            .arg(intervention.lastError().text()));
        return;
    }

    const int idZone = intervention.value(0).toInt();
    const QDateTime debut = intervention.value(1).toDateTime();
    QDateTime fin = intervention.value(2).toDateTime();
    const QString statut = intervention.value(3).toString();

    if (statut != QStringLiteral("Planifiée")
        && statut != QStringLiteral("En cours")) {
        m_info->setText(QStringLiteral(
            "Cette intervention ne peut pas recevoir une nouvelle affectation."));
        return;
    }

    // Convention métier : une intervention sans DATE_FIN dure un jour pour
    // le calcul des conflits. Adapter cette durée si le cahier des charges le demande.
    if (!fin.isValid() || fin <= debut)
        fin = debut.addDays(1);

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT a.ID_AGENT, a.NOM, a.PRENOM, a.ROLE, a.ID_ZONE,
               COUNT(DISTINCT charge.ID_INTERVENTION) AS CHARGE
        FROM AGENT a
        LEFT JOIN INTERVENTION charge
          ON charge.ID_AGENT = a.ID_AGENT
         AND charge.STATUT IN ('Planifiée', 'En cours')
        WHERE a.STATUT = 'Disponible'
          AND NOT EXISTS (
              SELECT 1
              FROM INTERVENTION conflit
              WHERE conflit.ID_AGENT = a.ID_AGENT
                AND conflit.ID_INTERVENTION <> :idIntervention
                AND conflit.STATUT IN ('Planifiée', 'En cours')
                AND conflit.DATE_DEBUT < :fin
                AND NVL(conflit.DATE_FIN, conflit.DATE_DEBUT + 1) > :debut
          )
        GROUP BY a.ID_AGENT, a.NOM, a.PRENOM, a.ROLE, a.ID_ZONE
        ORDER BY CASE WHEN a.ID_ZONE = :zone THEN 0 ELSE 1 END,
                 COUNT(DISTINCT charge.ID_INTERVENTION), a.NOM, a.PRENOM
    )");
    query.bindValue(":idIntervention", idIntervention);
    query.bindValue(":fin", fin);
    query.bindValue(":debut", debut);
    query.bindValue(":zone", idZone);

    if (!query.exec()) {
        m_info->setText(QStringLiteral("Erreur Oracle : %1")
                            .arg(query.lastError().text()));
        return;
    }

    int count = 0;
    while (query.next()) {
        const int idAgent = query.value(0).toInt();
        const int zoneAgent = query.value(4).isNull()
                                  ? -1 : query.value(4).toInt();
        const bool memeZone = zoneAgent == idZone;
        const int charge = query.value(5).toInt();

        auto makeItem = [idAgent](const QString &text) {
            auto *item = new QStandardItem(text);
            item->setData(idAgent, Qt::UserRole);
            item->setEditable(false);
            return item;
        };

        QList<QStandardItem *> row;
        row << makeItem(query.value(2).toString() + " " + query.value(1).toString())
            << makeItem(query.value(3).toString())
            << makeItem(memeZone ? QStringLiteral("Même zone")
                                 : QStringLiteral("Autre zone"))
            << makeItem(QString::number(charge))
            << makeItem(memeZone ? QStringLiteral("Prioritaire")
                                 : QStringLiteral("À considérer"));
        m_agentsModel->appendRow(row);
        ++count;
    }

    if (count == 0) {
        m_info->setText(QStringLiteral(
            "Aucun agent au statut Disponible n'est libre sur cette période."));
    } else {
        m_info->setText(QStringLiteral(
            "%1 agent(s) disponible(s). Le classement favorise la même zone, "
            "puis la charge active la plus faible. Sélectionne un agent.")
                            .arg(count));
    }
}

void AffectationIntelligenteWidget::affecterAgentSelectionne()
{
    const int idIntervention = m_interventionCombo->currentData().toInt();
    const auto selected = m_agentsTable->selectionModel()->selectedRows();

    if (idIntervention <= 0 || selected.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Affectation"),
                                 QStringLiteral("Sélectionne une intervention et un agent."));
        return;
    }

    const int idAgent = selected.first().data(Qt::UserRole).toInt();
    if (QMessageBox::question(
            this, QStringLiteral("Confirmer l'affectation"),
            QStringLiteral("Confirmer l'affectation de cet agent à l'intervention #%1 ?")
                .arg(idIntervention),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::warning(this, QStringLiteral("Oracle"), db.lastError().text());
        return;
    }

    QSqlQuery intervention(db);
    intervention.prepare(R"(
        SELECT DATE_DEBUT, DATE_FIN, STATUT, ID_AGENT
        FROM INTERVENTION
        WHERE ID_INTERVENTION = :id
        FOR UPDATE
    )");
    intervention.bindValue(":id", idIntervention);

    if (!intervention.exec() || !intervention.next()) {
        const QString error = intervention.lastError().text();
        db.rollback();
        QMessageBox::warning(this, QStringLiteral("Affectation impossible"),
                             QStringLiteral("Intervention introuvable : %1").arg(error));
        return;
    }

    const QDateTime debut = intervention.value(0).toDateTime();
    QDateTime fin = intervention.value(1).toDateTime();
    const QString statut = intervention.value(2).toString();
    const QVariant ancienAgent = intervention.value(3);

    if (statut != QStringLiteral("Planifiée")
        && statut != QStringLiteral("En cours")) {
        db.rollback();
        QMessageBox::warning(this, QStringLiteral("Affectation impossible"),
                             QStringLiteral("L'intervention n'est plus affectable."));
        chargerInterventions();
        return;
    }
    if (!fin.isValid() || fin <= debut)
        fin = debut.addDays(1);

    QSqlQuery controle(db);
    controle.prepare(R"(
        SELECT COUNT(*)
        FROM AGENT a
        WHERE a.ID_AGENT = :agent
          AND a.STATUT = 'Disponible'
          AND NOT EXISTS (
              SELECT 1
              FROM INTERVENTION i
              WHERE i.ID_AGENT = a.ID_AGENT
                AND i.ID_INTERVENTION <> :intervention
                AND i.STATUT IN ('Planifiée', 'En cours')
                AND i.DATE_DEBUT < :fin
                AND NVL(i.DATE_FIN, i.DATE_DEBUT + 1) > :debut
          )
    )");
    controle.bindValue(":agent", idAgent);
    controle.bindValue(":intervention", idIntervention);
    controle.bindValue(":fin", fin);
    controle.bindValue(":debut", debut);

    if (!controle.exec() || !controle.next()
        || controle.value(0).toInt() != 1) {
        const QString error = controle.lastError().text();
        db.rollback();
        QMessageBox::warning(
            this, QStringLiteral("Affectation refusée"),
            error.isEmpty()
                ? QStringLiteral("L'agent n'est plus disponible ou un conflit horaire est détecté.")
                : error);
        analyserDisponibilites();
        return;
    }

    QSqlQuery update(db);
    update.prepare(
        "UPDATE INTERVENTION SET ID_AGENT = :agent "
        "WHERE ID_INTERVENTION = :id");
    update.bindValue(":agent", idAgent);
    update.bindValue(":id", idIntervention);

    if (!update.exec() || update.numRowsAffected() != 1) {
        const QString error = update.lastError().text();
        db.rollback();
        QMessageBox::warning(this, QStringLiteral("Erreur Oracle"), error);
        return;
    }

    QSqlQuery historique(db);
    historique.prepare(R"(
        INSERT INTO INTERVENTION_HISTORIQUE
            (ID_INTERVENTION, DATE_CHANGEMENT, CHAMP,
             ANCIENNE_VALEUR, NOUVELLE_VALEUR)
        VALUES (:id, SYSDATE, 'Affectation agent', :ancien, :nouveau)
    )");
    historique.bindValue(":id", idIntervention);
    historique.bindValue(":ancien", ancienAgent.isNull()
        ? QStringLiteral("Non affectée") : ancienAgent.toString());
    historique.bindValue(":nouveau", QString::number(idAgent));

    if (!historique.exec()) {
        const QString error = historique.lastError().text();
        db.rollback();
        QMessageBox::warning(this, QStringLiteral("Erreur historique"), error);
        return;
    }

    if (!db.commit()) {
        const QString error = db.lastError().text();
        db.rollback();
        QMessageBox::warning(this, QStringLiteral("Erreur Oracle"), error);
        return;
    }

    QMessageBox::information(
        this, QStringLiteral("Affectation réussie"),
        QStringLiteral("L'agent a été affecté à l'intervention et l'historique a été mis à jour."));
    chargerInterventions();
    m_agentsModel->removeRows(0, m_agentsModel->rowCount());
    m_info->setText(QStringLiteral("Affectation enregistrée dans Oracle."));
    m_btnAffecter->setEnabled(false);
}
