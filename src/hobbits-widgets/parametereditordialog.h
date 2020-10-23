#ifndef PARAMETEREDITORDIALOG_H
#define PARAMETEREDITORDIALOG_H

#include <QDialog>
#include "parameterdelegate.h"
#include "hobbits-widgets_global.h"

namespace Ui {
class ParameterEditorDialog;
}

class HOBBITSWIDGETSSHARED_EXPORT ParameterEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterEditorDialog(
            QSharedPointer<ParameterDelegate> delegate,
            QJsonObject parameters = QJsonObject(),
            QWidget *parent = nullptr);
    ~ParameterEditorDialog();

    void setParameters(QJsonObject parameters);
    QJsonObject parameters();

    static QJsonObject promptForParameters(QSharedPointer<ParameterDelegate> delegate, QJsonObject parameters = QJsonObject());

private:
    Ui::ParameterEditorDialog *ui;
    AbstractParameterEditor *m_editor;
};

#endif // PARAMETEREDITORDIALOG_H
