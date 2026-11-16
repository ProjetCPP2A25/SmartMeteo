#pragma once
#include "interventionservice.h"

#include <QWidget>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QStandardItemModel;
class QTableView;
class StatsChartWidget;

// Page « Gestion des interventions » : indicateurs, onglets Liste / Statistiques,
// métiers (Estimer la durée, Détecter les conflits) affichés en haut, à côté des onglets.
class InterventionWidget : public QWidget {
    Q_OBJECT
public:
    explicit InterventionWidget(QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onCancel();
    void onClose();
    void onEstimate();
    void onConflicts();
    void exportCsv();
    void exportPdf();
    void refreshStats();

private:
    void buildUi();
    InterventionFilter currentFilter() const;
    int selectedId();
    void populateTable(const QVector<Intervention> &rows);
    void notify(const QString &text);

    // indicateurs du haut
    QLabel *m_kTotal = nullptr, *m_kEnCours = nullptr, *m_kRetard = nullptr, *m_kCritiques = nullptr;

    // liste
    QLineEdit *m_search = nullptr;
    QComboBox *m_fStatut = nullptr, *m_fPriorite = nullptr, *m_fType = nullptr, *m_fZone = nullptr;
    QCheckBox *m_fPeriode = nullptr;
    QDateEdit *m_fDu = nullptr, *m_fAu = nullptr;
    QTableView *m_table = nullptr;
    QStandardItemModel *m_model = nullptr;
    QLabel *m_msg = nullptr;

    // statistiques
    QButtonGroup *m_dimGroup = nullptr;
    QCheckBox *m_sPeriode = nullptr;
    QDateEdit *m_sDu = nullptr, *m_sAu = nullptr;
    StatsChartWidget *m_chart = nullptr;
    QLabel *m_cTotal = nullptr, *m_cRate = nullptr, *m_cLate = nullptr, *m_cDuree = nullptr;

    QVector<Intervention> m_shown;   // lignes affichées = lignes exportées
};
