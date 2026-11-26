#include "applicationoptions.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>

QString readControllerHost(const QCoreApplication &app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Mixing Station HMI"));
    parser.addHelpOption();

    const QCommandLineOption controllerHostOption(QStringLiteral("controller-host"), QStringLiteral("Connect the HMI to a Modbus TCP process controller."), QStringLiteral("address"));
    parser.addOption(controllerHostOption);
    parser.process(app);

    return parser.value(controllerHostOption).trimmed();
}