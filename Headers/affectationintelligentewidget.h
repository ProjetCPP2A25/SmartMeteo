#ifndef AFFECTATIONINTELLIGENTEWIDGET_H
#define AFFECTATIONINTELLIGENTEWIDGET_H

#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QStandardItemModel;
class QTableView;

class AffectationIntelligenteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AffectationIntelligenteWidget(QWidget *parent = nullptr);

private slots:
    void chargerInterventions();
    void analyserDisponibilites();
    void affecterAgentSelectionne();

private:
    QComboBox *m_interventionCombo = nullptr;
    QTableView *m_agentsTable = nullptr;
    QStandardItemModel *m_agentsModel = nullptr;
    QLabel *m_info = nullptr;
    QPushButton *m_btnAffecter = nullptr;
};

#endif // AFFECTATIONINTELLIGENTEWIDGET_H
