#include "ui/CaptionLabel.hpp"

#include <QFontMetrics>
#include <QPainter>

namespace qworldclock {

CaptionLabel::CaptionLabel(QWidget *parent)
    : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setContextMenuPolicy(Qt::NoContextMenu);
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

void CaptionLabel::setFontSize(int size) {
    const int clamped = qMax(0, size);
    if (m_fontSize != clamped) {
        m_fontSize = clamped;
        update();
    }
}

int CaptionLabel::fontSize() const {
    return m_fontSize;
}

QSize CaptionLabel::sizeHint() const {
    return {140, 48};
}

QSize CaptionLabel::minimumSizeHint() const {
    return {80, 36};
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

    // Adaptive font size calculation based on available widget width and height, or user-selected size
    int titlePixelSize = (m_fontSize > 0)
                             ? qBound(8, m_fontSize, 36)
                             : qBound(10, static_cast<int>(totalH * 0.36), 20);
    if (m_fontSize <= 0) {
        titlePixelSize = qMin(titlePixelSize, qBound(10, static_cast<int>(totalW * 0.08), 20));
    }
    const int subPixelSize = qBound(8, static_cast<int>(titlePixelSize * 0.75), 15);

    // 1. Configure Fonts and Measure Heights
    QFont titleFont = font();
    titleFont.setPixelSize(titlePixelSize);
    titleFont.setBold(true);

    QFont subFont = font();
    subFont.setPixelSize(subPixelSize);
    subFont.setBold(false);

    QFontMetrics fmTitle(titleFont);
    QFontMetrics fmSub(subFont);

    const int titleHeight = fmTitle.height();
    const int subHeight = fmSub.height();
    const int totalTextHeight = titleHeight + subHeight + 2;
    const int startY = qMax(1, (totalH - totalTextHeight) / 2);

    // 2. Draw Primary Caption (Location/Timezone)
    painter.setFont(titleFont);
    painter.setPen(QColor(0x1e, 0x29, 0x3b)); // Slate 800

    const QString elidedTitle = fmTitle.elidedText(m_caption, Qt::ElideRight, totalW - 8);
    const QRect titleRect(4, startY, totalW - 8, titleHeight);
    painter.drawText(titleRect, Qt::AlignCenter, elidedTitle);

    // 3. Draw Subtitle (Digital time, day difference, UTC offset)
    painter.setFont(subFont);
    painter.setPen(QColor(0x64, 0x74, 0x8b)); // Slate 500

    const QString subtitle = buildSubtitle();
    const QString elidedSub = fmSub.elidedText(subtitle, Qt::ElideRight, totalW - 8);
    const QRect subRect(4, titleRect.bottom() + 2, totalW - 8, subHeight);
    painter.drawText(subRect, Qt::AlignCenter, elidedSub);
}

} // namespace qworldclock
