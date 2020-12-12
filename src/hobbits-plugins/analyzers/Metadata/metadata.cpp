#include "metadata.h"
#include "metadataform.h"

Metadata::Metadata()
{
    QList<ParameterDelegate::ParameterInfo> infos = {
        {"label", QJsonValue::String},
        {"contents", QJsonValue::String}
    };

    m_delegate = ParameterDelegate::create(
                infos,
                [](const QJsonObject &parameters) {
        QString label = parameters.value("label").toString();
        return QString("Set Metadata Field '%1'").arg(label);
    },
    [](QSharedPointer<ParameterDelegate> delegate, QSize size) {
            Q_UNUSED(size)
            return new MetadataForm(delegate);
});
}

AnalyzerInterface* Metadata::createDefaultAnalyzer()
{
    return new Metadata();
}

QString Metadata::name()
{
    return "Metadata";
}

QString Metadata::description()
{
    return "Displays and allows editing of container metadata values";
}

QStringList Metadata::tags()
{
    return {"Generic"};
}

QSharedPointer<ParameterDelegate>  Metadata::parameterDelegate()
{
    return m_delegate;
}

QSharedPointer<const AnalyzerResult> Metadata::analyzeBits(
        QSharedPointer<const BitContainer> container,
        const QJsonObject &parameters,
        QSharedPointer<PluginActionProgress> progress)
{
    Q_UNUSED(progress)
    if (!m_delegate->validate(parameters)) {
        return AnalyzerResult::error(QString("Invalid parameters passed to %1").arg(name()));
    }

    QString label = parameters.value("label").toString();
    QString contents = parameters.value("contents").toString();

    auto info = BitInfo::copyFromContainer(container);
    info->setMetadata(label, contents);

    return AnalyzerResult::result(info, parameters);
}
