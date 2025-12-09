#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QCompleter>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QHash>

//------------------>  maybe here bug!!!!!!!!!!!!!!!!!!!!! <--------------

// Forward declaration
class CustomTextEdit;

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CustomTextEdit *editor);
    
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CustomTextEdit *textEdit;
};

class CustomTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CustomTextEdit(QWidget *parent = nullptr);
    ~CustomTextEdit();
    
    // Completer methods
    void setCompleter(QCompleter *completer);
    QCompleter *completer() const { return m_completer; }
    
    // Line numbering methods
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void resizeEvent(QResizeEvent *event) override;
    
    // Style methods for line numbers
    void setLineNumberAreaBackground(const QColor &color);
    void setLineNumberColor(const QColor &color);
    void setCurrentLineHighlight(const QColor &color);
    void setLineNumberFont(const QFont &font);
    void setLineNumberAlignment(Qt::Alignment alignment);
    void setLineNumberMargin(int margin);
    
    QColor lineNumberAreaBackground() const { return m_lineNumberBgColor; }
    QColor lineNumberColor() const { return m_lineNumberTextColor; }
    QColor currentLineHighlight() const { return m_currentLineColor; }
    QFont lineNumberFont() const { return m_lineNumberAreaFont; }
    Qt::Alignment lineNumberAlignment() const { return m_lineNumberAlign; }
    int lineNumberMargin() const { return m_lineNumberMarginPx; }
    
    // File properties
    void setFilePath(const QString &path) { m_filePath = path; }
    QString filePath() const { return m_filePath; }
    
    // Modification tracking
    bool isModified() const { return m_isModified; }
    void setModified(bool modified) { m_isModified = modified; }

signals:
    void fileModified(bool modified);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    bool event(QEvent *event) override;

private slots:
    void insertCompletion(const QString &completion);
    void onTextChanged();
    void highlightCurrentLine();

private:
    // Auto-completion helpers
    void handleAutoQuote(QChar quoteChar);
    void handleAutoBracket(QChar openingBracket);
    QString textUnderCursor() const;
    void handleBackspace();
    void handleEnter();
    void handleTabKey(bool shiftModifier);
    void handleAutoClose(QChar opening, QChar closing);
    
    // Completer management
    void showCompleter();
    void hideCompleter();
    void updateCompleter();
    
    // Initialization
    void createCompleter();
    void setupLineNumberArea();
    void setupConnections();
    
    // Configuration
    static QStringList createPythonKeywords();
    bool shouldSkipAutoComplete(QChar ch) const;
    bool isInsideQuotesOrComment(const QTextCursor &cursor) const;
    
    // Static helper methods
    static const QHash<QChar, QChar>& bracketPairs() {
        static const QHash<QChar, QChar> pairs = {
            {'(', ')'},
            {'[', ']'},
            {'{', '}'},
            {'"', '"'},
            {'\'', '\''}
        };
        return pairs;
    }
    
    static const QString& wordBoundaryChars() {
        static const QString chars = "~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=\t\n\r ";
        return chars;
    }
    
    // Member variables
    QCompleter *m_completer = nullptr;
    LineNumberArea *m_lineNumberArea = nullptr;
    
    // Style properties for line numbers
    QColor m_lineNumberBgColor = QColor(240, 240, 240);
    QColor m_lineNumberTextColor = Qt::black;
    QColor m_currentLineColor = QColor(200, 200, 255);
    QFont m_lineNumberAreaFont;
    Qt::Alignment m_lineNumberAlign = Qt::AlignRight;
    int m_lineNumberMarginPx = 5;
    
    // File management
    QString m_filePath;
    QString m_originalContent;
    bool m_isModified = false;
};

// Inline implementations
inline LineNumberArea::LineNumberArea(CustomTextEdit *editor) 
    : QWidget(editor), textEdit(editor) 
{
    setAttribute(Qt::WA_NoSystemBackground);
}

inline QSize LineNumberArea::sizeHint() const 
{
    return QSize(textEdit->lineNumberAreaWidth(), 0);
}

inline void LineNumberArea::paintEvent(QPaintEvent *event) 
{
    textEdit->lineNumberAreaPaintEvent(event);
}

inline CustomTextEdit::CustomTextEdit(QWidget *parent) 
    : QPlainTextEdit(parent)
{
    // Font setup
    QFont font("Consolas", 12);
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    
    // Set line number font (can be different from main font)
    m_lineNumberAreaFont = font;
    m_lineNumberAreaFont.setPointSize(font.pointSize() - 1);
    
    // Optimization
    setCenterOnScroll(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    
    // Create completer
    createCompleter();
    
    // Setup line number area
    setupLineNumberArea();
    
    // Setup connections
    setupConnections();
}

inline CustomTextEdit::~CustomTextEdit() 
{
    delete m_completer;
    delete m_lineNumberArea;
}

inline void CustomTextEdit::setupLineNumberArea()
{
    m_lineNumberArea = new LineNumberArea(this);
    
    connect(this, &QPlainTextEdit::blockCountChanged, 
            this, &CustomTextEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, 
            this, &CustomTextEdit::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, 
            this, &CustomTextEdit::highlightCurrentLine);
    
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

inline void CustomTextEdit::setupConnections()
{
    connect(this->document(), &QTextDocument::contentsChanged,
            this, &CustomTextEdit::onTextChanged);
}

inline void CustomTextEdit::createCompleter()
{
    QStringList keywords = createPythonKeywords();
    QCompleter *completer = new QCompleter(keywords, this);
    setCompleter(completer);
}

inline QStringList CustomTextEdit::createPythonKeywords()
{
    return {
        // Keywords
        "False", "None", "True", "and", "as", "assert", "async", 
        "await", "break", "class", "continue", "def", "del", "elif", 
        "else", "except", "finally", "for", "from", "global", "if", 
        "import", "in", "is", "lambda", "nonlocal", "not", "or", 
        "pass", "raise", "return", "try", "while", "with", "yield",
        
        // Special identifiers
        "self", "cls", "__init__", "__name__", "__main__",
        
        // Built-in functions
        "abs", "all", "any", "ascii", "bin", "bool", "breakpoint",
        "bytearray", "bytes", "callable", "chr", "classmethod",
        "compile", "complex", "delattr", "dict", "dir", "divmod",
        "enumerate", "eval", "exec", "filter", "float", "format",
        "frozenset", "getattr", "globals", "hasattr", "hash",
        "help", "hex", "id", "input", "int", "isinstance",
        "issubclass", "iter", "len", "list", "locals", "map",
        "max", "memoryview", "min", "next", "object", "oct",
        "open", "ord", "pow", "print", "property", "range",
        "repr", "reversed", "round", "set", "setattr", "slice",
        "sorted", "staticmethod", "str", "sum", "super", "tuple",
        "type", "vars", "zip",
        
        // Common modules
        "os", "sys", "json", "re", "datetime", "math", "random",
        "collections", "itertools", "functools", "typing"
    };
}

// Style methods implementation
inline void CustomTextEdit::setLineNumberAreaBackground(const QColor &color) 
{
    m_lineNumberBgColor = color;
    if (m_lineNumberArea) {
        m_lineNumberArea->update();
    }
}

inline void CustomTextEdit::setLineNumberColor(const QColor &color) 
{
    m_lineNumberTextColor = color;
    if (m_lineNumberArea) {
        m_lineNumberArea->update();
    }
}

inline void CustomTextEdit::setCurrentLineHighlight(const QColor &color) 
{
    m_currentLineColor = color;
    highlightCurrentLine();
}

inline void CustomTextEdit::setLineNumberFont(const QFont &font) 
{
    m_lineNumberAreaFont = font;
    if (m_lineNumberArea) {
        m_lineNumberArea->update();
        updateLineNumberAreaWidth(0);
    }
}

inline void CustomTextEdit::setLineNumberAlignment(Qt::Alignment alignment) 
{
    m_lineNumberAlign = alignment;
    if (m_lineNumberArea) {
        m_lineNumberArea->update();
    }
}

inline void CustomTextEdit::setLineNumberMargin(int margin) 
{
    m_lineNumberMarginPx = margin;
    if (m_lineNumberArea) {
        m_lineNumberArea->update();
    }
}

inline int CustomTextEdit::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    QFontMetrics fm(m_lineNumberAreaFont);
    int space = m_lineNumberMarginPx * 2 + fm.horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

inline void CustomTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event) 
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), m_lineNumberBgColor);
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    painter.setFont(m_lineNumberAreaFont);
    QFontMetrics fm(m_lineNumberAreaFont);
    int lineHeight = fm.height();
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            
            // Highlight current line
            if (textCursor().blockNumber() == blockNumber) {
                painter.fillRect(0, top, m_lineNumberArea->width(), lineHeight, m_currentLineColor);
            }
            
            // Draw line number
            painter.setPen(m_lineNumberTextColor);
            QRect numberRect(0, top, m_lineNumberArea->width() - m_lineNumberMarginPx, lineHeight);
            painter.drawText(numberRect, m_lineNumberAlign | Qt::AlignVCenter, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

inline void CustomTextEdit::updateLineNumberAreaWidth(int newBlockCount) 
{
    Q_UNUSED(newBlockCount);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

inline void CustomTextEdit::updateLineNumberArea(const QRect &rect, int dy) 
{
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

inline void CustomTextEdit::resizeEvent(QResizeEvent *event) 
{
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), 
                                        lineNumberAreaWidth(), cr.height()));
}

inline void CustomTextEdit::highlightCurrentLine() 
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(m_currentLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    
    setExtraSelections(extraSelections);
}

inline void CustomTextEdit::setCompleter(QCompleter *completer) 
{
    if (m_completer) {
        disconnect(m_completer, nullptr, this, nullptr);
        delete m_completer;
    }
    
    m_completer = completer;
    if (!m_completer) {
        return;
    }
    
    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CustomTextEdit::insertCompletion);
}

inline void CustomTextEdit::focusInEvent(QFocusEvent *event) 
{
    if (m_completer) {
        m_completer->setWidget(this);
    }
    QPlainTextEdit::focusInEvent(event);
}

inline bool CustomTextEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        // Можно добавить всплывающие подсказки здесь
        return QPlainTextEdit::event(event);
    }
    return QPlainTextEdit::event(event);
}

inline void CustomTextEdit::insertCompletion(const QString &completion) 
{
    if (!m_completer || m_completer->widget() != this) {
        return;
    }
    
    QTextCursor tc = textCursor();
    int prefixLength = m_completer->completionPrefix().length();
    
    tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, prefixLength);
    tc.insertText(completion);
    setTextCursor(tc);
}

inline QString CustomTextEdit::textUnderCursor() const 
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

inline void CustomTextEdit::hideCompleter() 
{
    if (m_completer && m_completer->popup()) {
        m_completer->popup()->hide();
    }
}

inline void CustomTextEdit::showCompleter()
{
    if (!m_completer || !m_completer->completionCount()) {
        return;
    }
    
    QRect cr = cursorRect();
    cr.setWidth(m_completer->popup()->sizeHintForColumn(0) +
                m_completer->popup()->verticalScrollBar()->sizeHint().width());
    m_completer->complete(cr);
}

inline void CustomTextEdit::updateCompleter()
{
    if (!m_completer) {
        return;
    }
    
    QString completionPrefix = textUnderCursor();
    if (completionPrefix != m_completer->completionPrefix()) {
        m_completer->setCompletionPrefix(completionPrefix);
        if (m_completer->popup()) {
            m_completer->popup()->setCurrentIndex(
                m_completer->completionModel()->index(0, 0));
        }
    }
    
    if (completionPrefix.length() >= 1 && m_completer->completionCount() > 0) {
        showCompleter();
    } else {
        hideCompleter();
    }
}

inline void CustomTextEdit::keyPressEvent(QKeyEvent *event) 
{
    // Handle completer popup
    if (m_completer && m_completer->popup() && m_completer->popup()->isVisible()) {
        switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            event->ignore();
            return;
        default:
            break;
        }
    }
    
    // Handle special keys
    switch (event->key()) {
    case Qt::Key_Backspace:
        hideCompleter();
        handleBackspace();
        event->accept();
        return;
        
    case Qt::Key_Tab:
        handleTabKey(event->modifiers() & Qt::ShiftModifier);
        event->accept();
        return;
        
    case Qt::Key_Return:
    case Qt::Key_Enter:
        handleEnter();
        event->accept();
        return;
        
    case Qt::Key_ParenLeft:
        handleAutoClose('(', ')');
        event->accept();
        return;
        
    case Qt::Key_BracketLeft:
        handleAutoClose('[', ']');
        event->accept();
        return;
        
    case Qt::Key_BraceLeft:
        handleAutoClose('{', '}');
        event->accept();
        return;
        
    case Qt::Key_QuoteDbl:
        handleAutoQuote('"');
        event->accept();
        return;
        
    case Qt::Key_Apostrophe:
        handleAutoQuote('\'');
        event->accept();
        return;
        
    case Qt::Key_ParenRight:
    case Qt::Key_BracketRight:
    case Qt::Key_BraceRight:
        // Skip over existing closing brackets
        QTextCursor cursor = textCursor();
        if (cursor.position() < document()->characterCount()) {
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
            QString nextChar = cursor.selectedText();
            if ((event->key() == Qt::Key_ParenRight && nextChar == ")") ||
                (event->key() == Qt::Key_BracketRight && nextChar == "]") ||
                (event->key() == Qt::Key_BraceRight && nextChar == "}")) {
                cursor.clearSelection();
                cursor.movePosition(QTextCursor::Right);
                setTextCursor(cursor);
                event->accept();
                return;
            }
        }
        break;
    }
    
    // Handle regular text input for auto-completion
    if (!event->text().isEmpty()) {
        QChar pressedChar = event->text().at(0);
        
        // Check if we should trigger auto-completion
        if (!shouldSkipAutoComplete(pressedChar)) {
            // Let parent class handle the key first
            QPlainTextEdit::keyPressEvent(event);
            
            // Then update completer
            if (m_completer && !isInsideQuotesOrComment(textCursor())) {
                updateCompleter();
            }
            return;
        }
    }
    
    // Default handling
    QPlainTextEdit::keyPressEvent(event);
}

inline bool CustomTextEdit::shouldSkipAutoComplete(QChar ch) const
{
    return wordBoundaryChars().contains(ch);
}

inline bool CustomTextEdit::isInsideQuotesOrComment(const QTextCursor &cursor) const
{
    Q_UNUSED(cursor);
    return false;
}

inline void CustomTextEdit::handleAutoQuote(QChar quoteChar) 
{
    QTextCursor cursor = textCursor();
    
    // Check for selection
    if (cursor.hasSelection()) {
        QString selectedText = cursor.selectedText();
        cursor.insertText(QString(quoteChar) + selectedText + quoteChar);
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }
    
    // Check if next character is the same quote
    int pos = cursor.position();
    if (pos < document()->characterCount()) {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        QString nextChar = cursor.selectedText();
        if (nextChar == quoteChar) {
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::Right);
            setTextCursor(cursor);
            return;
        }
    }
    
    // Insert quote pair
    cursor.insertText(QString(quoteChar) + quoteChar);
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::handleAutoClose(QChar opening, QChar closing)
{
    QTextCursor cursor = textCursor();
    
    // Check for selection
    if (cursor.hasSelection()) {
        QString selectedText = cursor.selectedText();
        cursor.insertText(QString(opening) + selectedText + closing);
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }
    
    // Insert bracket pair
    cursor.insertText(QString(opening) + closing);
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::handleAutoBracket(QChar openingBracket) 
{
    QChar closingBracket = bracketPairs().value(openingBracket);
    if (closingBracket.isNull()) {
        return;
    }
    
    handleAutoClose(openingBracket, closingBracket);
}

inline void CustomTextEdit::handleBackspace() 
{
    QTextCursor cursor = textCursor();
    
    if (cursor.hasSelection()) {
        cursor.removeSelectedText();
        return;
    }
    
    int position = cursor.position();
    
    // Check for auto-inserted pairs
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
    
    // Handle smart backspace for indentation
    cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 
                       position - cursor.position());
    QString textBeforeCursor = cursor.selectedText();
    
    // Count trailing spaces
    int spacesCount = 0;
    for (int i = textBeforeCursor.length() - 1; i >= 0; --i) {
        if (textBeforeCursor.at(i) == ' ') {
            spacesCount++;
        } else {
            break;
        }
    }
    
    // Delete 4 spaces if we have multiples of 4
    if (spacesCount >= 4 && spacesCount % 4 == 0) {
        cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 4);
        if (cursor.selectedText() == "    ") {
            cursor.removeSelectedText();
            return;
        }
    }
    
    // Default backspace
    cursor = textCursor();
    cursor.deletePreviousChar();
}

inline void CustomTextEdit::handleTabKey(bool shiftModifier)
{
    if (shiftModifier) {
        // Unindent
        QTextCursor cursor = textCursor();
        
        if (cursor.hasSelection()) {
            // Unindent all selected lines
            int start = cursor.selectionStart();
            int end = cursor.selectionEnd();
            
            cursor.setPosition(start);
            int startBlock = cursor.blockNumber();
            
            cursor.setPosition(end);
            int endBlock = cursor.blockNumber();
            
            cursor.beginEditBlock();
            
            for (int i = startBlock; i <= endBlock; ++i) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);
                
                if (cursor.selectedText() == "    ") {
                    cursor.removeSelectedText();
                }
                
                cursor.movePosition(QTextCursor::NextBlock);
            }
            
            cursor.endEditBlock();
        } else {
            // Unindent current line
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);
            
            if (cursor.selectedText() == "    ") {
                cursor.removeSelectedText();
            }
        }
    } else {
        // Indent or insert tab
        if (textCursor().hasSelection()) {
            // Indent all selected lines
            QTextCursor cursor = textCursor();
            int start = cursor.selectionStart();
            int end = cursor.selectionEnd();
            
            cursor.setPosition(start);
            int startBlock = cursor.blockNumber();
            
            cursor.setPosition(end);
            int endBlock = cursor.blockNumber();
            
            cursor.beginEditBlock();
            
            for (int i = startBlock; i <= endBlock; ++i) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.insertText("    ");
                cursor.movePosition(QTextCursor::NextBlock);
            }
            
            cursor.endEditBlock();
        } else {
            insertPlainText("    ");
        }
    }
}

inline void CustomTextEdit::handleEnter() 
{
    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();
    QString currentLineText = currentBlock.text();
    
    // Count leading spaces
    int indentCount = 0;
    while (indentCount < currentLineText.length() && 
           currentLineText.at(indentCount).isSpace()) {
        indentCount++;
    }
    
    // Check if we need extra indentation
    bool extraIndent = false;
    QString trimmedLine = currentLineText.trimmed();
    
    if (trimmedLine.endsWith(':')) {
        extraIndent = true;
    } else if (trimmedLine.startsWith("class ") || trimmedLine.startsWith("def ")) {
        extraIndent = true;
    } else if (trimmedLine.contains("{")) {
        extraIndent = true;
    }
    
    // Insert new line
    QPlainTextEdit::keyPressEvent(new QKeyEvent(QEvent::KeyPress, 
                                                Qt::Key_Return, 
                                                Qt::NoModifier));
    
    // Apply indentation
    int newIndent = indentCount + (extraIndent ? 4 : 0);
    if (newIndent > 0) {
        cursor = textCursor();
        cursor.insertText(QString(newIndent, ' '));
    }
}

inline void CustomTextEdit::onTextChanged()
{
    QString currentContent = toPlainText();
    if (!m_originalContent.isEmpty() && currentContent != m_originalContent) {
        m_isModified = true;
        emit fileModified(true);
    } else if (m_originalContent.isEmpty() && !currentContent.isEmpty()) {
        m_isModified = true;
        emit fileModified(true);
    }
}

#endif // CUSTOMTEXTEDIT_H