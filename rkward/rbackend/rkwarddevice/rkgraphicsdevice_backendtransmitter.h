#include <QAbstractSocket>

QByteArray buffer;
QDataStream protocol (&buffer);
QAbstractSocket* connection;
QMutex rkwarddeviceprotocolmutex;
 
