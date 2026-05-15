#pragma once

#include <stb_truetype.h>
#include <cstdint>
#include <cmath>

// helpers
static uint32_t nextUtf8Codepoint(const uint8_t*& ptr, const uint8_t* end) {
    if (ptr >= end) return 0;

    uint8_t byte0 = *ptr++;
    if (byte0 < 0x80) return byte0; // ASCII

    uint32_t codepoint = 0;
    int bytesToRead = 0;

    if ((byte0 & 0xE0) == 0xC0)      { codepoint = byte0 & 0x1F; bytesToRead = 1; }
    else if ((byte0 & 0xF0) == 0xE0) { codepoint = byte0 & 0x0F; bytesToRead = 2; }
    else if ((byte0 & 0xF8) == 0xF0) { codepoint = byte0 & 0x07; bytesToRead = 3; }
    else return '?'; // incorrect UTF-8

    while (bytesToRead > 0 && ptr < end) {
        uint8_t byte = *ptr++;
        if ((byte & 0xC0) != 0x80) return '?';
        codepoint = (codepoint << 6) | (byte & 0x3F);
        bytesToRead--;
    }
    return codepoint;
}

static float calculateTextWidthPixels(const stbtt_fontinfo& fontInfo, const uint8_t* data, size_t size, float scale, bool obfuscated) {
    float width = 0;
    const uint8_t* ptr = data;
    const uint8_t* end = ptr + size;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;
        if (obfuscated) cp = 0x2022; // Bullet

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&fontInfo, cp, &advanceWidth, &leftSideBearing);
        width += std::round(advanceWidth * scale);
    }
    return width;
}