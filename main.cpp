#include "mainwindow.h"
#include <QApplication>
#include <androidrfcomm.h>
#include <QtCore/QSettings>
#include <QThread>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QNetworkSession>

int main(int argc, char *argv[])
{
    QApplication    app(argc, argv);
//  ==========================================================================
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        QNetworkSession *networkSession = new QNetworkSession(config, &app);
        networkSession->open();
        networkSession->waitForOpened();

        if (networkSession->isOpen()) {
            // Save the used configuration
            QNetworkConfiguration config = networkSession->configuration();
            QString id;
            if (config.type() == QNetworkConfiguration::UserChoice) {
                id = networkSession->sessionProperty(
                        QLatin1String("UserChoiceConfiguration")).toString();
            } else {
                id = config.identifier();
            }

            QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
            settings.beginGroup(QLatin1String("QtNetwork"));
            settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
            settings.endGroup();
        }
    }

//  ==========================================================================
    QStringList     global_devicemac;
    QStringList     global_devicename;
    const QString   MACHINENAME("RNB");
    AndroidRfComm*  rfcomm = new AndroidRfComm();

    do{

        if (rfcomm->isEnabled())
        {
            QMap<QString,QString>           map=rfcomm->getPairedDevices();
            QMapIterator<QString,QString>   iterator(map);
            while (iterator.hasNext())
            {
                iterator.next();
                QString InfosMAC = iterator.key();
                QString InfosNAME = iterator.value();
                if(InfosNAME.contains(MACHINENAME))
                {
                    global_devicename << InfosNAME;
                    global_devicemac << InfosMAC;
                    rfcomm->connect(global_devicename[0]);
                }
            }
        }
     QThread::msleep(200);
    }while (!rfcomm -> isConnected());
    MainWindow* w = new MainWindow(rfcomm);
    w -> showFullScreen();
    return app.exec();
}