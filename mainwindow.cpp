#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <string>
#include "message_types.h"

#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QTcpSocket(this))
    , model(new QStandardItemModel(this))
{
    ui->setupUi(this);


    InitializationLayout();
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->lineEdit_3, SIGNAL(returnPressed()), this, SLOT(onSendButtonClicked()));
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::provideContextMenu);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(ui->toolButton, SIGNAL(clicked()), this, SLOT(goToParentDirectory()));
    connect(ui->tableView, &QTableView::doubleClicked, this, &MainWindow::onDirectoryClicked);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTabChanged(int index)
{

    QString inputText=ui->lineEdit_4->text();
    qDebug() <<index<< "Tab changed to:" << inputText.length();
    if(index==1 && inputText.length()==0){
        socket->write("LIST:");
    }
}


void MainWindow::InitializationLayout()
{
    ui->tabWidget->addTab(ui->textEdit, "Beacon");
    ui->tabWidget->addTab(ui->tableView, "File Browser");
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // 允许单行选择
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // 选择整行
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // QStandardItemModel *model = new QStandardItemModel();
    QHeaderView *horizontalHeader = ui->tableView->horizontalHeader();
    horizontalHeader->setSectionResizeMode(QHeaderView::Stretch);

    // 设置列的标题
    model->setHorizontalHeaderLabels(QStringList() << "Type" << "Name");
    QStringList dataList = {
        "[file] Agent.exe666",
        "[file] Agent.pdb",
        "[file] Client.exe",
        "[file] Client.pdb"
    };

    for(const QString &data : dataList) {
        QString type = data.mid(data.indexOf('[') + 1, data.indexOf(']') - 1);
        QString name = data.mid(data.indexOf(']') + 2);

        typeItem = new QStandardItem(type);
        nameItem = new QStandardItem(name);

        model->appendRow(QList<QStandardItem*>() << typeItem << nameItem);
    }
    ui->tableView->setModel(model);

    ui->toolButton->setIcon(QIcon("‪D:\Download\go_parent_dir.png"));

}



void MainWindow::onConnectButtonClicked() {
      qDebug() << "Attempting to connect to server...";
    socket->connectToHost("192.168.52.132", 8080);
    if (socket->waitForConnected(3000)) {
        QMessageBox::information(this, "Connection", "Connected to server");
             socket->write("CLIENT");
    } else {
        QMessageBox::warning(this, "Connection", "Failed to connect to server");
    }

}

void MainWindow::onSendButtonClicked() {
   QString inputText = ui->lineEdit_3->text(); // 获取QLineEdit的文本并保存到变量中


   qDebug() << "Input text: " << inputText; // 打印输入的文本（可选，需要#include <QDebug>）
    logMessage(true,inputText);
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->write(inputText.toUtf8());
    } else {
        QMessageBox::warning(this, "Connection", "Not connected to server");
    }
     ui->lineEdit_3->clear(); // 清空QLineEdit的内容
}

// void MainWindow::processChunks(const QByteArray& data) {
//     int totalSize = data.size();
//     const char* ptr = data.constData();

//     while (totalSize > 0) {
//         std::vector<char> chunk_data(ptr, ptr + sizeof(Message));
//         Message msg = Message::deserialize(chunk_data);

//         if (msg.type == MessageType::END) {
//             saveToFile();
//             break;
//         }

//         if (msg.validateChecksum()) {
//             receivedChunks[msg.chunk_id] = msg.data;
//         } else {
//             qCritical() << "Checksum validation failed for chunk" << msg.chunk_id;
//         }

//         ptr += sizeof(Message);
//         totalSize -= sizeof(Message);
//     }
// }

void MainWindow::onReadyRead() {
    // qDebug() << "Raw data in hex:" << data.toHex();
    QByteArray bytes = socket->readAll();

    // QTcpSocket *clientConnection = qobject_cast<QTcpSocket *>(sender());
    // if (clientConnection) {
    //     file->write(clientConnection->readAll());
    // }

    // // 确保读取完整的消息头
    // if (socket->bytesAvailable() < static_cast<int>(sizeof(uint32_t) + sizeof(uint64_t))) {
    //     return; // 消息头不完整，等待更多数据
    // }

    // // 处理消息头
    // uint32_t type;
    // uint64_t length;

    // QDataStream socketStream(socket);
    // socketStream.setByteOrder(QDataStream::BigEndian); // 设置字节序为网络字节序

    // socketStream >> type >> length;

    // 假设您已经收集了足够的字节来构造一个完整的Message对象
    std::vector<char> data(bytes.begin(), bytes.end());
    Message msg = Message::deserialize(data);
    qDebug() << "data size:." << msg.length;
    MessageType messageType = static_cast<MessageType>(msg.type);

    switch (messageType) {
    case MessageType::Command: {
        // 处理文本消息
        qDebug() << "Command data:" << msg.data;
        qDebug() << "Command length:" << msg.length;
        std::string text(msg.data.begin(), msg.data.end());
        QString qstr = QString::fromStdString(text);

        logMessage(false, qstr);
        break;
    }
    case MessageType::Binary: {
        qDebug() << "Binary data:" << msg.data.size();

        if(!file.isOpen()) {
            file.setFileName(this->filename); // 注意：文件扩展名应为 .txt 而不是 .text
            if(file.open(QIODevice::WriteOnly)) {
                // 获取指向数据的指针并写入文件
                file.write(msg.data.data(), msg.data.size());
                file.flush(); // 确保数据写到磁盘

                file.close();
            } else {
                // 文件打开失败的处理逻辑
                qDebug() << "Unable to open file for writing.";
            }
        }

        // 这里可以将bytes保存到文件等
        break;
    }
    case MessageType::Text: {

        qDebug() << "Text data:" << msg.data;
        std::string text(msg.data.begin(), msg.data.end());

        QString utf8_str=QString::fromLocal8Bit(text);
        logMessage(false, utf8_str);
        break;
    }
    case MessageType::FileList: {
        std::string text(msg.data.begin(), msg.data.end());
        QString utf8_str=QString::fromLocal8Bit(text);
        logMessage(false, utf8_str);

        model->clear();
        model->setHorizontalHeaderLabels(QStringList() << "Type" << "Name");
        QStringList list = utf8_str.split("[");
        QString firstItem = list.first();
        QString directory = utf8_str.mid(firstItem.length());
        qDebug() << "directory_path:"<<directory;
        ui->lineEdit_4->setText(firstItem);

        QStringList lists = directory.split("\n", Qt::SkipEmptyParts);

        for(const QString &data : lists) {
            QString type = data.mid(data.indexOf('[') + 1, data.indexOf(']') - 1);
            QString name = data.mid(data.indexOf(']') + 2);
            typeItem = new QStandardItem(type);
            nameItem = new QStandardItem(name);
            model->appendRow(QList<QStandardItem*>() << typeItem << nameItem);
        }
        ui->tableView->setModel(model);
        break;
    }
    default:
        qDebug() << "Unknown message type received.";
        break;
    }

}

void MainWindow::onDirectoryClicked(const QModelIndex &index) {
    if (!index.isValid())
        return;

    QVariant data = index.siblingAtColumn(0).data(); // 0 代表第一列的列索引
    qDebug() << "Data in the double-clicked row, first column: " << index.siblingAtColumn(1).data().toString();
    if(index.siblingAtColumn(0).data()=="dir"){
        QString inputText = ui->lineEdit_4->text();
        QDir dir(inputText);

        // 使用filePath()拼接路径和字符串
        QString newPath = dir.filePath(index.siblingAtColumn(1).data().toString().trimmed());

        qDebug() << "New path:" << newPath;
        getFileSystem(true,newPath);
    }


}

void MainWindow::goToParentDirectory() {
    QString inputText = ui->lineEdit_4->text();
    qDebug() << "goToParentDirectoryh:" << inputText;
    QDir dir(inputText);
    // 跳转到父目录
    if(dir.cdUp())
    {
        // 获取父路径
        QString parentPath = dir.absolutePath();
        qDebug() << "Parent path:" << parentPath;
        ui->lineEdit_4->setText(parentPath);
        getFileSystem(true,parentPath);
    }
}

void MainWindow::getFileSystem(bool start,const QString &message) {
    if(start){
        socket->write("LIST:"+message.toUtf8());
    }

}

void MainWindow::provideContextMenu(const QPoint &position) {
    QModelIndex index = ui->tableView->indexAt(position);
    if (index.isValid()) { // 确保点击时有项被选中
        QMenu *menu = new QMenu(this);

        QAction *action1 = menu->addAction("文件上传");
        QAction *action2 = menu->addAction("文件下载");
        // 更多操作...

        // 连接相应的操作到处理函数
        connect(action1, &QAction::triggered, this, &MainWindow::uploadFile);
        connect(action2, &QAction::triggered, [this, index](){this->downloadFile(index);});
        // 更多操作...

        // 弹出上下文菜单
        menu->popup(ui->tableView->viewport()->mapToGlobal(position));
    }
}

void MainWindow::uploadFile() {
    // 使用 QFileDialog 弹出文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select File to Upload"),
                                                    QDir::homePath(), tr("All Files (*)"));
    // 检查用户是否选择了文件
    if (!filePath.isEmpty()) {
        qDebug() << "上传文件路径: " << filePath;
        // 在此处添加上传或处理文件的代码
    }
}

void MainWindow::downloadFile(const QModelIndex &index) {

    QVariant data = index.siblingAtColumn(1).data(); // 示例：获取第一列的数据
    qDebug() << "downloadfile name: " << data.toString();
    if(index.siblingAtColumn(0).data()=="file"){
        QString inputText = ui->lineEdit_4->text();
        QDir dir(inputText);
        this->filename=index.siblingAtColumn(1).data().toString().trimmed();
        // 使用filePath()拼接路径和字符串
        QString newPath = dir.filePath(index.siblingAtColumn(1).data().toString().trimmed());

        qDebug() << "New path:" << newPath;
        socket->write("FILE:"+newPath.toUtf8());

    }

}

void MainWindow::logMessage(bool isMy,const QString &message) {
    // 获取当前日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 格式化日期和时间为字符串
    QString currentDateTimeString = currentDateTime.toString("yyyy-MM-dd HH:mm:ss");
    // 创建日志消息
    QString logMessage;

    if(isMy){
        logMessage = QString("[%1] <font color='red'><b>beacon>  %2</b></font> \n").arg(currentDateTimeString, message);
        // logMessage = QString("[%1] beacon>  %2 \n").arg(currentDateTimeString,message);
    }else{
        logMessage = QString("[%1] : %2 \n").arg(currentDateTimeString,message);
    }

    ui->textEdit->append(logMessage);
}
