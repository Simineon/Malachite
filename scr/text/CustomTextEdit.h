#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QTextCursor>
#include <QWidget>
#include <QTextBlock>

class CustomTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CustomTextEdit(QWidget *parent = nullptr);
    
protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void autoParens();
    void autoDoubleStrings();
    void autoSingleStrings();
    void autoBraces();
    void autoSquareBrackets();
    void handleBackspace();
    void handleEnter();
};

inline CustomTextEdit::CustomTextEdit(QWidget *parent) : 
    QPlainTextEdit(parent)
{
    // Устанавливаем моноширинный шрифт
    QFont font("Consolas", 12);
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    
    // Улучшаем производительность для больших файлов
    setCenterOnScroll(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
}

inline void CustomTextEdit::keyPressEvent(QKeyEvent *event) {
    // Обработка специальных клавиш
    switch (event->key()) {
        case Qt::Key_Tab:
            insertPlainText("    ");
            event->accept();
            break;
        case Qt::Key_Backspace:
            handleBackspace();
            event->accept();
            break;
        case Qt::Key_ParenLeft:
            autoParens();
            event->accept();
            break;
        case Qt::Key_Apostrophe:
            if (event->modifiers() == Qt::NoModifier) {
                autoSingleStrings();
                event->accept();
            } else {
                QPlainTextEdit::keyPressEvent(event);
            }
            break;
        case Qt::Key_QuoteDbl:
            if (event->modifiers() == Qt::NoModifier) {
                autoDoubleStrings();
                event->accept();
            } else {
                QPlainTextEdit::keyPressEvent(event);
            }
            break;
        case Qt::Key_BraceLeft:
            autoBraces();
            event->accept();
            break;
        case Qt::Key_BracketLeft:
            autoSquareBrackets();
            event->accept();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            handleEnter();
            event->accept();
            break;
        default:
            QPlainTextEdit::keyPressEvent(event);
            break;
    }
}

inline void CustomTextEdit::handleEnter() {
    QTextCursor cursor = textCursor();
    
    // Получаем текущий блок (строку)
    QTextBlock currentBlock = cursor.block();
    QString currentLineText = currentBlock.text();
    
    // Находим отступ текущей строки (количество начальных пробелов)
    int indentCount = 0;
    while (indentCount < currentLineText.length() && currentLineText.at(indentCount) == ' ') {
        indentCount++;
    }
    
    // Проверяем, содержит ли текущая строка class или def
    bool shouldAddExtraIndent = false;
    QString trimmedLine = currentLineText.trimmed();
    if (trimmedLine.startsWith("class ") || trimmedLine.startsWith("def ")) {
        shouldAddExtraIndent = true;
    }
    
    // Вставляем новую строку
    QPlainTextEdit::keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier));
    
    // Вычисляем новый отступ
    int newIndentCount = indentCount;
    if (shouldAddExtraIndent) {
        newIndentCount += 4;
    }
    
    // Если есть отступ, добавляем его к новой строке
    if (newIndentCount > 0) {
        cursor = textCursor();
        QString indent(newIndentCount, ' ');
        cursor.insertText(indent);
    }
}

inline void CustomTextEdit::autoParens() {
    QTextCursor cursor = textCursor();
    cursor.insertText("()");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoDoubleStrings() {
    QTextCursor cursor = textCursor();
    cursor.insertText("\"\"");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoSingleStrings() {
    QTextCursor cursor = textCursor();
    cursor.insertText("''");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoBraces() {
    QTextCursor cursor = textCursor();
    cursor.insertText("{}");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoSquareBrackets() {
    QTextCursor cursor = textCursor();
    cursor.insertText("[]");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::handleBackspace() {
    QTextCursor cursor = textCursor();
    
    if (cursor.hasSelection()) {
        cursor.removeSelectedText();
        return;
    }
    
    int position = cursor.position();
    
    // Проверяем автозакрывающиеся символы
    if (position > 0) {
        cursor.movePosition(QTextCursor::Left);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        QString selectedText = cursor.selectedText();
        
        if (selectedText == "()" || selectedText == "\"\"" || 
            selectedText == "''" || selectedText == "{}" || selectedText == "[]") {
            cursor.removeSelectedText();
            return;
        }
    }
    
    // Проверяем отступы (4 пробела)
    cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, position - cursor.position());
    QString textBeforeCursor = cursor.selectedText();
    
    int spacesCount = 0;
    for (int i = textBeforeCursor.length() - 1; i >= 0; --i) {
        if (textBeforeCursor.at(i) == ' ') {
            spacesCount++;
        } else {
            break;
        }
    }
    
    if (spacesCount >= 4 && spacesCount % 4 == 0) {
        cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 4);
        if (cursor.selectedText() == "    ") {
            cursor.removeSelectedText();
            return;
        }
    }
    
    // Обычное удаление
    cursor = textCursor();
    cursor.deletePreviousChar();
}

#endif // CUSTOMTEXTEDIT_H