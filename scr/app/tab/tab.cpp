#include "tab.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

Tab::Tab(QWidget *parent) 
    : QTabWidget(parent)
    , nextTabAction(nullptr)
    , prevTabAction(nullptr)
    , newTabAction(nullptr)
    , closeTabAction(nullptr)
{
    setupTabWidget();
    setupActions();
}

void Tab::setupTabWidget()
{
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(true);

    // Стилизация вкладок
    setStyleSheet(
        "QTabWidget::pane {"
        "    border: none;"
        "    background-color: #1e1e1e;"
        "}"
        "QTabWidget::tab-bar {"
        "    alignment: left;"
        "}"
        "QTabBar::tab {"
        "    background-color: #2d2d30;"
        "    color: #cccccc;"
        "    padding: 8px 16px;"
        "    margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: #1e1e1e;"
        "    color: white;"
        "    border-bottom: 2px solid #0e639c;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "    background-color: #383838;"
        "}"
        "QTabBar::close-button {"
        "    subcontrol-position: right;"
        "    width: 16px;"
        "    height: 16px;"
        "}"

    );

    // Connect для закрытия вкладок
    connect(this, &QTabWidget::tabCloseRequested, this, &Tab::closeTab);
    
    // Connect для смены активной вкладки
    connect(this, &QTabWidget::currentChanged, this, &Tab::onTabChanged);
}

void Tab::setupActions()
{
    nextTabAction = new QAction("Next Tab", this);
    prevTabAction = new QAction("Previous Tab", this);
    newTabAction = new QAction("New Tab", this);
    closeTabAction = new QAction("Close Tab", this);

    nextTabAction->setShortcut(QKeySequence("Ctrl+Tab"));
    prevTabAction->setShortcut(QKeySequence("Ctrl+Shift+Tab"));
    newTabAction->setShortcut(QKeySequence::AddTab);
    closeTabAction->setShortcut(QKeySequence::Close);

    connect(nextTabAction, &QAction::triggered, this, &Tab::nextTab);
    connect(prevTabAction, &QAction::triggered, this, &Tab::prevTab);
    connect(newTabAction, &QAction::triggered, this, &Tab::newTab);
    connect(closeTabAction, &QAction::triggered, this, &Tab::closeCurrentTab);
}

void Tab::setupWindowMenu(QMenu *windowMenu)
{
    windowMenu->addAction(newTabAction);
    windowMenu->addAction(closeTabAction);
    windowMenu->addSeparator();
    windowMenu->addAction(nextTabAction);
    windowMenu->addAction(prevTabAction);
}

CustomTextEdit* Tab::createEditor()
{
    CustomTextEdit *editor = new CustomTextEdit(this);
    editor->setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: none;"
        "    selection-background-color: #264f78;"
        "    font-family: 'Consolas', 'Monaco', 'Courier New', monospace;"
        "    font-size: 14px;"
        "    line-height: 1.4;"
        "}"
    );
    return editor;
}

CustomTextEdit* Tab::getCurrentEditor()
{
    return qobject_cast<CustomTextEdit*>(currentWidget());
}

QString Tab::getCurrentFilePath()
{
    CustomTextEdit *editor = getCurrentEditor();
    if (editor) {
        return editor->property("filePath").toString();
    }
    return QString();
}

void Tab::newTab()
{
    CustomTextEdit *editor = createEditor();
    editor->setProperty("filePath", QString());
    editor->setProperty("isModified", false);
    editor->setProperty("originalContent", "");
    
    int tabIndex = addTab(editor, "untitled.py");
    setCurrentIndex(tabIndex);
    
    // Добавляем подсветку синтаксиса
    new Parser(editor->document());
    
    // Connect для отслеживания изменений
    connect(editor, &CustomTextEdit::textChanged, this, [this, editor]() {
        this->onEditorTextChanged(editor, "");
    });
}

void Tab::openFileInTab(const QString &filePath)
{
    // Проверяем, не открыт ли файл уже в другой вкладке
    for (int i = 0; i < count(); ++i) {
        CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(widget(i));
        if (editor && editor->property("filePath").toString() == filePath) {
            setCurrentIndex(i);
            return;
        }
    }
    
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        
        // Автоматическое определение кодировки
        in.setAutoDetectUnicode(true);
        
        QString fileContent = in.readAll();
        
        // Создаем новую вкладку
        CustomTextEdit *editor = createEditor();
        editor->setPlainText(fileContent);
        editor->setProperty("filePath", filePath);
        editor->setProperty("isModified", false);
        editor->setProperty("originalContent", fileContent);
        
        QFileInfo fileInfo(filePath);
        QString tabName = fileInfo.fileName();
        int tabIndex = addTab(editor, tabName);
        setCurrentIndex(tabIndex);
        
        // Добавляем подсветку синтаксиса только для Python файлов
        if (filePath.endsWith(".py", Qt::CaseInsensitive)) {
            new Parser(editor->document());
        }
        
        // Connect для отслеживания изменений
        connect(editor, &CustomTextEdit::textChanged, this, [this, editor, fileContent]() {
            this->onEditorTextChanged(editor, fileContent);
        });
        
        file.close();
        emit currentTabChanged(); // Испускаем сигнал
    } else {
        QMessageBox::warning(this, "Error", "Error in file opening!");
    }
}

void Tab::saveTabContent(CustomTextEdit *editor, const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString content = editor->toPlainText();
        out << content;
        file.close();
        
        editor->setProperty("isModified", false);
        editor->setProperty("originalContent", content);
        updateTabTitle(currentIndex());
    } else {
        QMessageBox::warning(this, "Error", "Error in file saving!");
    }
}

void Tab::closeCurrentTab()
{
    closeTab(currentIndex());
}

void Tab::closeTab(int index)
{
    if (index < 0) return;
    
    CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(widget(index));
    if (editor && editor->property("isModified").toBool()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Save changes", 
                                    "The document has been modified. Do you want to save changes?",
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            QString filePath = editor->property("filePath").toString();
            if (filePath.isEmpty()) {
                // Если файл не сохранен, вызываем saveAs через родителя
                emit requestSaveAs(); // Теперь этот сигнал объявлен
                return;
            } else {
                saveTabContent(editor, filePath);
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    removeTab(index);
    
    // Если вкладок не осталось, создаем новую
    if (count() == 0) {
        newTab();
    }
}

void Tab::nextTab()
{
    int current = currentIndex();
    int next = (current + 1) % count();
    setCurrentIndex(next);
}

void Tab::prevTab()
{
    int current = currentIndex();
    int prev = (current - 1 + count()) % count();
    setCurrentIndex(prev);
}

void Tab::onTabChanged(int index)
{
    Q_UNUSED(index)
    // Сигнал для обновления заголовка окна
    emit currentTabChanged(); // Теперь этот сигнал объявлен
}

void Tab::onEditorTextChanged(CustomTextEdit *editor, const QString &originalContent)
{
    QString currentContent = editor->toPlainText();
    bool isModified = (currentContent != originalContent);
    
    if (editor->property("isModified").toBool() != isModified) {
        editor->setProperty("isModified", isModified);
        updateTabTitle(indexOf(editor));
    }
}

void Tab::updateTabTitle(int index)
{
    if (index < 0) return;
    
    CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(widget(index));
    if (!editor) return;
    
    QString filePath = editor->property("filePath").toString();
    QString title;
    
    if (filePath.isEmpty()) {
        title = "untitled.py";
    } else {
        QFileInfo fileInfo(filePath);
        title = fileInfo.fileName();
    }
    
    if (editor->property("isModified").toBool()) {
        title += " *";
    }
    
    setTabText(index, title);
}