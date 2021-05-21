#include "byteraster.h"
#include "byterastercontrols.h"
#include "displayhelper.h"
#include <QPainter>
#include <QtMath>
#include "displayresult.h"

ByteRaster::ByteRaster() :
    m_renderConfig(new DisplayRenderConfig())
{
    m_renderConfig->setFullRedrawTriggers(DisplayRenderConfig::NewBitOffset | DisplayRenderConfig::NewFrameOffset);
    m_renderConfig->setOverlayRedrawTriggers(DisplayRenderConfig::NewBitHover);

    ParameterDelegate::ParameterInfo scaleParam = {"scale", ParameterDelegate::ParameterType::Integer};
    scaleParam.ranges.append({1, 128});
    QList<ParameterDelegate::ParameterInfo> infos = {
        scaleParam,
        {"show_headers", ParameterDelegate::ParameterType::Boolean}
    };

    m_delegate = ParameterDelegate::create(
                    infos,
                    [](const Parameters &parameters) {
                        int scale = parameters.value("scale").toInt();
                        if (parameters.value("show_headers").toBool()) {
                            return QString("Byte Raster %1x with headers").arg(scale);
                        }
                        else {
                            return QString("Byte Raster %1x without headers").arg(scale);
                        }
                    },
                    [](QSharedPointer<ParameterDelegate> delegate, QSize size) {
                        Q_UNUSED(size)
                        return new ByteRasterControls(delegate);
                    });
}

DisplayInterface* ByteRaster::createDefaultDisplay()
{
    return new ByteRaster();
}

QString ByteRaster::name()
{
    return "Byte Raster";
}

QString ByteRaster::description()
{
    return "Displays each byte in the data as a single colored pixel";
}

QStringList ByteRaster::tags()
{
    return {"Generic"};
}

QSharedPointer<DisplayRenderConfig> ByteRaster::renderConfig()
{
    return m_renderConfig;
}

void ByteRaster::setDisplayHandle(QSharedPointer<DisplayHandle> displayHandle)
{
    m_handle = displayHandle;
    DisplayHelper::connectHoverUpdates(this, this, m_handle, [this](QPoint& offset, QSize &symbolSize, int &grouping, int &bitsPerSymbol) {
        if (!m_delegate->validate(m_lastParams).isEmpty()) {
            return false;
        }
        int scale = m_lastParams.value("scale").toInt();
        offset = headerOffset(m_lastParams);
        symbolSize = QSize(scale, scale);
        grouping = 1;
        bitsPerSymbol = 8;
        return true;
    });
}

QSharedPointer<ParameterDelegate> ByteRaster::parameterDelegate()
{
    return m_delegate;
}

QSharedPointer<DisplayResult> ByteRaster::renderDisplay(QSize viewportSize, const Parameters &parameters, QSharedPointer<PluginActionProgress> progress)
{
    Q_UNUSED(progress)
    m_lastParams = parameters;

    QStringList invalidations = m_delegate->validate(parameters);
    if (!invalidations.isEmpty()) {
        m_handle->setRenderedRange(this, Range());
        return DisplayResult::error(QString("Invalid parameters passed to %1:\n%2").arg(name()).arg(invalidations.join("\n")));
    }
    if (m_handle.isNull() || m_handle->currentContainer().isNull()) {
        m_handle->setRenderedRange(this, Range());
        return DisplayResult::nullResult();
    }

    int scale = parameters.value("scale").toInt();
    QPoint offset = headerOffset(parameters);
    QSize rasterSize(viewportSize.width() - offset.x(), viewportSize.height() - offset.y());
    QSize sourceSize(qMax(1, rasterSize.width() / scale), qMax(1, rasterSize.height() / scale));

    QImage raster = DisplayHelper::getByteRasterImage(
                m_handle->currentContainer(),
                m_handle->bitOffset(),
                m_handle->frameOffset(),
                sourceSize.width(),
                sourceSize.height());

    QImage destImage(viewportSize, QImage::Format_ARGB32);
    destImage.fill(Qt::transparent);
    QPainter painter(&destImage);
    painter.translate(offset);
    painter.scale(scale, scale);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.drawImage(0, 0, raster);

    painter.resetTransform();
    painter.translate(offset);
    QSize highlightSize(sourceSize.width() * 8, sourceSize.height());
    DisplayHelper::drawHighlights(
                m_handle,
                &painter,
                QSizeF(double(scale) / 8.0, scale),
                highlightSize,
                (m_handle->bitOffset() / 8) * 8,
                m_handle->frameOffset(),
                1);

    DisplayHelper::setRenderRange(this, m_handle, sourceSize.height());

    return DisplayResult::result(destImage, parameters);
}

QSharedPointer<DisplayResult> ByteRaster::renderOverlay(QSize viewportSize, const Parameters &parameters)
{
    m_lastParams = parameters;
    QStringList invalidations = m_delegate->validate(parameters);
    if (!invalidations.isEmpty()) {
        return DisplayResult::error(QString("Invalid parameters passed to %1:\n%2").arg(name()).arg(invalidations.join("\n")));
    }
    int scale = parameters.value("scale").toInt();

    auto overlay = DisplayHelper::drawHeadersFull(
                viewportSize,
                headerOffset(parameters),
                m_handle,
                QSizeF(double(scale) / 8.0, scale));

    return DisplayResult::result(overlay, parameters);
}

QPoint ByteRaster::headerOffset(const Parameters &parameters)
{
    if (!parameters.value("show_headers").toBool() || m_handle->currentContainer().isNull()) {
        return QPoint(0, 0);
    }

    auto font = DisplayHelper::monoFont(10);
    auto container = m_handle->currentContainer();
    auto margin = DisplayHelper::textSize(font, "0").width() * 2;
    return  QPoint(
                DisplayHelper::textSize(font, container->frameCount()).width() + margin,
                DisplayHelper::textSize(font, container->maxFrameWidth()).width() + margin);
}
