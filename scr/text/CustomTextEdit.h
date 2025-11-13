#include <QTextEdit>
#include <QKeyEvent>
#include <QTextCursor>

class CustomTextEdit : public QTextEdit {
public:
    explicit CustomTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Tab) {
            insertPlainText("    ");
            event->accept();
        } else if (event->key() == Qt::Key_Backspace) {
            handleBackspace();
            event->accept();
        } else if (event->key() == Qt::Key_ParenLeft) {
            autoParens();
            event->accept();
        } else {
            QTextEdit::keyPressEvent(event);
        }
    }

private:
    void autoParens() {
        QTextCursor cursor = textCursor();

        cursor.insertText("()");

        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }

    void handleBackspace() {
        QTextCursor cursor = textCursor();
        
        // Если есть выделенный текст - стандартное удаление
        if (cursor.hasSelection()) {
            cursor.removeSelectedText();
            return;
        }
        
        int position = cursor.position();
        
        // Проверяем, не пытаемся ли удалить открывающую скобку с парной закрывающей
        if (position > 0) {
            cursor.movePosition(QTextCursor::Left);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
            QString selectedText = cursor.selectedText();
            
            // Если это "()" - удаляем обе скобки
            if (selectedText == "()") {
                cursor.removeSelectedText();
                return;
            }
        }
        
        // Получаем текст текущей строки до курсора
        cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, position - cursor.position());
        QString textBeforeCursor = cursor.selectedText();
        
        // Считаем количество пробелов в конце (перед курсором)
        int spacesCount = 0;
        for (int i = textBeforeCursor.length() - 1; i >= 0; --i) {
            if (textBeforeCursor.at(i) == ' ') {
                spacesCount++;
            } else {
                break;
            }
        }
        
        // Если количество пробелов кратно 4 и есть хотя бы 4 пробела
        if (spacesCount >= 4 && spacesCount % 4 == 0) {
            cursor = textCursor();
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 4);
            if (cursor.selectedText() == "    ") {
                cursor.removeSelectedText();
                return;
            }
        }
        
        // Стандартное удаление одного символа
        cursor = textCursor();
        cursor.deletePreviousChar();
    }
};