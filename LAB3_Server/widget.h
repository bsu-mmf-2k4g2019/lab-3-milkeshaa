#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QVector>
#include <QDataStream>
#include <QTcpServer>
#include <QQueue>

// int is used for this constants
#define READ_FORTUNE_MARKER (0u)
#define WRITE_FORTUNE_MARKER (1u)
#define NO_TRANSACTION_TYPE (-1)

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void sendMessages(QTcpSocket*);
    void hanleNewConnection();
    void hanleReadyRead();
    void dropClient(QTcpSocket *client);

private:
    QLabel *statusLabel = nullptr;
    QTcpServer *tcpServer = nullptr;
    QVector<QString> fortunes;
    QVector<QString> messages;
    QQueue<QTcpSocket*> users;

    QDataStream in;
    int trType = NO_TRANSACTION_TYPE;
};

#endif // WIDGET_H
