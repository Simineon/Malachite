#include "parser.h"

Parser::Parser(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    keywordFormat.setForeground(QColor(248, 131, 66));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    QStringList keywordBuiltIn;
    keywordPatterns << "\\bdef\\b" << "\\bclass\\b" << "\\bpass\\b"
                    << "\\breturn\\b" << "\\bFalse\\b" << "\\bTrue\\b" << "\\bNone\\b"
                    << "\\band\\b" << "\\bor\\b" << "\\bnot\\b" << "\\bas\\b"
                    << "\\bawait\\b" << "\\byield\\b" << "\\bcontinue\\b" << "\\bin\\b"
                    << "\\bdel\\b" << "\\bimport\\b" << "\\bfrom\\b" << "\\bfor\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\belif\\b" << "\\brange\\b" 
                    << "\\bwhile\\b" << "\\bexcept\\b" << "\\btry\\b" << "\\bself\\b";
                   
    keywordBuiltIn << "\\blen\\b" << "\\bstr\\b" << "\\bint\\b" << "\\bfloat\\b"
                   << "\\blist\\b" << "\\bdict\\b" << "\\bset\\b" << "\\btuple\\b"
                   << "\\bprint\\b" << "\\binput\\b" << "\\btype\\b" << "\\bopen\\b"
                   << "\\brange\\b" << "\\benumerate\\b" << "\\bisinstance\\b"
                   << "\\bsuper\\b" << "\\bmap\\b" << "\\bfilter\\b" << "\\bsum\\b"
                   << "\\bmin\\b" << "\\bmax\\b" << "\\babs\\b" << "\\bround\\b"
                   << "\\bbool\\b";

    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    } 
    // Встроенные функции (фиолетовый)
    keywordFormat.setForeground(QColor(200, 1, 218));
    for (const QString &pattern : keywordBuiltIn) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Строки (оранжевый) ""
    doubleStringFormat.setForeground(QColor(0, 158, 0));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = doubleStringFormat;
    highlightingRules.append(rule);

    // Строки (зеленый) ''
    singleStringFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\'.*\'");
    rule.format = singleStringFormat;
    highlightingRules.append(rule);

    // Комментарии (серый)
    commentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Числа (красный)
    numberFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("\\b\\d+\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Числа (красный)
    parensFormat.setForeground(QColor(255, 255, 159));
    rule.pattern = QRegularExpression("[\\(\\)]");
    rule.format = parensFormat;
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
}