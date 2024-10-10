#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QStandardItemModel>
#include <QFileDialog>
// #include <QTextCodec>
#include "LineEditDelegate.h"
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
    // void processChunks(const QByteArray& data);


private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QStandardItemModel *model;
    QStandardItem *typeItem ;
    QStandardItem *nameItem;
    QFile file;
    QString filename;


signals:
    void logMessageSignal(const QString &message);
};
#endif // MAINWINDOW_H
