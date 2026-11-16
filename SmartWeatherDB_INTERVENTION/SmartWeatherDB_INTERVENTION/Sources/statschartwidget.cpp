#include "statschartwidget.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <utility>

StatsChartWidget::StatsChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(300);
}

void StatsChartWidget::setData(const Counts &data, const QString &title, const QString &subtitle)
{
    m_data = data;
    m_title = title;
    m_subtitle = subtitle;
    updateGeometry();
    update();
}

QSize StatsChartWidget::sizeHint() const { return {760, 320}; }

QSize StatsChartWidget::minimumSizeHint() const
{
    return {420, qMax(300, 80 + int(m_data.size()) * 48)};
}

QColor StatsChartWidget::colorFor(const QString &label, int index)
{
    // Couleurs « sémantiques » pour les statuts et les priorités, palette pour le reste.
    if (label == QStringLiteral("Planifiée")) return QColor("#6CB4E4");
    if (label == QStringLiteral("En cours"))  return QColor("#19C6B7");
    if (label == QStringLiteral("Terminée"))  return QColor("#409A71");
    if (label == QStringLiteral("Annulée"))   return QColor("#7A8A93");
    if (label == QStringLiteral("Faible"))    return QColor("#409A71");
    if (label == QStringLiteral("Moyenne"))   return QColor("#C6A555");
    if (label == QStringLiteral("Haute"))     return QColor("#E49740");
    if (label == QStringLiteral("Critique"))  return QColor("#CC4E4F");
    static const QColor palette[] = {QColor("#19C6B7"), QColor("#347EA7"), QColor("#E49740"), QColor("#8B6FD6"),
                                     QColor("#CC4E4F"), QColor("#409A71"), QColor("#C6A555"), QColor("#6CB4E4")};
    return palette[index % 8];
}

void StatsChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor textC("#E6F1F5"), mutedC("#8FB3C2"), trackC("#143A4D");

    // La feuille de style fixe la taille en pixels : on travaille donc en pixels (pointSizeF() vaudrait -1).
    auto makeFont = [this](int px, bool bold) {
        QFont f = font();
        f.setPixelSize(px);
        f.setBold(bold);
        return f;
    };
    const QFont normalF = makeFont(13, false);
    const QFont boldF = makeFont(13, true);

    // ----- Titre -----
    p.setFont(makeFont(17, true));
    p.setPen(Qt::white);
    p.drawText(QRect(24, 14, width() - 48, 26), Qt::AlignLeft | Qt::AlignVCenter, m_title);
    p.setFont(normalF);
    p.setPen(mutedC);
    p.drawText(QRect(24, 40, width() - 48, 20), Qt::AlignLeft | Qt::AlignVCenter, m_subtitle);

    if (m_data.isEmpty()) {
        p.setPen(mutedC);
        p.drawText(rect(), Qt::AlignCenter, tr("Aucune donnée pour cette sélection"));
        return;
    }

    int total = 0, maxV = 1;
    for (const auto &d : std::as_const(m_data)) {
        total += d.second;
        maxV = std::max(maxV, d.second);
    }
    if (total <= 0) total = 1;

    const int top = 78;
    const bool wide = width() >= 600;

    // ----- Anneau (donut) -----
    int listX = 24;
    if (wide) {
        const qreal d = qBound<qreal>(150, qMin<qreal>(height() - top - 24, 240), 240);
        const QRectF outer(24, top + 8, d, d);
        const qreal thick = d * 0.20;
        const QRectF inner = outer.adjusted(thick, thick, -thick, -thick);

        // piste de fond
        QPainterPath track;
        track.addEllipse(outer);
        track.addEllipse(inner);
        p.fillPath(track, trackC);

        qreal start = 90.0;   // départ en haut, sens horaire
        for (int i = 0; i < m_data.size(); ++i) {
            const qreal frac = qreal(m_data[i].second) / total;
            qreal span = -frac * 360.0;
            qreal a0 = start;
            if (m_data.size() > 1 && -span > 4.0) {   // petit espace entre les segments
                a0 -= 0.8;
                span += 1.6;
            }
            QPainterPath seg;
            seg.arcMoveTo(outer, a0);
            seg.arcTo(outer, a0, span);
            seg.arcTo(inner, a0 + span, -span);
            seg.closeSubpath();
            p.fillPath(seg, colorFor(m_data[i].first, i));
            start += -frac * 360.0;
        }

        // total au centre
        p.setFont(makeFont(36, true));
        p.setPen(Qt::white);
        p.drawText(QRectF(inner.left(), inner.top() + inner.height() * 0.22, inner.width(), inner.height() * 0.4),
                   Qt::AlignCenter, QString::number(total));
        p.setFont(normalF);
        p.setPen(mutedC);
        p.drawText(QRectF(inner.left(), inner.top() + inner.height() * 0.56, inner.width(), inner.height() * 0.2),
                   Qt::AlignCenter, tr("interventions"));

        listX = int(outer.right()) + 44;
    }

    // ----- Liste classée avec barres de progression -----
    const int listW = std::max(200, width() - listX - 24);
    const int rowH = 48;
    const QFontMetrics fm(normalF);
    p.setFont(normalF);

    for (int i = 0; i < m_data.size(); ++i) {
        const int y = top + 4 + i * rowH;
        const QColor col = colorFor(m_data[i].first, i);
        const int value = m_data[i].second;

        // pastille de couleur
        p.setPen(Qt::NoPen);
        p.setBrush(col);
        p.drawEllipse(QRectF(listX, y + 6, 12, 12));

        // libellé
        const int labelW = listW - 22 - 120;
        p.setPen(textC);
        p.drawText(QRect(listX + 22, y, labelW, 24), Qt::AlignLeft | Qt::AlignVCenter,
                   fm.elidedText(m_data[i].first, Qt::ElideRight, labelW));

        // valeur + pourcentage
        p.setFont(boldF);
        p.setPen(Qt::white);
        p.drawText(QRect(listX + listW - 120, y, 56, 24), Qt::AlignRight | Qt::AlignVCenter, QString::number(value));
        p.setFont(normalF);
        p.setPen(mutedC);
        p.drawText(QRect(listX + listW - 56, y, 56, 24), Qt::AlignRight | Qt::AlignVCenter,
                   QStringLiteral("%1 %").arg(qRound(100.0 * value / total)));

        // barre de progression arrondie
        const qreal barX = listX + 22;
        const qreal barW = listW - 22;
        p.setPen(Qt::NoPen);
        p.setBrush(trackC);
        p.drawRoundedRect(QRectF(barX, y + 28, barW, 8), 4, 4);
        p.setBrush(col);
        p.drawRoundedRect(QRectF(barX, y + 28, std::max<qreal>(8.0, barW * value / maxV), 8), 4, 4);
    }
}
