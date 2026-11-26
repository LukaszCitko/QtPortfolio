#include "historypersistence.h"

#include "eventhistorymodel.h"
#include "eventmanager.h"
#include "persistenceretryqueue.h"
#include "trendrecorder.h"

#include <QObject>

void connectHistoryPersistence(EventManager &events, TrendRecorder &trends, PersistenceRetryQueue &queue, EventHistoryModel &history)
{
    QObject::connect(&queue, &PersistenceRetryQueue::eventSaved, &history, &EventHistoryModel::prependSavedEvent);
    QObject::connect(&events, &EventManager::eventRecorded, &queue, &PersistenceRetryQueue::submitEvent);
    QObject::connect(&trends, &TrendRecorder::sampleRecorded, &queue, &PersistenceRetryQueue::submitSample);
}