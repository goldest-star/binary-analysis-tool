#include "parameterdelegate.h"
#include <QJsonObject>
#include <QJsonArray>

ParameterDelegate::ParameterDelegate(QList<ParameterDelegate::ParameterInfo> parameters,
                                     std::function<QString(const QJsonObject&)> actionDescriber) :
    m_actionDescriber(actionDescriber)
{
    for (auto parameter : parameters) {
        m_parameterMap.insert(parameter.name, parameter);
    }
}

AbstractParameterEditor* ParameterDelegate::createEditor(QSize targetBounds)
{
    Q_UNUSED(targetBounds)
    return nullptr;
}

QList<ParameterDelegate::ParameterInfo> ParameterDelegate::parameterInfos() const
{
    return m_parameterMap.values();
}

ParameterDelegate::ParameterInfo ParameterDelegate::getInfo(QString name) const
{
    return m_parameterMap.value(name);
}

bool ParameterDelegate::validate(const QJsonObject &parameters) const
{
    return validateAgainstInfos(parameters, parameterInfos());
}

QString ParameterDelegate::actionDescription(const QJsonObject &parameters) const
{
    if (!validate(parameters)) {
        return QString();
    }

    return m_actionDescriber(parameters);
}

bool ParameterDelegate::validateAgainstInfos(const QJsonObject &parameters, QList<ParameterDelegate::ParameterInfo> infos)
{
    if (parameters.isEmpty()) {
        return false;
    }

    for (auto param : infos) {
        if (!parameters.contains(param.name)) {
            if (!param.optional) {
                return false;
            }
        }
        else if (param.type != parameters.value(param.name).type()) {
            return false;
        }

        if (param.type == QJsonValue::Array) {
            QJsonArray array = parameters.value(param.name).toArray();
            if (array.isEmpty() && !param.optional) {
                return false;
            }
            for (QJsonValueRef value: array) {
                if (!value.isObject()) {
                    return false;
                }
                if (!validateAgainstInfos(value.toObject(), param.subInfos)) {
                    return false;
                }
            }
        }
        else if (param.type == QJsonValue::Object) {
            QJsonValue val = parameters.value(param.name);
            if (!val.isObject()) {
                return false;
            }
            if (!validateAgainstInfos(val.toObject(), param.subInfos)) {
                return false;
            }
        }
    }

    return true;
}
