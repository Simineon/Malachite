#ifndef PARSER_H
#define PARSER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QColor>

class Parser : public QSyntaxHighlighter {
    Q_OBJECT

public:
    Parser(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    // Formats for different types of text
    QTextCharFormat keywordFormat;
    QTextCharFormat doubleStringFormat;
    QTextCharFormat singleStringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat parensFormat;
};

#endif 