#pragma once
#include <QDateTime>
#include <QString>
#include <QVector>

// Une intervention telle qu'elle est affichée dans l'application.
// Le champ "statut" contient le LIBELLÉ avec accents (« Planifiée »...) ;
// la base Oracle stocke des codes sans accent (« Planifiee »...) : la
// conversion est faite uniquement dans InterventionService.
struct Intervention {
    int id = 0;
    QString type;
    QString priorite;
    QString statut;
    QDateTime debut;
    QDateTime fin;
    QString description;
    QString responsable;
    QString compteRendu;

    int idZone = 0;
    QString zoneNom;
    int idAgent = 0;            // relation « Affect »
    QString agentNom;
    int idEquipement = 0;       // relation « Utilise »
    QString equipementNom;
};

namespace Priorite {
    const QString Faible = QStringLiteral("Faible");
    const QString Moyenne = QStringLiteral("Moyenne");
    const QString Haute = QStringLiteral("Haute");
    const QString Critique = QStringLiteral("Critique");
}

namespace Statut {
    const QString Planifiee = QStringLiteral("Planifiée");
    const QString EnCours = QStringLiteral("En cours");
    const QString Terminee = QStringLiteral("Terminée");
    const QString Annulee = QStringLiteral("Annulée");
}

struct IdLabel {
    int id = 0;
    QString label;
};
