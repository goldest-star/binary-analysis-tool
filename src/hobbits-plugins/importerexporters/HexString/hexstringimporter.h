#ifndef HEXSTRINGIMPORTER_H
#define HEXSTRINGIMPORTER_H

#include "bitcontainer.h"
#include <QDialog>

namespace Ui
{
class HexStringImporter;
}

class HexStringImporter : public QDialog
{
    Q_OBJECT

public:
    explicit HexStringImporter(QWidget *parent = nullptr);

    ~HexStringImporter();

    void importFromFile(QString fileName);
    void importFromHexString(QString hexString, int repeats);
    QSharedPointer<BitContainer> getContainer() const;
    QString getFileName() const;
    QString getHexString() const;
    int getRepeats() const;

private slots:
    void on_te_hexString_textChanged();
    void on_pb_selectFile_pressed();
    void on_pb_submitInput_pressed();

    void on_cb_repeat_toggled(bool checked);

private:
    Ui::HexStringImporter *ui;

    QSharedPointer<BitContainer> m_container;
    QString m_fileName;
};

#endif // HEXSTRINGIMPORTER_H
