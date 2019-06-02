#include "widget.h"

#include <QDebug>
#include <QTcpSocket>
#include <QNetworkInterface>

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRandomGenerator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    qDebug() << "Server constructor is called";
    statusLabel = new QLabel();
    statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen()) {
        qDebug() << "Unable to make server listen";
        statusLabel->setText(QString("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    statusLabel->setText(QString("The server is running on\n\nIP: %1\nport: %2\n\n"
                            "Run the Fortune Client example now.")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
    qDebug() << "Start server on: " << ipAddress << ":" << tcpServer->serverPort();

    auto quitButton = new QPushButton(tr("Quit"));
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(tcpServer, &QTcpServer::newConnection, this, &Widget::hanleNewConnection);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    in.setVersion(QDataStream::Qt_4_0);
}

Widget::~Widget()
{

}

void Widget::sendMessages(QTcpSocket* client)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    if (!messages.empty()) {
        out << messages.size();
        for (int i = 0; i < 50 && i < messages.size(); i++) {
            out << messages[i];
        }
    } else {
        out << "No messages right now";
        qDebug() << "No messages right now";
    }

    client->write(block);
    client->flush();

//    dropClient(clientConnection);
}

void Widget::hanleNewConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    in.setDevice(clientConnection);
    users.push_back(clientConnection);
    if (!messages.empty()) {
        sendMessages(clientConnection);
    }
    connect(clientConnection, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
}

void Widget::hanleReadyRead()
{
    qDebug() << "Read fortune is called";
        QString message;

        // Read fortune from client
        in.startTransaction();
        in >> message;
        if (message != "") {
            if (message == "Disconnect") {
                dropClient(dynamic_cast<QTcpSocket*>(sender()));
                return;
            }
            if (!in.commitTransaction())
                return;

            qDebug() << "Message: " << message;

            if (messages.size() > 50) {
                 messages.clear();
            }
            messages.push_back(message);
        }

//        dropClient(dynamic_cast<QTcpSocket*>(sender()));
}

void Widget::dropClient(QTcpSocket *client)
{
    int index = users.indexOf(client);
    users.remove(index);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << "YOU ARE DISCONNECTED! CLICK ON CONNECT TO RECONNECT";
    client->write(block);
    client->flush();

    disconnect(client, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
    connect(client, &QAbstractSocket::disconnected,
            client, &QObject::deleteLater);
    client->disconnectFromHost();
}
