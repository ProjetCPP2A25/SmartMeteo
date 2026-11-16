#pragma once
#include "interventionservice.h"

#include <QWidget>

// Graphique moderne des statistiques : anneau (donut) + liste classée avec barres de progression.
// Dessiné entièrement avec QPainter (aucune dépendance à Qt Charts).
class StatsChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatsChartWidget(QWidget *parent = nullptr);
    void setData(const Counts &data, const QString &title, const QString &subtitle);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    static QColor colorFor(const QString &label, int index);

    Counts m_data;
    QString m_title;
    QString m_subtitle;
};
