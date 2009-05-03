/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef Lexer_h
#define Lexer_h

#include "Lookup.h"
#include "SegmentedVector.h"
#include "SourceCode.h"
#include <wtf/ASCIICType.h>
#include <wtf/Vector.h>
#include <wtf/unicode/Unicode.h>

namespace JSC {

    class RegExp;

    class Lexer : Noncopyable {
    public:
        // Character manipulation functions.
        static bool isWhiteSpace(int character);
        static bool isLineTerminator(int character);
        static unsigned char convertHex(int c1, int c2);
        static UChar convertUnicode(int c1, int c2, int c3, int c4);

        // Functions to set up parsing.
        void setCode(const SourceCode&);
        void setIsReparsing() { m_isReparsing = true; }

        // Functions for the parser itself.
        int lex(void* lvalp, void* llocp);
        int lineNumber() const { return m_lineNumber; }
        bool prevTerminator() const { return m_terminator; }
        SourceCode sourceCode(int openBrace, int closeBrace, int firstLine);
        bool scanRegExp();
        const UString& pattern() const { return m_pattern; }
        const UString& flags() const { return m_flags; }

        // Functions for use after parsing.
        bool sawError() const { return m_error; }
        void clear();

    private:
        friend class JSGlobalData;

        Lexer(JSGlobalData*);
        ~Lexer();

        enum State {
            Start,
            IdentifierOrKeyword,
            Identifier,
            InIdentifierOrKeyword,
            InIdentifier,
            InIdentifierStartUnicodeEscapeStart,
            InIdentifierStartUnicodeEscape,
            InIdentifierPartUnicodeEscapeStart,
            InIdentifierPartUnicodeEscape,
            InSingleLineComment,
            InMultiLineComment,
            InNum,
            InNum0,
            InHex,
            InOctal,
            InDecimal,
            InExponentIndicator,
            InExponent,
            Hex,
            Octal,
            Number,
            String,
            Eof,
            InString,
            InEscapeSequence,
            InHexEscape,
            InUnicodeEscape,
            Other,
            Bad
        };

        void setDone(State);
        void shift1();
        void shift2();
        void shift3();
        void shift4();
        void nextLine();
        int lookupKeyword(const char *);

        bool isLineTerminator();

        int matchPunctuator(int& charPos);

        void record8(int);
        void record16(int);
        void record16(UChar);

        void copyCodeWithoutBOMs();

        int currentOffset() const;

        JSC::Identifier* makeIdentifier(const Vector<UChar>& buffer);

        bool lastTokenWasRestrKeyword() const;

        static const size_t initialReadBufferCapacity = 32;
        static const size_t initialIdentifierTableCapacity = 64;

        int m_lineNumber;

        bool m_done;
        Vector<char> m_buffer8;
        Vector<UChar> m_buffer16;
        bool m_terminator;
        bool m_delimited; // encountered delimiter like "'" and "}" on last run
        unsigned char m_skipLineEnd;
        int m_lastToken;

        State m_state;
        const SourceCode* m_source;
        const UChar* m_code;
        const UChar* m_codeStart;
        const UChar* m_codeEnd;
        bool m_isReparsing;
        bool m_atLineStart;
        bool m_error;

        // current and following unicode characters (int to allow for -1 for end-of-file marker)
        int m_current;
        int m_next1;
        int m_next2;
        int m_next3;
        
        SegmentedVector<JSC::Identifier, initialIdentifierTableCapacity> m_identifiers;

        JSGlobalData* m_globalData;

        UString m_pattern;
        UString m_flags;

        const HashTable m_keywordTable;

        Vector<UChar> m_codeWithoutBOMs;
    };

    inline bool Lexer::isWhiteSpace(int ch)
    {
        return isASCII(ch) ? (ch == ' ' || ch == '\t' || ch == 0xB || ch == 0xC) : WTF::Unicode::isSeparatorSpace(ch);
    }

    inline bool Lexer::isLineTerminator(int ch)
    {
        return ch == '\r' || ch == '\n' || ch == 0x2028 || ch == 0x2029;
    }

    inline unsigned char Lexer::convertHex(int c1, int c2)
    {
        return (toASCIIHexValue(c1) << 4) | toASCIIHexValue(c2);
    }

    inline UChar Lexer::convertUnicode(int c1, int c2, int c3, int c4)
    {
        return (convertHex(c1, c2) << 8) | convertHex(c3, c4);
    }

} // namespace JSC

#endif // Lexer_h
