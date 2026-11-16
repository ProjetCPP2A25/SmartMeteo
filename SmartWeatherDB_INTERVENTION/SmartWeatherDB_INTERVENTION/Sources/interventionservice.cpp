#include "interventionservice.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <utility>

namespace {

// ------------------------------------------------------------------ outils

bool dbReady(QString *err = nullptr)
{
    if (QSqlDatabase::database().isOpen())
        return true;
    if (err) *err = QStringLiteral("La connexion Oracle n'est pas ouverte.");
    return false;
}

QString sqlError(const QSqlQuery &q) { return q.lastError().text(); }

// Minuscules, sans accents. Répare aussi le « mojibake » (UTF-8 lu comme Latin-1) :
// « PlanifiÃ©e » redevient « planifiee ».
QString fold(QString s)
{
    if (s.contains(QChar(0x00C3)))
        s = QString::fromUtf8(s.toLatin1());
    s = s.normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(s.size());
    for (const QChar c : std::as_const(s))
        if (c.category() != QChar::Mark_NonSpacing)
            out += c;
    return out.toLower().trimmed();
}

// NULL typés : indispensable avec le pilote ODBC (un QVariant() sans type peut être rejeté).
QVariant nullInt()  { return QVariant(QMetaType::fromType<int>()); }
QVariant nullDate() { return QVariant(QMetaType::fromType<QDateTime>()); }
QVariant nullText() { return QVariant(QMetaType::fromType<QString>()); }
QVariant intOrNull(int v)               { return v > 0 ? QVariant(v) : nullInt(); }
QVariant dateOrNull(const QDateTime &d) { return d.isValid() ? QVariant(d) : nullDate(); }
QVariant textOrNull(const QString &s)   { return s.trimmed().isEmpty() ? nullText() : QVariant(s.trimmed()); }

QString lookup(const QString &sql, int id)
{
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(0, id);
    return (q.exec() && q.next()) ? q.value(0).toString() : QString();
}

Intervention fromQuery(const QSqlQuery &q)
{
    Intervention i;
    i.id = q.value("ID_INTERVENTION").toInt();
    i.type = q.value("TYPE").toString();
    i.priorite = q.value("PRIORITE").toString();
    i.statut = InterventionService::statutFromDb(q.value("STATUT").toString());
    i.debut = q.value("DATE_DEBUT").toDateTime();
    i.fin = q.value("DATE_FIN").isNull() ? QDateTime() : q.value("DATE_FIN").toDateTime();
    i.description = q.value("DESCRIPTION").toString();
    i.responsable = q.value("RESPONSABLE").toString();
    i.compteRendu = q.value("COMPTE_RENDU").toString();
    i.idZone = q.value("ID_ZONE").toInt();
    i.zoneNom = q.value("ZONE_NOM").toString();
    i.idAgent = q.value("ID_AGENT").toInt();
    i.agentNom = q.value("AGENT_NOM").toString();
    i.idEquipement = q.value("ID_EQUIPEMENT").toInt();
    i.equipementNom = q.value("EQUIPEMENT_NOM").toString();
    return i;
}

// Intervention + zone + agent + équipement (jointures externes : agent et équipement sont facultatifs).
QString baseSelect()
{
    return QStringLiteral(
        "SELECT I.ID_INTERVENTION, I.TYPE, I.PRIORITE, I.STATUT, I.DATE_DEBUT, I.DATE_FIN, "
        "       I.DESCRIPTION, I.RESPONSABLE, I.COMPTE_RENDU, I.ID_ZONE, I.ID_AGENT, I.ID_EQUIPEMENT, "
        "       Z.NOM AS ZONE_NOM, A.PRENOM || ' ' || A.NOM AS AGENT_NOM, E.NOM AS EQUIPEMENT_NOM "
        "  FROM INTERVENTION I "
        "  JOIN ZONE Z ON Z.ID_ZONE = I.ID_ZONE "
        "  LEFT JOIN AGENT A ON A.ID_AGENT = I.ID_AGENT "
        "  LEFT JOIN EQUIPEMENT E ON E.ID_EQUIPEMENT = I.ID_EQUIPEMENT");
}

} // namespace

// ------------------------------------------------------------------ valeurs autorisées

const QStringList &InterventionService::types()
{
    static const QStringList v = {QStringLiteral("Inondation"), QStringLiteral("Chaleur"), QStringLiteral("Vent fort"),
                                  QStringLiteral("Maintenance station"), QStringLiteral("Inspection"), QStringLiteral("Autre")};
    return v;
}

const QStringList &InterventionService::priorites()
{
    static const QStringList v = {Priorite::Faible, Priorite::Moyenne, Priorite::Haute, Priorite::Critique};
    return v;
}

const QStringList &InterventionService::statuts()
{
    static const QStringList v = {Statut::Planifiee, Statut::EnCours, Statut::Terminee, Statut::Annulee};
    return v;
}

QStringList InterventionService::transitionsFrom(const QString &s)
{
    if (s == Statut::Planifiee) return {Statut::Planifiee, Statut::EnCours, Statut::Annulee};
    if (s == Statut::EnCours)   return {Statut::EnCours, Statut::Terminee, Statut::Annulee};
    if (s == Statut::Terminee)  return {Statut::Terminee, Statut::EnCours};
    return {s};
}

QString InterventionService::statutToDb(const QString &libelle)
{
    const QString f = fold(libelle);
    if (f == QLatin1String("planifiee")) return QStringLiteral("Planifiee");
    if (f == QLatin1String("en cours"))  return QStringLiteral("En cours");
    if (f == QLatin1String("terminee"))  return QStringLiteral("Terminee");
    if (f == QLatin1String("annulee"))   return QStringLiteral("Annulee");
    return libelle;
}

QString InterventionService::statutFromDb(const QString &code)
{
    const QString f = fold(code);
    if (f == QLatin1String("planifiee")) return Statut::Planifiee;
    if (f == QLatin1String("en cours"))  return Statut::EnCours;
    if (f == QLatin1String("terminee"))  return Statut::Terminee;
    if (f == QLatin1String("annulee"))   return Statut::Annulee;
    return code;
}

// ------------------------------------------------------------------ contrôle de saisie

QStringList InterventionService::controleCloture(const Intervention &i)
{
    QStringList e;
    if (i.compteRendu.trimmed().size() < 20)
        e << QStringLiteral("Le compte-rendu doit contenir au moins 20 caractères pour clôturer.");
    return e;
}

QStringList InterventionService::validate(const Intervention &i, const Intervention *old)
{
    QStringList e;
    if (!types().contains(i.type))         e << QStringLiteral("Type invalide.");
    if (!priorites().contains(i.priorite)) e << QStringLiteral("Priorité invalide.");
    if (!statuts().contains(i.statut))     e << QStringLiteral("Statut invalide.");
    if (!old && i.statut != Statut::Planifiee)
        e << QStringLiteral("Une nouvelle intervention doit être « Planifiée ».");
    if (old && old->statut != i.statut && !transitionsFrom(old->statut).contains(i.statut))
        e << QStringLiteral("Transition de statut interdite (%1 → %2).").arg(old->statut, i.statut);

    if (!i.debut.isValid())
        e << QStringLiteral("La date de début est obligatoire.");
    if (i.debut.isValid() && i.fin.isValid() && i.fin < i.debut)
        e << QStringLiteral("La date de fin doit être postérieure ou égale au début.");

    // Zone (relation « Possède »)
    if (i.idZone <= 0) {
        e << QStringLiteral("Choisissez une zone.");
    } else if ((!old || old->idZone != i.idZone)
               && lookup(QStringLiteral("SELECT STATUT FROM ZONE WHERE ID_ZONE = ?"), i.idZone) != QLatin1String("Active")) {
        e << QStringLiteral("La zone choisie est inactive ou introuvable.");
    }

    // Responsable : lettres, espaces, apostrophes, points, tirets
    const QString resp = i.responsable.trimmed();
    static const QRegularExpression nameRe(QStringLiteral("^[\\p{L}][\\p{L} .'’-]{1,99}$"));
    if (!resp.isEmpty() && !nameRe.match(resp).hasMatch())
        e << QStringLiteral("Responsable : 2 à 100 caractères, lettres, espaces, apostrophes, points ou tirets uniquement.");

    if (i.description.size() > 500)  e << QStringLiteral("La description ne doit pas dépasser 500 caractères.");
    if (i.compteRendu.size() > 2000) e << QStringLiteral("Le compte-rendu ne doit pas dépasser 2000 caractères.");

    if (i.statut == Statut::EnCours && resp.isEmpty() && i.idAgent <= 0)
        e << QStringLiteral("Une intervention « En cours » exige un responsable ou un agent affecté.");
    if (i.statut == Statut::Terminee)
        e += controleCloture(i);

    // Agent (relation « Affect »)
    if (i.idAgent > 0 && (!old || old->idAgent != i.idAgent)) {
        const QString s = lookup(QStringLiteral("SELECT STATUT FROM AGENT WHERE ID_AGENT = ?"), i.idAgent);
        if (s.isEmpty())
            e << QStringLiteral("Agent introuvable.");
        else if (s != QLatin1String("Disponible") && s != QLatin1String("En service"))
            e << QStringLiteral("L'agent sélectionné n'est pas disponible (congé ou formation).");
    }

    // Équipement (relation « Utilise »)
    if (i.idEquipement > 0 && (!old || old->idEquipement != i.idEquipement)) {
        const QString s = lookup(QStringLiteral("SELECT STATUT FROM EQUIPEMENT WHERE ID_EQUIPEMENT = ?"), i.idEquipement);
        if (s.isEmpty())
            e << QStringLiteral("Équipement introuvable.");
        else if (s != QLatin1String("Disponible"))
            e << QStringLiteral("L'équipement sélectionné n'est pas disponible (%1).").arg(s);
    }
    return e;
}

// ------------------------------------------------------------------ CRUD

bool InterventionService::add(Intervention &i, QString *err)
{
    if (!dbReady(err)) return false;
    const QStringList e = validate(i);
    if (!e.isEmpty()) {
        if (err) *err = e.join(QLatin1Char('\n'));
        return false;
    }

    QSqlQuery seq;
    if (!seq.exec(QStringLiteral("SELECT SEQ_INTERVENTION.NEXTVAL FROM DUAL")) || !seq.next()) {
        if (err) *err = sqlError(seq);
        return false;
    }
    const int newId = seq.value(0).toInt();

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "INSERT INTO INTERVENTION (ID_INTERVENTION, TYPE, PRIORITE, STATUT, DATE_DEBUT, DATE_FIN, "
        "                          DESCRIPTION, RESPONSABLE, COMPTE_RENDU, ID_ZONE, ID_AGENT, ID_EQUIPEMENT) "
        "VALUES (:id, :t, :p, :s, :d, :f, :de, :r, :cr, :z, :a, :e)"));
    q.bindValue(":id", newId);
    q.bindValue(":t", i.type);
    q.bindValue(":p", i.priorite);
    q.bindValue(":s", statutToDb(i.statut));
    q.bindValue(":d", i.debut);
    q.bindValue(":f", dateOrNull(i.fin));
    q.bindValue(":de", textOrNull(i.description));
    q.bindValue(":r", textOrNull(i.responsable));
    q.bindValue(":cr", textOrNull(i.compteRendu));
    q.bindValue(":z", i.idZone);
    q.bindValue(":a", intOrNull(i.idAgent));
    q.bindValue(":e", intOrNull(i.idEquipement));
    if (!q.exec()) {
        if (err) *err = sqlError(q);
        return false;
    }
    i.id = newId;
    return true;
}

bool InterventionService::update(const Intervention &i, QString *err)
{
    if (!dbReady(err)) return false;
    const auto old = get(i.id);
    if (!old) {
        if (err) *err = QStringLiteral("Intervention introuvable.");
        return false;
    }
    const QStringList e = validate(i, &*old);
    if (!e.isEmpty()) {
        if (err) *err = e.join(QLatin1Char('\n'));
        return false;
    }

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "UPDATE INTERVENTION SET TYPE = :t, PRIORITE = :p, STATUT = :s, DATE_DEBUT = :d, DATE_FIN = :f, "
        "       DESCRIPTION = :de, RESPONSABLE = :r, COMPTE_RENDU = :cr, ID_ZONE = :z, ID_AGENT = :a, ID_EQUIPEMENT = :e "
        " WHERE ID_INTERVENTION = :id"));
    q.bindValue(":t", i.type);
    q.bindValue(":p", i.priorite);
    q.bindValue(":s", statutToDb(i.statut));
    q.bindValue(":d", i.debut);
    q.bindValue(":f", dateOrNull(i.fin));
    q.bindValue(":de", textOrNull(i.description));
    q.bindValue(":r", textOrNull(i.responsable));
    q.bindValue(":cr", textOrNull(i.compteRendu));
    q.bindValue(":z", i.idZone);
    q.bindValue(":a", intOrNull(i.idAgent));
    q.bindValue(":e", intOrNull(i.idEquipement));
    q.bindValue(":id", i.id);
    if (!q.exec()) {
        if (err) *err = sqlError(q);
        return false;
    }
    return true;
}

bool InterventionService::remove(int id, QString *err)
{
    if (!dbReady(err)) return false;
    QSqlQuery q;
    q.prepare(QStringLiteral("DELETE FROM INTERVENTION WHERE ID_INTERVENTION = :id"));
    q.bindValue(":id", id);
    if (!q.exec()) {
        if (err) *err = sqlError(q);
        return false;
    }
    if (q.numRowsAffected() <= 0) {
        if (err) *err = QStringLiteral("Intervention introuvable.");
        return false;
    }
    return true;
}

bool InterventionService::cancel(int id, QString *err)
{
    if (!dbReady(err)) return false;
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "UPDATE INTERVENTION SET STATUT = :s WHERE ID_INTERVENTION = :id AND STATUT IN (:p, :e)"));
    q.bindValue(":s", statutToDb(Statut::Annulee));
    q.bindValue(":id", id);
    q.bindValue(":p", statutToDb(Statut::Planifiee));
    q.bindValue(":e", statutToDb(Statut::EnCours));
    if (!q.exec()) {
        if (err) *err = sqlError(q);
        return false;
    }
    if (q.numRowsAffected() <= 0) {
        if (err) *err = QStringLiteral("Annulation impossible : seules les interventions « Planifiée » ou « En cours » peuvent être annulées.");
        return false;
    }
    return true;
}

bool InterventionService::cloturer(int id, const QString &report, const QDateTime &fin, QString *err)
{
    if (!dbReady(err)) return false;
    const auto cur = get(id);
    if (!cur) {
        if (err) *err = QStringLiteral("Intervention introuvable.");
        return false;
    }
    QStringList e;
    if (cur->statut != Statut::EnCours)
        e << QStringLiteral("Seule une intervention « En cours » peut être clôturée.");
    if (report.trimmed().size() < 20)
        e << QStringLiteral("Le compte-rendu doit contenir au moins 20 caractères.");
    if (!fin.isValid() || fin < cur->debut)
        e << QStringLiteral("La date de fin réelle doit être postérieure au début de l'intervention.");
    if (!e.isEmpty()) {
        if (err) *err = e.join(QLatin1Char('\n'));
        return false;
    }

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "UPDATE INTERVENTION SET STATUT = :s, COMPTE_RENDU = :c, DATE_FIN = :f WHERE ID_INTERVENTION = :id"));
    q.bindValue(":s", statutToDb(Statut::Terminee));
    q.bindValue(":c", report.trimmed());
    q.bindValue(":f", fin);
    q.bindValue(":id", id);
    if (!q.exec()) {
        if (err) *err = sqlError(q);
        return false;
    }
    return true;
}

std::optional<Intervention> InterventionService::get(int id)
{
    if (!dbReady()) return std::nullopt;
    QSqlQuery q;
    q.prepare(baseSelect() + QStringLiteral(" WHERE I.ID_INTERVENTION = :id"));
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) return std::nullopt;
    return fromQuery(q);
}

QVector<Intervention> InterventionService::list(const InterventionFilter &f, QString *err)
{
    QVector<Intervention> out;
    if (!dbReady(err)) return out;

    const QString txt = f.texte.trimmed();
    QString sql = baseSelect() + QStringLiteral(" WHERE 1 = 1");
    if (!f.statut.isEmpty())   sql += QStringLiteral(" AND I.STATUT = :s");
    if (!f.priorite.isEmpty()) sql += QStringLiteral(" AND I.PRIORITE = :p");
    if (!f.type.isEmpty())     sql += QStringLiteral(" AND I.TYPE = :t");
    if (f.idZone > 0)          sql += QStringLiteral(" AND I.ID_ZONE = :z");
    if (f.periode)             sql += QStringLiteral(" AND I.DATE_DEBUT >= :du AND I.DATE_DEBUT < :au");
    if (!txt.isEmpty())
        sql += QStringLiteral(
            " AND (TO_CHAR(I.ID_INTERVENTION) LIKE :txt OR UPPER(I.TYPE) LIKE UPPER(:txt) "
            "      OR UPPER(Z.NOM) LIKE UPPER(:txt) OR UPPER(NVL(I.RESPONSABLE, ' ')) LIKE UPPER(:txt) "
            "      OR UPPER(NVL(A.PRENOM || ' ' || A.NOM, ' ')) LIKE UPPER(:txt) "
            "      OR UPPER(I.STATUT) LIKE UPPER(:txs))");
    sql += QStringLiteral(" ORDER BY I.DATE_DEBUT DESC");

    QSqlQuery q;
    if (!q.prepare(sql)) {
        if (err) *err = sqlError(q);
        return out;
    }
    if (!f.statut.isEmpty())   q.bindValue(":s", statutToDb(f.statut));
    if (!f.priorite.isEmpty()) q.bindValue(":p", f.priorite);
    if (!f.type.isEmpty())     q.bindValue(":t", f.type);
    if (f.idZone > 0)          q.bindValue(":z", f.idZone);
    if (f.periode) {
        q.bindValue(":du", QDateTime(f.du, QTime(0, 0)));
        q.bindValue(":au", QDateTime(f.au.addDays(1), QTime(0, 0)));
    }
    if (!txt.isEmpty()) {
        q.bindValue(":txt", QStringLiteral("%") + txt + QStringLiteral("%"));
        q.bindValue(":txs", QStringLiteral("%") + fold(txt) + QStringLiteral("%"));   // « planifiée » trouve « Planifiee »
    }
    if (!q.exec()) {
        if (err) *err = sqlError(q);
        return out;
    }
    while (q.next())
        out << fromQuery(q);
    return out;
}

// ------------------------------------------------------------------ métier 1 : estimer la durée

DurationEstimate InterventionService::estimerDuree(const QString &type, int idZone)
{
    DurationEstimate d;
    auto run = [&](bool avecZone) {
        QString sql = QStringLiteral(
            "SELECT AVG((DATE_FIN - DATE_DEBUT) * 24), COUNT(*) FROM INTERVENTION "
            " WHERE TYPE = :t AND STATUT = 'Terminee' AND DATE_FIN IS NOT NULL");
        if (avecZone) sql += QStringLiteral(" AND ID_ZONE = :z");
        QSqlQuery q;
        q.prepare(sql);
        q.bindValue(":t", type);
        if (avecZone) q.bindValue(":z", idZone);
        if (q.exec() && q.next() && q.value(1).toInt() > 0) {
            d.heures = q.value(0).toDouble();
            d.echantillon = q.value(1).toInt();
            return true;
        }
        return false;
    };
    if (run(true))
        d.base = QStringLiteral("interventions terminées de même type et de même zone");
    else if (run(false))
        d.base = QStringLiteral("interventions terminées de même type (toutes zones confondues)");
    return d;
}

bool InterventionService::appliquerFinEstimee(int id, const QDateTime &fin, QString *err)
{
    if (!dbReady(err)) return false;
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "UPDATE INTERVENTION SET DATE_FIN = :f WHERE ID_INTERVENTION = :id AND STATUT IN ('Planifiee', 'En cours') "
        "AND DATE_DEBUT <= :f2"));
    q.bindValue(":f", fin);
    q.bindValue(":id", id);
    q.bindValue(":f2", fin);
    if (!q.exec()) {
        if (err) *err = sqlError(q);
        return false;
    }
    if (q.numRowsAffected() <= 0) {
        if (err) *err = QStringLiteral("Mise à jour impossible : l'intervention doit être « Planifiée » ou « En cours ».");
        return false;
    }
    return true;
}

// ------------------------------------------------------------------ métier 2 : détecter les conflits

QVector<Conflict> InterventionService::detecterConflits()
{
    QVector<Conflict> out;
    if (!dbReady()) return out;

    // Deux interventions actives (Planifiée / En cours) dont les horaires se chevauchent et qui partagent
    // le même agent ou le même équipement. Sans fin renseignée, on suppose une durée de 2 h.
    QSqlQuery q;
    const bool ok = q.exec(QStringLiteral(
        "SELECT A.ID_INTERVENTION, A.TYPE, A.DATE_DEBUT, B.ID_INTERVENTION, B.TYPE, B.DATE_DEBUT, "
        "       CASE WHEN A.ID_AGENT = B.ID_AGENT AND A.ID_EQUIPEMENT = B.ID_EQUIPEMENT THEN 'BOTH' "
        "            WHEN A.ID_AGENT = B.ID_AGENT THEN 'AGENT' ELSE 'EQUIP' END AS CAUSE, "
        "       AG.PRENOM || ' ' || AG.NOM AS AGENT_NOM, EQ.NOM AS EQUIP_NOM "
        "  FROM INTERVENTION A "
        "  JOIN INTERVENTION B ON A.ID_INTERVENTION < B.ID_INTERVENTION "
        "  LEFT JOIN AGENT AG ON AG.ID_AGENT = A.ID_AGENT "
        "  LEFT JOIN EQUIPEMENT EQ ON EQ.ID_EQUIPEMENT = A.ID_EQUIPEMENT "
        " WHERE A.STATUT IN ('Planifiee', 'En cours') AND B.STATUT IN ('Planifiee', 'En cours') "
        "   AND ((A.ID_AGENT IS NOT NULL AND A.ID_AGENT = B.ID_AGENT) "
        "     OR (A.ID_EQUIPEMENT IS NOT NULL AND A.ID_EQUIPEMENT = B.ID_EQUIPEMENT)) "
        "   AND A.DATE_DEBUT < NVL(B.DATE_FIN, B.DATE_DEBUT + 2/24) "
        "   AND B.DATE_DEBUT < NVL(A.DATE_FIN, A.DATE_DEBUT + 2/24) "
        " ORDER BY A.DATE_DEBUT"));
    if (!ok) return out;

    while (q.next()) {
        Conflict c;
        c.idA = q.value(0).toInt();
        c.typeA = q.value(1).toString();
        c.debutA = q.value(2).toDateTime();
        c.idB = q.value(3).toInt();
        c.typeB = q.value(4).toString();
        c.debutB = q.value(5).toDateTime();
        const QString cause = q.value(6).toString();
        const QString agent = q.value(7).toString();
        const QString equip = q.value(8).toString();
        if (cause == QLatin1String("BOTH"))
            c.cause = QStringLiteral("Même agent (%1) et même équipement (%2)").arg(agent, equip);
        else if (cause == QLatin1String("AGENT"))
            c.cause = QStringLiteral("Même agent : %1").arg(agent);
        else
            c.cause = QStringLiteral("Même équipement : %1").arg(equip);
        out << c;
    }
    return out;
}

// ------------------------------------------------------------------ statistiques

Counts InterventionService::repartition(const QString &dim, const InterventionFilter &f)
{
    Counts c;
    if (!dbReady()) return c;

    QString expr = QStringLiteral("I.STATUT");
    if (dim == QLatin1String("priorite"))      expr = QStringLiteral("I.PRIORITE");
    else if (dim == QLatin1String("type"))     expr = QStringLiteral("I.TYPE");
    else if (dim == QLatin1String("zone"))     expr = QStringLiteral("Z.NOM");
    else if (dim == QLatin1String("mois"))     expr = QStringLiteral("TO_CHAR(I.DATE_DEBUT, 'YYYY-MM')");
    else if (dim == QLatin1String("agent"))    expr = QStringLiteral("A.PRENOM || ' ' || A.NOM");

    QString sql = QStringLiteral(
        "SELECT %1 AS LBL, COUNT(*) AS NB FROM INTERVENTION I "
        "JOIN ZONE Z ON Z.ID_ZONE = I.ID_ZONE LEFT JOIN AGENT A ON A.ID_AGENT = I.ID_AGENT WHERE 1 = 1").arg(expr);
    if (f.periode) sql += QStringLiteral(" AND I.DATE_DEBUT >= :d AND I.DATE_DEBUT < :a");
    sql += QStringLiteral(" GROUP BY %1 ORDER BY %2").arg(expr, dim == QLatin1String("mois") ? QStringLiteral("1") : QStringLiteral("2 DESC, 1"));

    QSqlQuery q;
    if (!q.prepare(sql)) return c;
    if (f.periode) {
        q.bindValue(":d", QDateTime(f.du, QTime(0, 0)));
        q.bindValue(":a", QDateTime(f.au.addDays(1), QTime(0, 0)));
    }
    if (!q.exec()) return c;
    while (q.next()) {
        QString label = q.value(0).toString();
        if (dim == QLatin1String("statut")) label = statutFromDb(label);
        if (label.isEmpty()) label = QStringLiteral("Non affecté");
        c << qMakePair(label, q.value(1).toInt());
    }
    return c;
}

StatsSummary InterventionService::resume(const InterventionFilter &f)
{
    StatsSummary s;
    if (!dbReady()) return s;

    QString sql = QStringLiteral(
        "SELECT COUNT(*), "
        "       SUM(CASE WHEN STATUT = 'Planifiee' THEN 1 ELSE 0 END), "
        "       SUM(CASE WHEN STATUT = 'En cours'  THEN 1 ELSE 0 END), "
        "       SUM(CASE WHEN STATUT = 'Terminee'  THEN 1 ELSE 0 END), "
        "       SUM(CASE WHEN STATUT = 'Annulee'   THEN 1 ELSE 0 END), "
        "       SUM(CASE WHEN STATUT = 'En cours' AND DATE_FIN IS NOT NULL AND DATE_FIN < SYSDATE THEN 1 ELSE 0 END), "
        "       AVG(CASE WHEN STATUT = 'Terminee' AND DATE_FIN IS NOT NULL THEN (DATE_FIN - DATE_DEBUT) * 24 END) "
        "  FROM INTERVENTION WHERE 1 = 1");
    if (f.periode) sql += QStringLiteral(" AND DATE_DEBUT >= :d AND DATE_DEBUT < :a");

    QSqlQuery q;
    if (!q.prepare(sql)) return s;
    if (f.periode) {
        q.bindValue(":d", QDateTime(f.du, QTime(0, 0)));
        q.bindValue(":a", QDateTime(f.au.addDays(1), QTime(0, 0)));
    }
    if (!q.exec() || !q.next()) return s;
    s.total = q.value(0).toInt();
    s.planifiees = q.value(1).toInt();
    s.enCours = q.value(2).toInt();
    s.terminees = q.value(3).toInt();
    s.annulees = q.value(4).toInt();
    s.enRetard = q.value(5).toInt();
    s.dureeMoyenneH = q.value(6).isNull() ? -1.0 : q.value(6).toDouble();
    return s;
}

// ------------------------------------------------------------------ listes déroulantes

QVector<ZoneItem> InterventionService::zones()
{
    QVector<ZoneItem> v;
    QSqlQuery q(QStringLiteral("SELECT ID_ZONE, NOM, STATUT FROM ZONE ORDER BY NOM"));
    while (q.next())
        v << ZoneItem{q.value(0).toInt(), q.value(1).toString(), q.value(2).toString() == QLatin1String("Active")};
    return v;
}

QVector<IdLabel> InterventionService::agents(int inclureId)
{
    QVector<IdLabel> v;
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT ID_AGENT, PRENOM || ' ' || NOM || ' (' || ROLE || ')' FROM AGENT "
        " WHERE STATUT IN ('Disponible', 'En service') OR ID_AGENT = :inc ORDER BY NOM"));
    q.bindValue(":inc", inclureId);
    if (q.exec())
        while (q.next())
            v << IdLabel{q.value(0).toInt(), q.value(1).toString()};
    return v;
}

QVector<IdLabel> InterventionService::equipements(int inclureId)
{
    QVector<IdLabel> v;
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT ID_EQUIPEMENT, NOM || CASE WHEN CODE IS NOT NULL THEN ' (' || CODE || ')' END FROM EQUIPEMENT "
        " WHERE STATUT = 'Disponible' OR ID_EQUIPEMENT = :inc ORDER BY NOM"));
    q.bindValue(":inc", inclureId);
    if (q.exec())
        while (q.next())
            v << IdLabel{q.value(0).toInt(), q.value(1).toString()};
    return v;
}
