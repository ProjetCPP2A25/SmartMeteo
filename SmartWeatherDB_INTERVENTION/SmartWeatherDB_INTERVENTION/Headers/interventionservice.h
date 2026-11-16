#pragma once
#include "intervention.h"

#include <QDate>
#include <QPair>
#include <QStringList>
#include <QVector>
#include <optional>

struct InterventionFilter {
    QString texte, statut, priorite, type;
    int idZone = 0;
    bool periode = false;
    QDate du, au;
};

struct DurationEstimate {
    double heures = -1;      // < 0 : pas assez de données
    int echantillon = 0;
    QString base;
};

struct ZoneItem {
    int id = 0;
    QString nom;
    bool active = true;
};

// Métier « Détecter les conflits » : deux interventions actives qui se chevauchent
// et utilisent le même agent (Affect) ou le même équipement (Utilise).
struct Conflict {
    int idA = 0, idB = 0;
    QString typeA, typeB;
    QDateTime debutA, debutB;
    QString cause;           // « Même agent : ... » / « Même équipement : ... »
};

struct StatsSummary {
    int total = 0, planifiees = 0, enCours = 0, terminees = 0, annulees = 0, enRetard = 0;
    double dureeMoyenneH = -1;
};

using Counts = QVector<QPair<QString, int>>;

class InterventionService {
public:
    // --- listes de valeurs autorisées (libellés avec accents) ---
    static const QStringList &types();
    static const QStringList &priorites();
    static const QStringList &statuts();
    static QStringList transitionsFrom(const QString &statut);

    // --- conversion libellé <-> code stocké en base (sans accent) ---
    static QString statutToDb(const QString &libelle);
    static QString statutFromDb(const QString &code);

    // --- contrôle de saisie ---
    static QStringList validate(const Intervention &i, const Intervention *ancien = nullptr);
    static QStringList controleCloture(const Intervention &i);

    // --- CRUD ---
    static bool add(Intervention &i, QString *err = nullptr);
    static bool update(const Intervention &i, QString *err = nullptr);
    static bool remove(int id, QString *err = nullptr);
    static bool cancel(int id, QString *err = nullptr);
    static bool cloturer(int id, const QString &compteRendu, const QDateTime &fin, QString *err = nullptr);
    static std::optional<Intervention> get(int id);
    static QVector<Intervention> list(const InterventionFilter &f = {}, QString *err = nullptr);

    // --- métiers ---
    static DurationEstimate estimerDuree(const QString &type, int idZone);
    static bool appliquerFinEstimee(int idIntervention, const QDateTime &fin, QString *err = nullptr);
    static QVector<Conflict> detecterConflits();

    // --- statistiques ---
    static Counts repartition(const QString &dimension, const InterventionFilter &f = {});
    static StatsSummary resume(const InterventionFilter &f = {});

    // --- données pour les listes déroulantes ---
    static QVector<ZoneItem> zones();
    static QVector<IdLabel> agents(int inclureId = 0);
    static QVector<IdLabel> equipements(int inclureId = 0);
};
