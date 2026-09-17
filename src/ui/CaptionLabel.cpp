#include "ui/CaptionLabel.hpp"

#include <QFontMetrics>
#include <QPainter>

namespace qworldclock {

CaptionLabel::CaptionLabel(QWidget *parent)
    : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void CaptionLabel::setCaption(const QString &caption) {
    if (m_caption != caption) {
        m_caption = caption;
        update();
    }
}

QString CaptionLabel::caption() const {
    return m_caption;
}

void CaptionLabel::setTimeZone(const QTimeZone &timeZone) {
    if (m_timeZone != timeZone) {
        m_timeZone = timeZone;
        update();
    }
}

QTimeZone CaptionLabel::timeZone() const {
    return m_timeZone;
}

void CaptionLabel::setTime(const QDateTime &utcNow) {
    m_currentUtcTime = utcNow;
    update();
}

QSize CaptionLabel::sizeHint() const {
    return {180, 48};
}

QSize CaptionLabel::minimumSizeHint() const {
    return {80, 32};
}

QString CaptionLabel::buildSubtitle() const {
    const QDateTime targetTime = m_currentUtcTime.toTimeZone(m_timeZone);
    const QDate sysDate = m_currentUtcTime.toLocalTime().date();
    const QDate targetDate = targetTime.date();

    QString dayDiff;
    const qint64 dayDelta = sysDate.daysTo(targetDate);
    if (dayDelta > 0) {
        dayDiff = QStringLiteral("+%1d · ").arg(dayDelta);
    } else if (dayDelta < 0) {
        dayDiff = QStringLiteral("%1d · ").arg(dayDelta);
    }

    const QString timeStr = targetTime.toString(QStringLiteral("hh:mm:ss"));

    // UTC offset format: e.g. UTC+1 or UTC-5
    const int offsetSec = m_timeZone.offsetFromUtc(m_currentUtcTime);
    const int offsetHours = offsetSec / 3600;
    const int offsetMins = qAbs((offsetSec % 3600) / 60);

    QString offsetStr = QStringLiteral("UTC");
    if (offsetHours >= 0) {
        offsetStr += QStringLiteral("+%1").arg(offsetHours);
    } else {
        offsetStr += QStringLiteral("%1").arg(offsetHours);
    }
    if (offsetMins > 0) {
        offsetStr += QStringLiteral(":%1").arg(offsetMins, 2, 10, QLatin1Char('0'));
    }

    return QStringLiteral("%1%2 (%3)").arg(dayDiff, timeStr, offsetStr);
}

void CaptionLabel::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    const int totalH = height();
    const int totalW = width();
    if (totalH < 16 || totalW < 20) {
        return;
    }

    // Adaptive font size calculation based on available widget height and width
    const int maxDim = qMin(totalW / 8, totalH);
    const int titlePixelSize = qBound(10, static_cast<int>(maxDim * 0.38), 20);
    const int subPixelSize = qBound(8, static_cast<int>(maxDim * 0.28), 13);

    // 1. Draw Primary Caption (Location/Timezone)
    QFont titleFont = font();
    titleFont.setPixelSize(titlePixelSize);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(0x1e, 0x29, 0x3b)); // Slate 800

    QFontMetrics fmTitle(titleFont);
    const QString elidedTitle = fmTitle.elidedText(m_caption, Qt::ElideRight, totalW - 8);
    const int titleHeight = fmTitle.height();

    const QRect titleRect(4, 2, totalW - 8, titleHeight);
    painter.drawText(titleRect, Qt::AlignCenter, elidedTitle);

    // 2. Draw Subtitle (Digital time, day difference, UTC offset)
    QFont subFont = font();
    subFont.setPixelSize(subPixelSize);
    subFont.setBold(false);
    painter.setFont(subFont);
    painter.setPen(QColor(0x64, 0x74, 0x8b)); // Slate 500

    QFontMetrics fmSub(subFont);
    const QString subtitle = buildSubtitle();
    const QString elidedSub = fmSub.elidedText(subtitle, Qt::ElideRight, totalW - 8);
    const int subHeight = fmSub.height();

    const QRect subRect(4, titleRect.bottom() + 1, totalW - 8, subHeight);
    painter.drawText(subRect, Qt::AlignCenter, elidedSub);
}

} // namespace qworldclock
