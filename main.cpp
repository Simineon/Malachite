#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <iostream>
#include <list>
#include "scr/text/CustomTextEdit.h"


// Класс для подсветки синтаксиса
class Parser : public QSyntaxHighlighter {
    Q_OBJECT

public:
    Parser(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {
        // Настройка правил подсветки
        HighlightingRule rule;

        // Ключевые слова Python (синий)
        keywordFormat.setForeground(Qt::blue);
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns;
        keywordPatterns << "\\bprint\\b" << "\\bdef\\b" << "\\bclass\\b" << "\\bpass\\b"
                        << "\\breturn\\b" << "\\bFalse\\b" << "\\bTrue\\b" << "\\binput\\b"
                        << "\\bNone\\b" << "\\band\\b" << "\\bor\\b" << "\\bnot\\b"
                        << "\\bas\\b" << "\\bawait\\b" << "\\byield\\b" << "\\bcontinue\\b"
                        << "\\bdel\\b" << "\\bimport\\b" << "\\bfrom\\b" << "\\bin\\b"
                        << "\\bif\\b" << "\\belse\\b" << "\\belif\\b" << "\\bfor\\b"
                        << "\\bwhile\\b" << "\\bexcept\\b" << "\\btry\\b" << "\\brange\\b" 
                        << "\\bself\\b";

        for (const QString &pattern : keywordPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        // Строки (зеленый) ""
        doubleStringFormat.setForeground(Qt::darkGreen);
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
    }

protected:
    void highlightBlock(const QString &text) override {
        for (const HighlightingRule &rule : highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    // Форматы для разных типов текста
    QTextCharFormat keywordFormat;
    QTextCharFormat doubleStringFormat;
    QTextCharFormat singleStringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat numberFormat;
};

class App : public QWidget {
    Q_OBJECT

public:
    App(QWidget *parent = nullptr) : QWidget(parent) {
        // Создаем текстовый редактор
        CustomTextEdit *editor = new CustomTextEdit(this);
        
        // Устанавливаем начальный текст с Python кодом
        editor->setPlainText(
            "def hello_world():\n"
            "    # Это комментарий\n"
            "    print(\"Hello, World!\")\n"
            "    x = 42\n"
            "    if x > 10:\n"
            "        return True\n"
            "    else:\n"
            "        return False\n"
            "\n"
            "class MyClass:\n"
            "    def __init__(self):\n"
            "        self.value = 100\n"
        );
        
        // Создаем подсветку синтаксиса
        Parser *highlighter = new Parser(editor->document());
        
        // Создаем layout и добавляем элементы
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(editor);

        // Настраиваем окно
        setWindowTitle("Malachite IDE");
        setFixedSize(700, 500);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    App window;
    window.show();
    
    return app.exec();
}

#include "main.moc"