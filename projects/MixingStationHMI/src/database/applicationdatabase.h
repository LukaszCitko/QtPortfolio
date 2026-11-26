#ifndef APPLICATIONDATABASE_H
#define APPLICATIONDATABASE_H

#include <memory>

#include <QSqlDatabase>
#include <QString>

#include "eventhistorymodel.h"
#include "eventrepository.h"
#include "persistenceretryqueue.h"
#include "runlistmodel.h"
#include "runrepository.h"
#include "trendhistorysource.h"
#include "trendsamplerepository.h"
#include "userlistmodel.h"
#include "userrepository.h"

class ApplicationDatabase
{
public:
    static std::unique_ptr<ApplicationDatabase> open(QString &errorMessage);

    const QSqlDatabase &connection() const { return m_connection; }
    UserRepository &userRepository() { return m_userRepository; }
    UserListModel &userListModel() { return m_userListModel; }
    EventHistoryModel &eventHistoryModel() { return m_eventHistoryModel; }
    RunListModel &runListModel() { return m_runListModel; }
    TrendHistorySource &trendHistorySource() { return m_trendHistorySource; }
    PersistenceRetryQueue &retryQueue() { return m_retryQueue; }

private:
    explicit ApplicationDatabase(const QSqlDatabase &connection);

    QSqlDatabase m_connection;
    UserRepository m_userRepository;
    RunRepository m_runRepository;
    EventRepository m_eventRepository;
    TrendSampleRepository m_trendSampleRepository;
    UserListModel m_userListModel;
    EventHistoryModel m_eventHistoryModel;
    RunListModel m_runListModel;
    TrendHistorySource m_trendHistorySource;
    PersistenceRetryQueue m_retryQueue;
};

#endif