#ifndef HISTORYPERSISTENCE_H
#define HISTORYPERSISTENCE_H

class EventManager;
class EventHistoryModel;
class PersistenceRetryQueue;
class TrendRecorder;

void connectHistoryPersistence(EventManager &events, TrendRecorder &trends, PersistenceRetryQueue &queue, EventHistoryModel &history);

#endif