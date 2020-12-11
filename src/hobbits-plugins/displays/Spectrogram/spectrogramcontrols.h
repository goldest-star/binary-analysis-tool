#ifndef SPECTROGRAMCONTROLS_H
#define SPECTROGRAMCONTROLS_H

#include <QWidget>
#include "parameterdelegate.h"
#include "abstractparametereditor.h"
#include "parameterhelper.h"

namespace Ui
{
class SpectrogramControls;
}

class SpectrogramControls : public AbstractParameterEditor
{
    Q_OBJECT

public:
    enum DataType {
        Real = 1,
        RealComplexInterleaved = 2
    };

    SpectrogramControls(QSharedPointer<ParameterDelegate> delegate);

    QString title() override;

    virtual bool setParameters(QJsonObject parameters) override;
    virtual QJsonObject parameters() override;

signals:

private slots:

private:
    void previewBitsUiImpl(QSharedPointer<BitContainerPreview> container) override;
    Ui::SpectrogramControls *ui;
    QSharedPointer<ParameterHelper> m_paramHelper;
};

#endif // SPECTROGRAMCONTROLS_H
