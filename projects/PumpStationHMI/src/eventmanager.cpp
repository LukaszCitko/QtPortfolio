#include "eventmanager.h"

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

void EventManager::addInfo(const QString &source,
                           const QString &message)
{
    addEvent(Level::Info, source, message);
}

void EventManager::addWarning(const QString &source,
                              const QString &message)
{
    addEvent(Level::Warning, source, message);
}

void EventManager::addAlarm(const QString &source,
                            const QString &message)
{
    addEvent(Level::Alarm, source, message);
}

void EventManager::addEvent(Level level,
                            const QString &source,
                            const QString &message)
{
    m_currentLevel = level;
    m_currentSource = source;
    m_currentMessage = message;

    emit currentEventChanged();
    emit eventAdded();
}