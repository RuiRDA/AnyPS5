// Derived from neonmodder123/PS5-WebKit-11.x/Source/WTF/wtf/text/TextBreakIterator.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "prx/libSceNKWebKit/include/GraphemeClusterExports.hpp"

#include <unicode/uchar.h>
#include <unicode/utf16.h>
#include <stdexcept>

namespace {

using NKWebKit::WtfAbi::StringView;

struct GraphemeProperties {
    int breakClass;
    int conjunctClass;
    bool pictographic;
};

GraphemeProperties ReadProperties(const char16_t* text, unsigned length, unsigned& offset) {
    UChar32 codePoint;
    U16_NEXT(text, offset, length, codePoint);
    return {
        u_getIntPropertyValue(codePoint, UCHAR_GRAPHEME_CLUSTER_BREAK),
        u_getIntPropertyValue(codePoint, UCHAR_INDIC_CONJUNCT_BREAK),
        static_cast<bool>(u_hasBinaryProperty(codePoint, UCHAR_EXTENDED_PICTOGRAPHIC)),
    };
}

bool IsControl(int breakClass) {
    return breakClass == U_GCB_CR || breakClass == U_GCB_LF || breakClass == U_GCB_CONTROL;
}

bool BreaksBetween(const GraphemeProperties& left, const GraphemeProperties& right,
    bool joinedConsonant, bool joinedPictograph, bool oddRegionalIndicators) {
    const int before = left.breakClass;
    const int after = right.breakClass;
    if (before == U_GCB_CR && after == U_GCB_LF) return false;
    if (IsControl(before) || IsControl(after)) return true;

    if (before == U_GCB_L && (after == U_GCB_L || after == U_GCB_V || after == U_GCB_LV || after == U_GCB_LVT))
        return false;
    if ((before == U_GCB_LV || before == U_GCB_V) && (after == U_GCB_V || after == U_GCB_T))
        return false;
    if ((before == U_GCB_LVT || before == U_GCB_T) && after == U_GCB_T)
        return false;

    if (after == U_GCB_EXTEND || after == U_GCB_ZWJ || after == U_GCB_SPACING_MARK || before == U_GCB_PREPEND)
        return false;
    if (joinedConsonant && right.conjunctClass == U_INCB_CONSONANT) return false;
    if (joinedPictograph && right.pictographic) return false;
    return !(oddRegionalIndicators && after == U_GCB_REGIONAL_INDICATOR);
}

class Utf16Clusters {
public:
    Utf16Clusters(const char16_t* text, unsigned length)
        : text_(text), length_(length), next_(ReadProperties(text_, length_, nextOffset_)) {}

    unsigned Offset() const { return offset_; }

    void Advance() {
        enum class Conjunct { None, Consonant, Linked };
        Conjunct conjunct = Conjunct::None;
        bool pictographicSequence = false;
        bool oddRegionalIndicators = false;
        while (true) {
            const auto current = next_;
            offset_ = nextOffset_;
            if (offset_ == length_) return;
            switch (current.conjunctClass) {
            case U_INCB_CONSONANT:
                conjunct = Conjunct::Consonant;
                break;
            case U_INCB_LINKER:
                if (conjunct != Conjunct::None) conjunct = Conjunct::Linked;
                break;
            case U_INCB_EXTEND:
                break;
            default:
                conjunct = Conjunct::None;
                break;
            }
            const bool joinedPictograph = pictographicSequence && current.breakClass == U_GCB_ZWJ;
            pictographicSequence = current.pictographic
                || (pictographicSequence && current.breakClass == U_GCB_EXTEND);
            oddRegionalIndicators = current.breakClass == U_GCB_REGIONAL_INDICATOR && !oddRegionalIndicators;
            next_ = ReadProperties(text_, length_, nextOffset_);
            if (BreaksBetween(current, next_, conjunct == Conjunct::Linked, joinedPictograph, oddRegionalIndicators))
                return;
        }
    }

private:
    const char16_t* text_;
    unsigned length_;
    unsigned offset_{0};
    unsigned nextOffset_{0};
    GraphemeProperties next_;
};

}

extern "C" {

unsigned APS5_VABI _ZN3WTF19numGraphemeClustersENS_10StringViewE(StringView string) {
    unsigned stringLength = string.length;
    if (!stringLength) return 0;
    if (!string.data) throw std::invalid_argument("null WTF character buffer");
    if (!string.is8Bit) {
        Utf16Clusters clusters(static_cast<const char16_t*>(string.data), stringLength);
        unsigned count = 0;
        for (; clusters.Offset() < stringLength; ++count) clusters.Advance();
        return count;
    }
    const auto* characters = static_cast<const unsigned char*>(string.data);
    unsigned numCRLF = 0;
    for (unsigned i = 1; i < stringLength; ++i) numCRLF += characters[i - 1] == '\r' && characters[i] == '\n';
    return stringLength - numCRLF;
}

unsigned APS5_VABI _ZN3WTF30numCodeUnitsInGraphemeClustersENS_10StringViewEj(StringView string, unsigned numGraphemeClusters) {
    unsigned stringLength = string.length;
    if (stringLength <= numGraphemeClusters) return stringLength;
    if (!string.data) throw std::invalid_argument("null WTF character buffer");
    if (!numGraphemeClusters) return 0;
    if (!string.is8Bit) {
        Utf16Clusters clusters(static_cast<const char16_t*>(string.data), stringLength);
        for (unsigned i = 0; i < numGraphemeClusters && clusters.Offset() < stringLength; ++i) clusters.Advance();
        return clusters.Offset();
    }
    const auto* characters = static_cast<const unsigned char*>(string.data);
    unsigned i, j;
    for (i = 0, j = 0; i < numGraphemeClusters && j + 1 < stringLength; ++i, ++j) j += characters[j] == '\r' && characters[j + 1] == '\n';
    return j + (i < numGraphemeClusters);
}

}
