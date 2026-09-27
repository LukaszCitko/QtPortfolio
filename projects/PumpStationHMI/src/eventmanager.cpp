#include "eventmanager.h"
#include <QDateTime>

EventManager::EventManager(QObject *parent)
    : QObject(parent)
{
}

QString EventManager::currentMessage() const
{
    return m_currentMessage;
}

QString EventManager::currentSource() const
{
    return m_currentSource;
}

EventManager::Level EventManager::currentLevel() const
{
    return m_currentLevel;
}

void EventManager::addInfo(const QString &source, const QString &message)
{
    addEvent(Level::Info, source, message);
}

void EventManager::addWarning(const QString &source, const QString &message)
{
    addEvent(Level::Warning, source, message);
}

void EventManager::addAlarm(const QString &source, const QString &message)
{
    addEvent(Level::Alarm, source, message);
}

void EventManager::addEvent(Level level, const QString &source, const QString &message)
{
    const qint64 timestampMs = QDateTime::currentMSecsSinceEpoch();
    const QString displayTime = QDateTime::fromMSecsSinceEpoch(timestampMs).toString("yyyy-MM-dd HH:mm:ss");

    m_events.append({displayTime, level, source, message, m_activeBatchId});

    m_currentLevel = level;
    m_currentSource = source;
    m_currentMessage = message;

    emit currentEventChanged();
    emit eventAdded();

    emit eventRecorded(m_activeBatchId, timestampMs, currentLevelText(), source, message);
}

QString EventManager::currentLevelText() const
{
    switch (m_currentLevel)
    {
    case Level::Info:
        return "INFO";
    case Level::Warning:
        return "WARNING";
    case Level::Alarm:
        return "ALARM";
    }
    return "UNKNOWN";
}

int EventManager::eventCount() const
{
    return m_events.size();
}

QString EventManager::eventTime(int index) const
{
    return index >= 0 && index < m_events.size()
    ? m_events.at(index).time : QString();
}

QString EventManager::eventLevel(int index) const
{
    if (index < 0 || index >= m_events.size())
        return QString();

    switch (m_events.at(index).level)
    {
    case Level::Info:    return "INFO";
    case Level::Warning: return "WARNING";
    case Level::Alarm:   return "ALARM";
    }

    return QString();
}

QString EventManager::eventSource(int index) const
{
    return index >= 0 && index < m_events.size()
    ? m_events.at(index).source : QString();
}

QString EventManager::eventMessage(int index) const
{
    return index >= 0 && index < m_events.size()
    ? m_events.at(index).message : QString();
}

void EventManager::setBatchId(const QString &batchId)
{
    m_activeBatchId = batchId;
}

QString EventManager::eventBatchId(int index) const
{
    return index >= 0 && index < m_events.size()
    ? m_events.at(index).batchId : QString();
}

