#include "secure_font_renderer.h"
#include "utils/widget_helpers.h"
#include <QFile>
#include <cmath>
#include <sodium.h>

bool SecureFontRenderer::loadFont(const QString& fontPath) {
    QFile fontFile(fontPath);
    if (!fontFile.open(QIODevice::ReadOnly)) return false;

    m_fontData = fontFile.readAll();
    const unsigned char* fontPtr = reinterpret_cast<const unsigned char*>(m_fontData.constData());

    if (stbtt_InitFont(&m_fontInfo, fontPtr, stbtt_GetFontOffsetForIndex(fontPtr, 0))) {
        m_fontLoaded = true;
        return true;
    }

    return false;
}

void SecureFontRenderer::setFontSize(float fontSize) {
    m_fontSize = fontSize;
}

SecureFontRenderer::FontMetrics SecureFontRenderer::getMetrics() const {
    if (!m_fontLoaded) return {0, 0, 0};
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    return { ascent * scale, descent * scale, (ascent - descent) * scale };
}

float SecureFontRenderer::calculateTextWidth(const SecureBuffer& text, size_t textLen) const {
    if (!m_fontLoaded || textLen == 0) return 0.0f;
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    float width = 0.0f;
    const uint8_t* ptr = text.data();
    const uint8_t* end = ptr + textLen;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;
        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        width += advanceWidth * scale;
    }

    return width;
}

float SecureFontRenderer::charIndexToOffset(int targetIdx, const SecureBuffer& text, size_t textLen) const {
    if (!m_fontLoaded || textLen == 0) return 0.0f;
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    float offset = 0.0f;
    int idx = 0;
    const uint8_t* ptr = text.data();
    const uint8_t* end = ptr + textLen;

    while (ptr < end && idx < targetIdx) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        offset += advanceWidth * scale;
        idx++;
    }

    return offset;
}

int SecureFontRenderer::offsetToCharIndex(float offsetX, const SecureBuffer& text, size_t textLen) const {
    if (!m_fontLoaded || textLen == 0) return 0;
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    float currentX = 0.0f;
    int charIdx = 0;
    const uint8_t* ptr = text.data();
    const uint8_t* end = ptr + textLen;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        float step = advanceWidth * scale;
        if (offsetX < currentX + step / 2.0f) return charIdx;
        currentX += step;
        charIdx++;
    }

    return charIdx;
}

void SecureFontRenderer::renderText(
    const SecureBuffer& text, size_t textLen,
    const QRect& cRect, qreal dpr,
    Qt::Alignment alignment,
    float scrollOffset,
    int selMin, int selMax,
    const QColor& normalColor, const QColor& highlightColor,
    float& outTextStartX, float& outTextStartY)
{
    if (!m_fontLoaded || cRect.isEmpty() || textLen == 0) {
        m_pixelBuffer = SecureBuffer();
        m_imageWidth = 0; m_imageHeight = 0;
        outTextStartX = 0; outTextStartY = 0;
        return;
    }

    m_imageWidth = qCeil(cRect.width() * dpr);
    m_imageHeight = qCeil(cRect.height() * dpr);
    float physScale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize * dpr);

    float physTextWidth = 0.0f;
    const uint8_t* wPtr = text.data();
    const uint8_t* wEnd = wPtr + textLen;
    while (wPtr < wEnd) {
        uint32_t cp = nextUtf8Codepoint(wPtr, wEnd);
        int aw, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &aw, &lsb);
        physTextWidth += aw * physScale;
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int physAscent = std::round(ascent * physScale);
    int physTextHeight = std::round((ascent - descent) * physScale);

    float cursorX = 0, cursorY = 0;

    if (alignment != (Qt::AlignLeft | Qt::AlignVCenter)) {
        if (alignment & Qt::AlignHCenter) cursorX = (m_imageWidth - physTextWidth) / 2.0f;
        else if (alignment & Qt::AlignRight) cursorX = m_imageWidth - physTextWidth;

        if (alignment & Qt::AlignVCenter) cursorY = (m_imageHeight - physTextHeight) / 2.0f + physAscent;
        else if (alignment & Qt::AlignBottom) cursorY = m_imageHeight - (physTextHeight - physAscent);
        else cursorY = physAscent;
    } else {
        cursorY = (m_imageHeight - physTextHeight) / 2.0f + physAscent;
    }

    cursorX -= scrollOffset * dpr;

    outTextStartX = cursorX / dpr;
    outTextStartY = (cursorY - physAscent) / dpr;

    size_t reqSize = m_imageWidth * m_imageHeight * 4;
    if (reqSize > m_pixelBufferCapacity || m_pixelBuffer.empty()) {
        m_pixelBufferCapacity = reqSize + 1024;
        m_pixelBuffer = SecureBuffer(m_pixelBufferCapacity);
    }
    sodium_memzero(m_pixelBuffer.data(), reqSize);

    uint32_t* destPixels = reinterpret_cast<uint32_t*>(m_pixelBuffer.data());
    bool hasSelection = (selMin != selMax);

    uint32_t nR = normalColor.red(), nG = normalColor.green(), nB = normalColor.blue();
    uint32_t hR = highlightColor.red(), hG = highlightColor.green(), hB = highlightColor.blue();

    int boxX0, boxY0, boxX1, boxY1;
    stbtt_GetFontBoundingBox(&m_fontInfo, &boxX0, &boxY0, &boxX1, &boxY1);
    size_t maxGlyphBytes = std::ceil((boxX1 - boxX0) * physScale) * std::ceil((boxY1 - boxY0) * physScale);
    if (m_glyphBuffer.size() < maxGlyphBytes) m_glyphBuffer = SecureBuffer(maxGlyphBytes);

    const uint8_t* ptr = text.data();
    const uint8_t* end = ptr + textLen;
    int charIdx = 0;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;

        bool isSelected = hasSelection && (charIdx >= selMin && charIdx < selMax);
        uint32_t rColor = isSelected ? hR : nR;
        uint32_t gColor = isSelected ? hG : nG;
        uint32_t bColor = isSelected ? hB : nB;

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&m_fontInfo, cp, physScale, physScale, &x0, &y0, &x1, &y1);
        int glyphWidth = x1 - x0;
        int glyphHeight = y1 - y0;

        if (glyphWidth > 0 && glyphHeight > 0) {
            if (static_cast<size_t>(glyphWidth * glyphHeight) <= m_glyphBuffer.size()) {
                stbtt_MakeCodepointBitmap(&m_fontInfo, m_glyphBuffer.data(), glyphWidth, glyphHeight, glyphWidth, physScale, physScale, cp);

                int baseDrawX = static_cast<int>(std::round(cursorX)) + x0;
                int baseDrawY = static_cast<int>(std::round(cursorY)) + y0;
                int startX = qMax(0, -baseDrawX);
                int startY = qMax(0, -baseDrawY);
                int endX = qMin(glyphWidth, m_imageWidth - baseDrawX);
                int endY = qMin(glyphHeight, m_imageHeight - baseDrawY);

                for (int y = startY; y < endY; ++y) {
                    const uint8_t* alphaRow = m_glyphBuffer.data() + (y * glyphWidth);
                    uint32_t* destRow = destPixels + ((baseDrawY + y) * m_imageWidth);
                    for (int x = startX; x < endX; ++x) {
                        uint8_t alpha = alphaRow[x];
                        if (alpha > 0) {
                            uint32_t bg = destRow[baseDrawX + x];
                            if (bg == 0) {
                                uint32_t r = (rColor * alpha) / 255;
                                uint32_t g = (gColor * alpha) / 255;
                                uint32_t b = (bColor * alpha) / 255;
                                destRow[baseDrawX + x] = (alpha << 24) | (r << 16) | (g << 8) | b;
                            } else {
                                uint32_t bgA = (bg >> 24) & 0xFF;
                                uint32_t bgR = (bg >> 16) & 0xFF;
                                uint32_t bgG = (bg >> 8) & 0xFF;
                                uint32_t bgB = bg & 0xFF;

                                uint32_t invAlpha = 255 - alpha;
                                uint32_t r = ((rColor * alpha) + bgR * invAlpha) / 255;
                                uint32_t g = ((gColor * alpha) + bgG * invAlpha) / 255;
                                uint32_t b = ((bColor * alpha) + bgB * invAlpha) / 255;
                                uint32_t a = alpha + (bgA * invAlpha) / 255;

                                destRow[baseDrawX + x] = (a << 24) | (r << 16) | (g << 8) | b;
                            }
                        }
                    }
                }
            }
        }

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &leftSideBearing);
        cursorX += advanceWidth * physScale;
        charIdx++;
    }
}