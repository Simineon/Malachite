#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QTextCursor>
#include <QWidget>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>

class LineNumberArea;

class CustomTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CustomTextEdit(QWidget *parent = nullptr);
    
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    void autoParens();
    void autoDoubleStrings();
    void autoSingleStrings();
    void autoBraces();
    void autoSquareBrackets();
    void handleBackspace();
    
    LineNumberArea *lineNumberArea;

    // Добавляем дружественный класс для доступа к protected методам
    friend class LineNumberArea;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CustomTextEdit *editor) : QWidget(editor), textEditor(editor) {}
    
    QSize sizeHint() const override {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CustomTextEdit *textEditor;
};

inline CustomTextEdit::CustomTextEdit(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);
    
    connect(this->document(), &QTextDocument::blockCountChanged, 
            this, &CustomTextEdit::updateLineNumberAreaWidth);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, [this](int){ lineNumberArea->update(); });
    connect(this, &QPlainTextEdit::textChanged,
            this, [this](){ lineNumberArea->update(); });
    
    connect(this->document(), &QTextDocument::contentsChange,
            this, [this](int, int, int){ lineNumberArea->update(); });
    
    updateLineNumberAreaWidth(0);
}

inline void CustomTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                            Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

inline int CustomTextEdit::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

inline void CustomTextEdit::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), 
                                     lineNumberAreaWidth(), cr.height()));
}

inline void CustomTextEdit::updateLineNumberAreaWidth(int newBlockCount) {
    Q_UNUSED(newBlockCount);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

inline void CustomTextEdit::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

inline void CustomTextEdit::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        event->accept();
    } else if (event->key() == Qt::Key_Backspace) {
        handleBackspace();
        event->accept();
    } else if (event->key() == Qt::Key_ParenLeft) {
        autoParens();
        event->accept();
    } else if (event->key() == Qt::Key_Apostrophe) {
        autoSingleStrings();
        event->accept();
    } else if (event->key() == Qt::Key_QuoteDbl) {
        autoDoubleStrings();
        event->accept();
    } else if (event->key() == Qt::Key_BraceLeft) {
        autoBraces();
        event->accept();
    } else if (event->key() == Qt::Key_BracketLeft) {
        autoSquareBrackets();
        event->accept();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Обрабатываем Enter для немедленного обновления нумерации
        QPlainTextEdit::keyPressEvent(event);
        lineNumberArea->update();
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
    
    // Всегда обновляем область нумерации после любого нажатия клавиши
    lineNumberArea->update();
}

inline void CustomTextEdit::autoParens() {
    QTextCursor cursor = textCursor();
    cursor.insertText("()");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
    lineNumberArea->update();
}

inline void CustomTextEdit::autoDoubleStrings() {
    QTextCursor cursor = textCursor();
    cursor.insertText("\"\"");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
    lineNumberArea->update();
}

inline void CustomTextEdit::autoSingleStrings() {
    QTextCursor cursor = textCursor();
    cursor.insertText("\'\'");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
    lineNumberArea->update();
}

inline void CustomTextEdit::autoBraces() {
    QTextCursor cursor = textCursor();
    cursor.insertText("{}");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
    lineNumberArea->update();
}

inline void CustomTextEdit::autoSquareBrackets() {
    QTextCursor cursor = textCursor();
    cursor.insertText("[]");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
    lineNumberArea->update();
}

inline void CustomTextEdit::handleBackspace() {
    QTextCursor cursor = textCursor();
    
    if (cursor.hasSelection()) {
        cursor.removeSelectedText();
        lineNumberArea->update();
        return;
    }
    
    int position = cursor.position();
    
    if (position > 0) {
        cursor.movePosition(QTextCursor::Left);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        QString selectedText = cursor.selectedText();
        
        if (selectedText == "()" || selectedText == "\"\"" || 
            selectedText == "\'\'" || selectedText == "{}" || selectedText == "[]") {
            cursor.removeSelectedText();
            lineNumberArea->update();
            return;
        }
    }
    
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
            lineNumberArea->update();
            return;
        }
    }
    
    cursor = textCursor();
    cursor.deletePreviousChar();
    lineNumberArea->update();
}

#endif // CUSTOMTEXTEDIT_H