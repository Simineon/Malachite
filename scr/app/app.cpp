#include "app.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QFrame>
#include <QTreeView>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QInputDialog>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include "../parser/parser.h"
#include "../app/execute/executer.h"

App::App(QWidget *parent) : QWidget(parent) {
    QMenuBar *menuBar = new QMenuBar(this);
    
    // splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    
    // left panel - explorer
    QWidget *explorerPanel = new QWidget(this);
    explorerPanel->setStyleSheet("background-color: #262626; border: 1px solid #212121;");
    
    QVBoxLayout *leftLayout = new QVBoxLayout(explorerPanel);
    
    QLabel *explorerLabel = new QLabel("File Explorer");
    explorerLabel->setStyleSheet("font-weight: bold; padding: 5px; background-color: #161616; color: white;");
    explorerLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(explorerLabel);
    
    QFileSystemModel *fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::homePath());
    
    QTreeView *fileTree = new QTreeView(this);
    fileTree->setModel(fileModel);
    fileTree->setRootIndex(fileModel->index(QDir::homePath()));
    fileTree->setAnimated(false);
    fileTree->setIndentation(15);
    fileTree->setSortingEnabled(true);
    
    // collumn settings
    fileTree->setHeaderHidden(false);
    fileTree->setColumnHidden(1, true); // Unshow Size column
    fileTree->setColumnHidden(2, true); // Unshow Type column
    fileTree->setColumnHidden(3, true); // Unshow Date Modified column
    
    // Стилизация проводника
    fileTree->setStyleSheet(
        "QTreeView {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: none;"
        "    outline: 0;"
        "}"
        "QTreeView::item {"
        "    padding: 2px;"
        "}"
        "QTreeView::item:hover {"
        "    background-color: #2a2d2e;"
        "}"
        "QTreeView::item:selected {"
        "    background-color: #094771;"
        "}"
        "QHeaderView::section {"
        "    background-color: #2d2d30;"
        "    color: #cccccc;"
        "    padding: 4px;"
        "    border: 1px solid #3e3e42;"
        "}"
    );
    
    // Панель инструментов проводника
    QWidget *toolbar = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(5, 2, 5, 2);
    
    QPushButton *openFolderBtn = new QPushButton("Open Folder");
    QPushButton *newFileBtn = new QPushButton("New File");
    QPushButton *newFolderBtn = new QPushButton("New Folder");
    
    // Стилизация кнопок
    QString buttonStyle = 
        "QPushButton {"
        "    background-color: #0e639c;"
        "    color: white;"
        "    border: none;"
        "    padding: 4px 8px;"
        "    border-radius: 3px;"
        "    font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1177bb;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0c547d;"
        "}";
    
    openFolderBtn->setStyleSheet(buttonStyle);
    newFileBtn->setStyleSheet(buttonStyle);
    newFolderBtn->setStyleSheet(buttonStyle);
    
    toolbarLayout->addWidget(openFolderBtn);
    toolbarLayout->addWidget(newFileBtn);
    toolbarLayout->addWidget(newFolderBtn);
    toolbarLayout->addStretch();
    
    leftLayout->addWidget(toolbar);
    leftLayout->addWidget(fileTree);
    
    // Создаем виджет с вкладками вместо одного редактора
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);
    
    // Стилизация вкладок
    tabWidget->setStyleSheet(
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
        "    image: url(close.png);"
        "    subcontrol-position: right;"
        "}"
        "QTabBar::close-button:hover {"
        "    background-color: #e81123;"
        "    border-radius: 8px;"
        "}"
    );
    
    splitter->addWidget(explorerPanel);
    splitter->addWidget(tabWidget);     
    
    // Обновляем stretch factors
    splitter->setStretchFactor(0, 3); // explorerPanel 
    splitter->setStretchFactor(1, 7); // tabWidget 
    
    splitter->setChildrenCollapsible(false);
    
    // Menus
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *runMenu = menuBar->addMenu(tr("&Run"));
    QMenu *viewMenu = menuBar->addMenu(tr("&View")); 
    QMenu *windowMenu = menuBar->addMenu(tr("&Window")); // Новое меню для управления окнами
    
    // File Menu
    QAction *newAction = fileMenu->addAction(tr("&New"));
    QAction *openAction = fileMenu->addAction(tr("&Open"));
    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    QAction *saveAsAction = fileMenu->addAction(tr("Save &As"));
    QAction *closeTabAction = fileMenu->addAction(tr("&Close Tab"));
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));

    newAction->setShortcut(QKeySequence::New);
    openAction->setShortcut(QKeySequence::Open);
    saveAction->setShortcut(QKeySequence::Save);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    closeTabAction->setShortcut(QKeySequence::Close);

    // Run Menu 
    QAction *runCurrentFile = runMenu->addAction(tr("&Run current file"));
    runCurrentFile->setShortcut(QKeySequence("F5")); 

    runMenu->addSeparator();
    exitAction = runMenu->addAction(tr("E&xit"));
    
    // View Menu
    QAction *toggleSplitView = viewMenu->addAction(tr("&Toggle Split View"));
    QAction *editorOnlyView = viewMenu->addAction(tr("&Editor Only"));
    QAction *panelOnlyView = viewMenu->addAction(tr("&Panel Only"));
    
    toggleSplitView->setShortcut(QKeySequence("Ctrl+\\"));
    editorOnlyView->setShortcut(QKeySequence("Ctrl+1"));
    panelOnlyView->setShortcut(QKeySequence("Ctrl+2"));

    // Window Menu - новые действия для управления вкладками
    QAction *nextTabAction = windowMenu->addAction(tr("&Next Tab"));
    QAction *prevTabAction = windowMenu->addAction(tr("&Previous Tab"));
    windowMenu->addSeparator();
    QAction *newTabAction = windowMenu->addAction(tr("&New Tab"));
    
    nextTabAction->setShortcut(QKeySequence("Ctrl+Tab"));
    prevTabAction->setShortcut(QKeySequence("Ctrl+Shift+Tab"));
    newTabAction->setShortcut(QKeySequence::AddTab);

    // Connect Actions
    connect(newAction, &QAction::triggered, this, &App::newFile);
    connect(openAction, &QAction::triggered, this, &App::openFile);
    connect(saveAction, &QAction::triggered, this, &App::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &App::saveAsFile);
    connect(closeTabAction, &QAction::triggered, this, &App::closeCurrentTab);
    connect(exitAction, &QAction::triggered, this, &App::exitApp);
    connect(runCurrentFile, &QAction::triggered, this, &App::executePy);
    
    // Connect Window actions
    connect(nextTabAction, &QAction::triggered, this, &App::nextTab);
    connect(prevTabAction, &QAction::triggered, this, &App::prevTab);
    connect(newTabAction, &QAction::triggered, this, &App::newTab);
    
    // Connect View actions 
    connect(toggleSplitView, &QAction::triggered, [splitter]() {
        QWidget *explorerPanel = splitter->widget(0);
        explorerPanel->setVisible(!explorerPanel->isVisible());
    });
    
    connect(editorOnlyView, &QAction::triggered, [splitter]() {
        splitter->widget(0)->setVisible(false); // explorerPanel 
        splitter->widget(1)->setVisible(true);  // tabWidget 
    });
    
    connect(panelOnlyView, &QAction::triggered, [splitter]() {
        splitter->widget(0)->setVisible(true);  // explorerPanel 
        splitter->widget(1)->setVisible(false); // tabWidget 
    });
    
    // Connect проводник действий
    connect(fileTree, &QTreeView::doubleClicked, [this, fileModel](const QModelIndex &index) {
        if (fileModel->isDir(index)) {
            return; // Папки обрабатываются самим QTreeView
        }
        
        QString filePath = fileModel->filePath(index);
        this->openFileInTab(filePath);
    });
    
    connect(openFolderBtn, &QPushButton::clicked, [this, fileTree, fileModel]() {
        QString folderPath = QFileDialog::getExistingDirectory(
            this,
            "Select Folder",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        
        if (!folderPath.isEmpty()) {
            fileTree->setRootIndex(fileModel->index(folderPath));
        }
    });
    
    // Connect для создания новых файлов и папок
    connect(newFileBtn, &QPushButton::clicked, this, [this, fileModel, fileTree]() {
        this->newFile();
        
        QModelIndex currentIndex = fileTree->currentIndex();
        QString currentPath = currentIndex.isValid() ? fileModel->filePath(currentIndex) : fileModel->rootPath();
        
        QFileInfo fileInfo(currentPath);
        if (fileInfo.isFile()) {
            currentPath = fileInfo.path();
        }
        
        QString newFilePath = QFileDialog::getSaveFileName(
            this, 
            "Create New File",
            currentPath + "/untitled.py",
            "Python files (*.py);;Text files (*.txt);;All files (*)"
        );
        
        if (!newFilePath.isEmpty()) {
            QFile file(newFilePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
                refreshFileModel(fileModel, fileTree);
            }
        }
    });
    
    connect(newFolderBtn, &QPushButton::clicked, this, [this, fileModel, fileTree]() {
        QModelIndex currentIndex = fileTree->currentIndex();
        QString currentPath = currentIndex.isValid() ? fileModel->filePath(currentIndex) : fileModel->rootPath();
        
        QFileInfo fileInfo(currentPath);
        if (fileInfo.isFile()) {
            currentPath = fileInfo.path();
        }
        
        bool ok;
        QString folderName = QInputDialog::getText(
            fileTree,
            "New Folder",
            "Enter folder name:",
            QLineEdit::Normal,
            "",
            &ok
        );
        
        if (ok && !folderName.isEmpty()) {
            QDir dir(currentPath);
            if (dir.mkdir(folderName)) {
                refreshFileModel(fileModel, fileTree);
            }
        }
    });

    // Connect для закрытия вкладок
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &App::closeTab);
    
    // Connect для смены активной вкладки
    connect(tabWidget, &QTabWidget::currentChanged, this, &App::onTabChanged);

    QFileSystemWatcher *fileWatcher = new QFileSystemWatcher(this);
    
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, [this, fileModel, fileTree](const QString &path) {
        Q_UNUSED(path)
        QTimer::singleShot(100, this, [this, fileModel, fileTree]() {
            refreshFileModel(fileModel, fileTree);
        });
    });
    
    fileWatcher->addPath(fileModel->rootPath());
    
    connect(fileTree, &QTreeView::expanded, this, [fileWatcher, fileModel](const QModelIndex &index) {
        if (fileModel->isDir(index)) {
            QString dirPath = fileModel->filePath(index);
            if (!fileWatcher->directories().contains(dirPath)) {
                fileWatcher->addPath(dirPath);
            }
        }
    });
    
    connect(openFolderBtn, &QPushButton::clicked, this, [fileWatcher, fileModel, fileTree]() {
        QStringList oldDirs = fileWatcher->directories();
        if (!oldDirs.isEmpty()) {
            fileWatcher->removePaths(oldDirs);
        }
        
        QModelIndex rootIndex = fileTree->rootIndex();
        if (rootIndex.isValid()) {
            QString rootPath = fileModel->filePath(rootIndex);
            fileWatcher->addPath(rootPath);
        }
    });

    // Our main layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(menuBar);
    layout->addWidget(splitter);

    // Создаем начальную вкладку с примером кода
    newTab();
    CustomTextEdit *firstEditor = getCurrentEditor();
    if (firstEditor) {
        firstEditor->setPlainText(
            "def func():\n"
            "    # Say hello\n"
            "    print('Hello, Malachite IDE!')\n"
            "\n"
            "func()\n"
        );
        
        // highlighting для первой вкладки
        new Parser(firstEditor->document());
        
        tabWidget->setTabText(0, "untitled.py");
    }

    // Window settings
    setWindowTitle("Malachite IDE");
    setMinimumSize(1000, 800);
    setMaximumSize(1600, 1000);
}

void App::refreshFileModel(QFileSystemModel *fileModel, QTreeView *fileTree) {
    QModelIndex currentRoot = fileTree->rootIndex();
    QString currentPath = fileModel->filePath(currentRoot);
    
    fileTree->collapseAll();
    
    fileModel->setRootPath("");
    fileModel->setRootPath(currentPath);
    fileTree->setRootIndex(fileModel->index(currentPath));
    
    if (currentRoot.isValid()) {
        fileTree->setRootIndex(fileModel->index(currentPath));
    }
}

void App::openFileFromExplorer(const QString &filePath) {
    openFileInTab(filePath);
}

void App::openFileInTab(const QString &filePath) {
    // Проверяем, не открыт ли файл уже в другой вкладке
    for (int i = 0; i < tabWidget->count(); ++i) {
        CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(tabWidget->widget(i));
        if (editor && editor->property("filePath").toString() == filePath) {
            tabWidget->setCurrentIndex(i);
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
        int tabIndex = tabWidget->addTab(editor, tabName);
        tabWidget->setCurrentIndex(tabIndex);
        
        // Добавляем подсветку синтаксиса только для Python файлов
        if (filePath.endsWith(".py", Qt::CaseInsensitive)) {
            new Parser(editor->document());
        }
        
        // Connect для отслеживания изменений
        connect(editor, &CustomTextEdit::textChanged, this, [this, editor, fileContent]() {
            this->onEditorTextChanged(editor, fileContent);
        });
        
        file.close();
        updateWindowTitle();
    } else {
        QMessageBox::warning(this, "Error", "Error in file opening!");
    }
}

CustomTextEdit* App::createEditor() {
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

CustomTextEdit* App::getCurrentEditor() {
    return qobject_cast<CustomTextEdit*>(tabWidget->currentWidget());
}

QString App::getCurrentFilePath() {
    CustomTextEdit *editor = getCurrentEditor();
    if (editor) {
        return editor->property("filePath").toString();
    }
    return QString();
}

void App::newFile() {
    newTab();
}

void App::newTab() {
    CustomTextEdit *editor = createEditor();
    editor->setProperty("filePath", QString());
    editor->setProperty("isModified", false);
    editor->setProperty("originalContent", "");
    
    int tabIndex = tabWidget->addTab(editor, "untitled.py");
    tabWidget->setCurrentIndex(tabIndex);
    
    // Добавляем подсветку синтаксиса
    new Parser(editor->document());
    
    // Connect для отслеживания изменений
    connect(editor, &CustomTextEdit::textChanged, this, [this, editor]() {
        this->onEditorTextChanged(editor, "");
    });
    
    updateWindowTitle();
}

void App::openFile() {
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open file", "", "Python files (*.py);;Text files (*.txt);;All files (*)");

    if (!filePath.isEmpty()) {
        openFileInTab(filePath);
    }
}

void App::saveFile() {
    CustomTextEdit *editor = getCurrentEditor();
    if (!editor) return;
    
    // Проверяем, есть ли несохраненные изменения
    if (!editor->property("isModified").toBool()) {
        return; // Файл не изменялся, не нужно сохранять
    }
    
    QString filePath = editor->property("filePath").toString();
    
    if (filePath.isEmpty()) {
        saveAsFile();
    } else {
        saveTabContent(editor, filePath);
    }
}

void App::saveAsFile() {
    CustomTextEdit *editor = getCurrentEditor();
    if (!editor) return;
    
    QString currentPath = editor->property("filePath").toString();
    if (currentPath.isEmpty()) {
        currentPath = QDir::homePath();
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save file",
        currentPath,
        "Python files (*.py);;Text files (*.txt);;All files (*)"
    );
    
    if (!filePath.isEmpty()) {
        saveTabContent(editor, filePath);
        
        // Обновляем свойства вкладки
        editor->setProperty("filePath", filePath);
        editor->setProperty("isModified", false);
        editor->setProperty("originalContent", editor->toPlainText());
        
        QFileInfo fileInfo(filePath);
        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.fileName());
        
        updateWindowTitle();
    }
}

void App::saveTabContent(CustomTextEdit *editor, const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString content = editor->toPlainText();
        out << content;
        file.close();
        
        editor->setProperty("isModified", false);
        editor->setProperty("originalContent", content);
        updateTabTitle(tabWidget->currentIndex());
    } else {
        QMessageBox::warning(this, "Error", "Error in file saving!");
    }
}

void App::closeCurrentTab() {
    closeTab(tabWidget->currentIndex());
}

void App::closeTab(int index) {
    if (index < 0) return;
    
    CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(tabWidget->widget(index));
    if (editor && editor->property("isModified").toBool()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Save changes", 
                                    "The document has been modified. Do you want to save changes?",
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            // Сохраняем только если есть изменения
            if (editor->property("isModified").toBool()) {
                saveFile();
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    tabWidget->removeTab(index);
    
    // Если вкладок не осталось, создаем новую
    if (tabWidget->count() == 0) {
        newTab();
    }
    
    updateWindowTitle();
}

void App::nextTab() {
    int current = tabWidget->currentIndex();
    int next = (current + 1) % tabWidget->count();
    tabWidget->setCurrentIndex(next);
}

void App::prevTab() {
    int current = tabWidget->currentIndex();
    int prev = (current - 1 + tabWidget->count()) % tabWidget->count();
    tabWidget->setCurrentIndex(prev);
}

void App::onTabChanged(int index) {
    updateWindowTitle();
}

void App::onEditorTextChanged(CustomTextEdit *editor, const QString &originalContent) {
    QString currentContent = editor->toPlainText();
    bool isModified = (currentContent != originalContent);
    
    if (editor->property("isModified").toBool() != isModified) {
        editor->setProperty("isModified", isModified);
        updateTabTitle(tabWidget->indexOf(editor));
    }
}

void App::updateTabTitle(int index) {
    if (index < 0) return;
    
    CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(tabWidget->widget(index));
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
    
    tabWidget->setTabText(index, title);
}

void App::updateWindowTitle() {
    CustomTextEdit *editor = getCurrentEditor();
    if (editor) {
        QString filePath = editor->property("filePath").toString();
        if (filePath.isEmpty()) {
            setWindowTitle("Malachite IDE - untitled.py");
        } else {
            setWindowTitle("Malachite IDE - " + filePath);
        }
    } else {
        setWindowTitle("Malachite IDE");
    }
}

void App::executePy() {
    CustomTextEdit *editor = getCurrentEditor();
    if (!editor) return;
    
    // Сохраняем файл только если он был изменен
    if (editor->property("isModified").toBool()) {
        saveFile();
    }
    
    QString filePath = getCurrentFilePath();
    if (!filePath.isEmpty()) {
        Executer::executePy(filePath, this);
    } else {
        QMessageBox::warning(this, "Error", "No file to execute!");
    }
}

void App::exitApp() {
    // Check all tabs for unsaved changes
    for (int i = 0; i < tabWidget->count(); ++i) {
        CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(tabWidget->widget(i));
        if (editor && editor->property("isModified").toBool()) {
            tabWidget->setCurrentIndex(i);
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Save changes", 
                                        "There are unsaved changes. Do you want to save before exiting?",
                                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            
            if (reply == QMessageBox::Save) {
                // Сохраняем только если есть изменения
                if (editor->property("isModified").toBool()) {
                    saveFile();
                }
            } else if (reply == QMessageBox::Cancel) {
                return;
            }
        }
    }
    
    QApplication::quit();
}