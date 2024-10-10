#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H
#include <QMainWindow>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QLineEdit>


class LineEditDelegate : public QStyledItemDelegate {
public:
    LineEditDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) { }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        Q_UNUSED(option)
        Q_UNUSED(index)

        QLineEdit *editor = new QLineEdit(parent);

        // 连接QLineEdit的textChanged信号以便监听文本变化
        connect(editor, &QLineEdit::textChanged, [index](const QString &text){
            // 在这里处理文本变化，例如打印变化的文本
            qDebug() << "Text changed in row" << index.row() << ": " << text;
        });

        return editor;
    }
};



#endif // LINEEDITDELEGATE_H
