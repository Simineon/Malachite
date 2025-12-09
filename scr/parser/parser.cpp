#include "parser.h"

Parser::Parser(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    keywordFormat.setForeground(QColor(248, 131, 66));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    QStringList keywordBuiltIn;
    
    keywordPatterns 
        << "\\bFalse\\b" << "\\bNone\\b" << "\\bTrue\\b" 
        << "\\band\\b" << "\\bas\\b" << "\\bassert\\b" << "\\basync\\b" 
        << "\\bawait\\b" << "\\bbreak\\b" << "\\bclass\\b" << "\\bcontinue\\b" 
        << "\\bdef\\b" << "\\bdel\\b" << "\\belif\\b" << "\\belse\\b" 
        << "\\bexcept\\b" << "\\bfinally\\b" << "\\bfor\\b" << "\\bfrom\\b" 
        << "\\bglobal\\b" << "\\bif\\b" << "\\bimport\\b" << "\\bin\\b" 
        << "\\bis\\b" << "\\blambda\\b" << "\\bnonlocal\\b" << "\\bnot\\b" 
        << "\\bor\\b" << "\\bpass\\b" << "\\braise\\b" << "\\breturn\\b" 
        << "\\btry\\b" << "\\bwhile\\b" << "\\bwith\\b" << "\\byield\\b"
        
        // special ids
        << "\\bself\\b" << "\\bcls\\b";

    keywordBuiltIn 
        << "\\babs\\b" << "\\ball\\b" << "\\bany\\b" << "\\bascii\\b" 
        << "\\bbin\\b" << "\\bbool\\b" << "\\bbreakpoint\\b" << "\\bbytearray\\b" 
        << "\\bbytes\\b" << "\\bcallable\\b" << "\\bchr\\b" << "\\bclassmethod\\b" 
        << "\\bcompile\\b" << "\\bcomplex\\b" << "\\bdelattr\\b" << "\\bdict\\b" 
        << "\\bdir\\b" << "\\bdivmod\\b" << "\\benumerate\\b" << "\\beval\\b" 
        << "\\bexec\\b" << "\\bfilter\\b" << "\\bfloat\\b" << "\\bformat\\b" 
        << "\\bfrozenset\\b" << "\\bgetattr\\b" << "\\bglobals\\b" << "\\bhasattr\\b" 
        << "\\bhash\\b" << "\\bhelp\\b" << "\\bhex\\b" << "\\bid\\b" 
        << "\\binput\\b" << "\\bint\\b" << "\\bisinstance\\b" << "\\bissubclass\\b" 
        << "\\biter\\b" << "\\blen\\b" << "\\blist\\b" << "\\blocals\\b" 
        << "\\bmap\\b" << "\\bmax\\b" << "\\bmemoryview\\b" << "\\bmin\\b" 
        << "\\bnext\\b" << "\\bobject\\b" << "\\boct\\b" << "\\bopen\\b" 
        << "\\bord\\b" << "\\bpow\\b" << "\\bprint\\b" << "\\bproperty\\b" 
        << "\\brange\\b" << "\\brepr\\b" << "\\breversed\\b" << "\\bround\\b" 
        << "\\bset\\b" << "\\bsetattr\\b" << "\\bslice\\b" << "\\bsorted\\b" 
        << "\\bstaticmethod\\b" << "\\bstr\\b" << "\\bsum\\b" << "\\bsuper\\b" 
        << "\\btuple\\b" << "\\btype\\b" << "\\bvars\\b" << "\\bzip\\b" 
        
        // Exceptions
        << "\\bBaseException\\b" << "\\bException\\b" << "\\bArithmeticError\\b" 
        << "\\bBufferError\\b" << "\\bLookupError\\b" << "\\bAssertionError\\b" 
        << "\\bAttributeError\\b" << "\\bEOFError\\b" << "\\bFloatingPointError\\b" 
        << "\\bGeneratorExit\\b" << "\\bImportError\\b" << "\\bModuleNotFoundError\\b" 
        << "\\bIndexError\\b" << "\\bKeyError\\b" << "\\bKeyboardInterrupt\\b" 
        << "\\bMemoryError\\b" << "\\bNameError\\b" << "\\bNotImplementedError\\b" 
        << "\\bOSError\\b" << "\\bOverflowError\\b" << "\\bRecursionError\\b" 
        << "\\bReferenceError\\b" << "\\bRuntimeError\\b" << "\\bStopIteration\\b" 
        << "\\bStopAsyncIteration\\b" << "\\bSyntaxError\\b" << "\\bIndentationError\\b" 
        << "\\bTabError\\b" << "\\bSystemError\\b" << "\\bSystemExit\\b" 
        << "\\bTypeError\\b" << "\\bUnboundLocalError\\b" << "\\bUnicodeError\\b" 
        << "\\bUnicodeDecodeError\\b" << "\\bUnicodeEncodeError\\b" << "\\bUnicodeTranslateError\\b" 
        << "\\bValueError\\b" << "\\bZeroDivisionError\\b" << "\\bEnvironmentError\\b" 
        << "\\bIOError\\b" << "\\bWindowsError\\b" << "\\bBlockingIOError\\b" 
        << "\\bChildProcessError\\b" << "\\bConnectionError\\b" << "\\bBrokenPipeError\\b" 
        << "\\bConnectionAbortedError\\b" << "\\bConnectionRefusedError\\b" << "\\bConnectionResetError\\b" 
        << "\\bFileExistsError\\b" << "\\bFileNotFoundError\\b" << "\\bInterruptedError\\b" 
        << "\\bIsADirectoryError\\b" << "\\bNotADirectoryError\\b" << "\\bPermissionError\\b" 
        << "\\bProcessLookupError\\b" << "\\bTimeoutError\\b"
        
        // constants
        << "\\b__name__\\b" << "\\b__main__\\b" << "\\b__file__\\b" << "\\b__doc__\\b"
        << "\\b__package__\\b" << "\\b__version__\\b";

    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    } 
    
    // build in funcs (purple)
    keywordFormat.setForeground(QColor(200, 1, 218));
    for (const QString &pattern : keywordBuiltIn) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // strings (green) ""
    doubleStringFormat.setForeground(QColor(0, 158, 0));
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = doubleStringFormat;
    highlightingRules.append(rule);

    // strings (green) ''
    singleStringFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\'.*?\'");
    rule.format = singleStringFormat;
    highlightingRules.append(rule);

    // Many strings (triple quotes)
    multiLineStringFormat.setForeground(QColor(0, 128, 0)); 
    rule.pattern = QRegularExpression("\"\"\".*?\"\"\"");
    rule.format = multiLineStringFormat;
    highlightingRules.append(rule);
    
    rule.pattern = QRegularExpression("\'\'\'.*?\'\'\'");
    rule.format = multiLineStringFormat;
    highlightingRules.append(rule);

    // comments (gray)
    commentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // numbers (red)
    numberFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
    
    // hexademicimal numbers
    rule.pattern = QRegularExpression("\\b0[xX][0-9a-fA-F]+\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
    // something else
    // 8
    rule.pattern = QRegularExpression("\\b0[oO][0-7]+\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
    
    // 2
    rule.pattern = QRegularExpression("\\b0[bB][01]+\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    decoratorFormat.setForeground(QColor(0, 100, 200));  
    decoratorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("@\\w+");
    rule.format = decoratorFormat;
    highlightingRules.append(rule);
}

void Parser::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    
    setCurrentBlockState(0);
    
    highlightMultiLine(text, "\"\"\"", multiLineStringFormat);
    highlightMultiLine(text, "'''", multiLineStringFormat);
}

void Parser::highlightMultiLine(const QString &text, const QString &delimiter, const QTextCharFormat &format) {
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(delimiter);
    }

    while (startIndex >= 0) {
        int endIndex = text.indexOf(delimiter, startIndex + delimiter.length());
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + delimiter.length();
        }
        setFormat(startIndex, commentLength, format);
        startIndex = text.indexOf(delimiter, startIndex + commentLength);
    }
}