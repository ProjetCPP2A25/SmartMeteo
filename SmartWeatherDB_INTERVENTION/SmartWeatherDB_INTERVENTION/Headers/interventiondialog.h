#pragma once
#include "intervention.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class InterventionDialog; }
QT_END_NAMESPACE

// Formulaire d'ajout / modification d'une intervention.
// Le formulaire ENREGISTRE lui-même dans la base : en cas d'erreur il reste ouvert
// et l'utilisateur ne perd pas sa saisie. L'appelant n'a donc plus qu'à rafraîchir.
class InterventionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InterventionDialog(int idIntervention = 0, QWidget *parent = nullptr);
    ~InterventionDialog() override;

    Intervention result() const { return m_result; }

private slots:
    void onSave();

private:
    Ui::InterventionDialog *ui;
    int m_id;
    Intervention m_result;
};
