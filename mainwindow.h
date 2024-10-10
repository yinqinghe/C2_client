#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QThread>
#include <QDataStream>
#include <QDir>
#include <QIcon>
#include <QFile>
// 在发送方和接收方的源文件中
#include "Message_types.h"

#include <Qstring>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void logMessage(bool,const QString &message);
    void onConnectButtonClicked();
    void onSendButtonClicked();
    void onReadyRead();
    void getFileSystem(bool,const QString &message);
    void uploadFile();
    void downloadFile(const QModelIndex &index);
    void provideContextMenu(const QPoint &position);
    void InitializationLayout();
    void onTabChanged(int);
    void goToParentDirectory();
    void onDirectoryClicked(const QModelIndex &index);
    void processChunks(const QByteArray& data);
    void saveToFile();
    void switchType(MessageType messageType);


private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QStandardItemModel *model;
    QStandardItem *typeItem ;
    QStandardItem *nameItem;
    QFile file;
    QString filename;
    std::unordered_map<int, std::vector<char>> receivedChunks;
    quint32 currentBlockSize = 0; // 当前数据块大小
    QByteArray buffer;
    uint32_t chunk_id=0;
    uint32_t filenameLength=0;
    uint32_t length; // 消息内容的长度
    std::vector<char> *data; // 消息内容
    Message msg ;
    int msg_length;
    bool isFile=false;
    bool expectingNewPacket=true;


signals:
    void logMessageSignal(const QString &message);
};
#endif // MAINWINDOW_H
