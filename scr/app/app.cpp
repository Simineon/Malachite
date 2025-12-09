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
#include <QCloseEvent>
#include <QStatusBar>
#include <QMenu>
#include "../parser/parser.h"
#include "execute/executer.h"
#include "../text/CustomTextEdit.h"

App::App(QWidget *parent) 
    : QWidget(parent)
    , menuBar(nullptr)
    , splitter(nullptr)
    , tabWidget(nullptr)
    , fileModel(nullptr)
    , fileTree(nullptr)
    , explorerPanel(nullptr)
    , statusBar(nullptr)
    , miniWindow(nullptr)
{
    setupUI();
    setupMenuBar();
    setupFileExplorer();
    setupConnections();
    setupContextMenu();
    setupStatusBar(); 

    tabWidget->newTab();
    CustomTextEdit *firstEditor = tabWidget->getCurrentEditor();
    
    if (firstEditor) {
        firstEditor->setPlainText(
            "def func():\n"
            "    # Say hello\n"
            "    print('Hello, Malachite IDE!')\n"
            "\n"
            "func()\n"
        );
        
        // highlighting 
        new Parser(firstEditor->document());

        // new context menu
        setupContextMenu();
    }

    // Window settings
    setWindowTitle("Malachite IDE");
    setMinimumSize(800, 600);
    resize(1000, 700);
}

void App::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    menuBar = new QMenuBar(this);
    
    // splitter
    splitter = new QSplitter(Qt::Horizontal, this);
    
    // Tab widget
    tabWidget = new Tab(this);
    splitter->addWidget(tabWidget);

    statusBar = new QStatusBar(this);

    // Add in layout
    layout->addWidget(menuBar);
    layout->addWidget(splitter, 1);
    layout->addWidget(statusBar);
}

void App::setupContextMenu() {
    CustomTextEdit *editor = tabWidget->getCurrentEditor();
    if (!editor) return;
    
    QMenu *contextMenu = new QMenu(editor);

    contextMenu->setStyleSheet(
        "QMenu {"
        "    background-color: #252526;"
        "    color: #cccccc;"
        "    border: 1px solid #454545;"
        "    padding: 4px;"
        "    border-radius: 4px;"
        "}"
        "QMenu::item {"
        "    padding: 6px 24px 6px 12px;"
        "    margin: 2px 4px;"
        "    border-radius: 4px;"
        "    border: none;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #306442;"
        "    color: #ffffff;"
        "}"
        "QMenu::item:disabled {"
        "    color: #666666;"
        "}"
        "QMenu::separator {"
        "    height: 1px;"
        "    background-color: #3e3e42;"
        "    margin: 4px 8px;"
        "}"
    );
    
    // simple actions
    QAction *undoAction = contextMenu->addAction("Undo");
    QAction *redoAction = contextMenu->addAction("Redo");
    contextMenu->addSeparator();
    QAction *cutAction = contextMenu->addAction("Cut");
    QAction *copyAction = contextMenu->addAction("Copy");
    QAction *pasteAction = contextMenu->addAction("Paste");
    contextMenu->addSeparator();
    
    // dont think about it!
    QAction *searchAction = contextMenu->addAction("Search in Engine");
    
    // connecting
    connect(undoAction, &QAction::triggered, editor, &CustomTextEdit::undo);
    connect(redoAction, &QAction::triggered, editor, &CustomTextEdit::redo);
    connect(cutAction, &QAction::triggered, editor, &CustomTextEdit::cut);
    connect(copyAction, &QAction::triggered, editor, &CustomTextEdit::copy);
    connect(pasteAction, &QAction::triggered, editor, &CustomTextEdit::paste);
    
    // connect search action 
    connect(searchAction, &QAction::triggered, this, [this]() {
        if (!miniWindow) {
            miniWindow = new QDialog(this);
            miniWindow->setWindowTitle("Search Engine");
            miniWindow->setFixedSize(1000, 800);

            QVBoxLayout *layout = new QVBoxLayout(miniWindow);
            QLabel *label = new QLabel("Here will our search engine", miniWindow);
            QPushButton *closeButton = new QPushButton("close", miniWindow);

            layout->addWidget(label);
            layout->addWidget(closeButton);

            connect(closeButton, &QPushButton::clicked, 
                miniWindow, &QDialog::close);
        }

        miniWindow->show();
        qDebug() << "Search option clicked";
    });
    
    // set context menu for Text edit
    editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(editor, &CustomTextEdit::customContextMenuRequested,
            [editor, contextMenu](const QPoint &pos) {
                contextMenu->exec(editor->mapToGlobal(pos));
            });
    
    connect(tabWidget, &Tab::currentChanged, this, [this]() {
        setupContextMenu();
    });
}

void App::setupStatusBar() {
    if (!statusBar) return;
    
    // Main style
    statusBar->setStyleSheet("QStatusBar {background-color: #306442;}");

    // Clearing status bar
    statusBar->clearMessage();
    
    lineLabel = new QLabel("Ln: 1, Col: 1", statusBar);
    indentLabel = new QLabel("Indent: Spaces", statusBar);

    lineLabel->setStyleSheet("QLabel { padding: 0 8px; border: none; }");
    indentLabel->setStyleSheet("QLabel { padding: 0 8px; border: none; border-left: 1px solid #cbcbcb; }");
    
    statusBar->addPermanentWidget(lineLabel);
    statusBar->addPermanentWidget(indentLabel);
    
    // Show cursor info
    updateCursorInfo();
}

void App::updateCursorInfo() {
    CustomTextEdit *editor = tabWidget->getCurrentEditor();
    if (!editor) {
        lineLabel->setText("Ln: -, Col: -");
        indentLabel->setText("Indent: -");
        return;
    }
    
    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.positionInBlock() + 1;
    
    // Updating information in Status bar
    lineLabel->setText(QString("Ln: %1, Col: %2").arg(line).arg(column));
    
    QTextBlock block = cursor.block();
    QString lineText = block.text();
    int indentLevel = 0;
    
    while (indentLevel < lineText.length() && lineText.at(indentLevel).isSpace()) {
        indentLevel++;
    }
    
    // tabs or spaces
    QString indentType = "Spaces";
    if (indentLevel > 0 && lineText.at(0) == '\t') {
        indentType = "Tabs";
    }
    
    // Show informations 
    indentLabel->setText(QString("Indent: %1 (%2)").arg(indentType).arg(indentLevel));
}

void App::setupMenuBar() {
    // Menus
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *runMenu = menuBar->addMenu(tr("&Run"));
    QMenu *viewMenu = menuBar->addMenu(tr("&View")); 
    QMenu *windowMenu = menuBar->addMenu(tr("&Window"));
    
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
    QAction *exitRunAction = runMenu->addAction(tr("E&xit"));
    
    // View Menu
    QAction *toggleSplitViewAction = viewMenu->addAction(tr("&Toggle Split View"));
    QAction *editorOnlyViewAction = viewMenu->addAction(tr("&Editor Only"));
    QAction *panelOnlyViewAction = viewMenu->addAction(tr("&Panel Only"));
    
    toggleSplitViewAction->setShortcut(QKeySequence("Ctrl+\\"));
    editorOnlyViewAction->setShortcut(QKeySequence("Ctrl+1"));
    panelOnlyViewAction->setShortcut(QKeySequence("Ctrl+2"));

    // Window Menu 
    tabWidget->setupWindowMenu(windowMenu);
    
    // Connect Actions
    connect(newAction, &QAction::triggered, this, &App::newFile);
    connect(openAction, &QAction::triggered, this, &App::openFile);
    connect(saveAction, &QAction::triggered, this, &App::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &App::saveAsFile);
    connect(closeTabAction, &QAction::triggered, tabWidget, &Tab::closeCurrentTab);
    connect(exitAction, &QAction::triggered, this, &App::exitApp);
    connect(exitRunAction, &QAction::triggered, this, &App::exitApp);
    connect(runCurrentFile, &QAction::triggered, this, &App::executePy);
    
    // Connect for update window title
    connect(tabWidget, &Tab::currentChanged, this, &App::updateWindowTitle);
    
    // Connect for update cursor info
    connect(tabWidget, &Tab::currentChanged, this, [this]() {
        updateCursorInfo();
    });
    
    // Connect for changing appearans 
    connect(toggleSplitViewAction, &QAction::triggered, this, &App::toggleSplitView);
    connect(editorOnlyViewAction, &QAction::triggered, this, &App::showEditorOnly);
    connect(panelOnlyViewAction, &QAction::triggered, this, &App::showPanelOnly);
}

void App::setupFileExplorer() {
    // left panel - explorer
    explorerPanel = new QWidget(this);
    
    QString panelStyle = 
        "QWidget {"
        "    background-color: #2d2d30;"
        "    border: 1px solid #1e1e1e;"
        "}"
        "QLabel {"
        "    font-weight: bold;"
        "    padding: 8px;"
        "    background-color: #252526;"
        "    color: #cccccc;"
        "    border-bottom: 1px solid #1e1e1e;"
        "}"
        "QPushButton {"
        "    background-color: #306442;"
        "    color: white;"
        "    border: none;"
        "    padding: 6px 12px;"
        "    border-radius: 4px;"
        "    font-size: 12px;"
        "    min-height: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #499764ff;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #306442;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #464647;"
        "    color: #969696;"
        "}";
    
    explorerPanel->setStyleSheet(panelStyle);
    
    QVBoxLayout *leftLayout = new QVBoxLayout(explorerPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    
    QLabel *explorerLabel = new QLabel("Explorer");
    explorerLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(explorerLabel);
    
    // Toolbar
    QWidget *toolbar = new QWidget(this);
    toolbar->setStyleSheet("background-color: #333337; padding: 4px;");
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(4, 4, 4, 4);
    toolbarLayout->setSpacing(4);
    
    QPushButton *openFolderBtn = new QPushButton("Open Folder");
    QPushButton *newFileBtn = new QPushButton("New File");
    QPushButton *newFolderBtn = new QPushButton("New Folder");
    QPushButton *refreshBtn = new QPushButton("Refresh");
    
    openFolderBtn->setFixedHeight(28);
    newFileBtn->setFixedHeight(28);
    newFolderBtn->setFixedHeight(28);
    refreshBtn->setFixedHeight(28);
    
    toolbarLayout->addWidget(openFolderBtn);
    toolbarLayout->addWidget(newFileBtn);
    toolbarLayout->addWidget(newFolderBtn);
    toolbarLayout->addWidget(refreshBtn);
    toolbarLayout->addStretch();
    
    leftLayout->addWidget(toolbar);
    
    // File system model and tree view
    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::homePath());
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    fileModel->setNameFilters(QStringList() << "*.py" << "*.txt" << "*.md" << "*.json" << "*.xml");
    fileModel->setNameFilterDisables(false);
    
    fileTree = new QTreeView(this);
    fileTree->setModel(fileModel);
    fileTree->setRootIndex(fileModel->index(QDir::homePath()));
    fileTree->setAnimated(true);
    fileTree->setIndentation(20);
    fileTree->setSortingEnabled(true);
    
    // Column settings
    fileTree->setHeaderHidden(false);
    fileTree->setColumnHidden(1, true); // Hide Size column
    fileTree->setColumnHidden(2, true); // Hide Type column
    fileTree->setColumnHidden(3, true); // Hide Date Modified column
    
    // Стили для tree view
    fileTree->setStyleSheet(
        "QTreeView {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: none;"
        "    outline: 0;"
        "    font-size: 12px;"
        "}"
        "QTreeView::item {"
        "    padding: 4px;"
        "    border: none;"
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
        "    padding: 6px;"
        "    border: 1px solid #3e3e42;"
        "    font-weight: bold;"
        "}"
    );
    
    leftLayout->addWidget(fileTree, 1);
    
    splitter->insertWidget(0, explorerPanel);
    
    QList<int> sizes;
    sizes << 280 << 720;
    splitter->setSizes(sizes);
    
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(2);
    
    connect(openFolderBtn, &QPushButton::clicked, this, &App::openFolder);
    connect(newFileBtn, &QPushButton::clicked, this, &App::createNewFileInExplorer);
    connect(newFolderBtn, &QPushButton::clicked, this, &App::createNewFolderInExplorer);
    connect(refreshBtn, &QPushButton::clicked, this, &App::refreshFileExplorer);
}

void App::setupConnections() {
    connect(fileTree, &QTreeView::doubleClicked, this, &App::onFileDoubleClicked);
    
    // Connect | here we following for cursor in all editors
    connect(tabWidget, &Tab::currentChanged, this, [this]() {
        if (currentEditorCursorConnection) {
            disconnect(currentEditorCursorConnection);
        }
        
        // connect with new editor
        CustomTextEdit *editor = tabWidget->getCurrentEditor();
        if (editor) {
            currentEditorCursorConnection = connect(editor, &CustomTextEdit::cursorPositionChanged,
                                                  this, &App::updateCursorInfo);
        }
        
        updateCursorInfo();
    });
}

void App::onFileDoubleClicked(const QModelIndex &index) {
    if (fileModel->isDir(index)) {
        return; 
    }
    
    QString filePath = fileModel->filePath(index);
    this->openFileInTab(filePath);
}

void App::openFolder() {
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select Folder",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!folderPath.isEmpty()) {
        fileTree->setRootIndex(fileModel->index(folderPath));
    }
}

void App::createNewFileInExplorer() {
    QModelIndex currentIndex = fileTree->currentIndex();
    QString parentDir = currentIndex.isValid() ? fileModel->filePath(currentIndex) : fileModel->rootPath();
    
    if (!fileModel->isDir(currentIndex) && currentIndex.isValid()) {
        parentDir = fileModel->filePath(currentIndex.parent());
    }
    
    bool ok;
    QString fileName = QInputDialog::getText(this, "New File", 
                                           "Enter file name:", 
                                           QLineEdit::Normal, 
                                           "new_file.py", 
                                           &ok);
    
    if (ok && !fileName.isEmpty()) {
        QString filePath = QDir(parentDir).filePath(fileName);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            refreshFileExplorer();
        } else {
            QMessageBox::warning(this, "Error", "Could not create file: " + file.errorString());
        }
    }
}

void App::createNewFolderInExplorer() {
    QModelIndex currentIndex = fileTree->currentIndex();
    QString parentDir = currentIndex.isValid() ? fileModel->filePath(currentIndex) : fileModel->rootPath();
    
    if (!fileModel->isDir(currentIndex) && currentIndex.isValid()) {
        parentDir = fileModel->filePath(currentIndex.parent());
    }
    
    bool ok;
    QString folderName = QInputDialog::getText(this, "New Folder", 
                                             "Enter folder name:", 
                                             QLineEdit::Normal, 
                                             "NewFolder", 
                                             &ok);
    
    if (ok && !folderName.isEmpty()) {
        QDir dir(parentDir);
        if (dir.mkdir(folderName)) {
            refreshFileExplorer();
        } else {
            QMessageBox::warning(this, "Error", "Could not create folder");
        }
    }
}

void App::refreshFileExplorer() {
    QModelIndex currentRoot = fileTree->rootIndex();
    QString currentPath = fileModel->filePath(currentRoot);
    fileModel->setRootPath("");
    fileModel->setRootPath(currentPath);
}

void App::toggleSplitView() {
    if (explorerPanel->isVisible()) {
        explorerPanel->hide();
    } else {
        explorerPanel->show();
    }
}

void App::showEditorOnly() {
    explorerPanel->hide();
}

void App::showPanelOnly() {
    explorerPanel->show();
    tabWidget->hide();
    
    QTimer::singleShot(100, [this]() {
        tabWidget->show();
    });
}

void App::newFile() {
    tabWidget->newTab();
}

void App::openFile() {
    QString filePath = QFileDialog::getOpenFileName(
        this, 
        "Open file", 
        QDir::homePath(), 
        "Python files (*.py);;Text files (*.txt);;All files (*)"
    );

    if (!filePath.isEmpty()) {
        openFileInTab(filePath);
    }
}

void App::openFileInTab(const QString &filePath) {
    tabWidget->openFileInTab(filePath);
    updateWindowTitle();
}

void App::saveFile() {
    CustomTextEdit *editor = tabWidget->getCurrentEditor();
    if (!editor) return;
    
    // checking 
    if (!editor->property("isModified").toBool()) {
        return; // File havent changes - we dont saving
    }
    
    QString filePath = editor->property("filePath").toString();
    
    if (filePath.isEmpty()) {
        saveAsFile();
    } else {
        tabWidget->saveTabContent(editor, filePath);
    }
}

void App::saveAsFile() {
    CustomTextEdit *editor = tabWidget->getCurrentEditor();
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
        tabWidget->saveTabContent(editor, filePath);
        
        // updating
        editor->setProperty("filePath", filePath);
        editor->setProperty("isModified", false);
        editor->setProperty("originalContent", editor->toPlainText());
        
        tabWidget->updateTabTitle(tabWidget->currentIndex());
        updateWindowTitle();
    }
}

void App::updateWindowTitle() {
    QString filePath = tabWidget->getCurrentFilePath();
    if (filePath.isEmpty()) {
        setWindowTitle("Malachite IDE - untitled.py");
    } else {
        QFileInfo fileInfo(filePath);
        setWindowTitle("Malachite IDE - " + fileInfo.fileName() + " [" + fileInfo.path() + "]");
    }
}

void App::executePy() {
    CustomTextEdit *editor = tabWidget->getCurrentEditor();
    if (!editor) return;
    
    // save if file is changed
    if (editor->property("isModified").toBool()) {
        saveFile();
    }
    
    QString filePath = tabWidget->getCurrentFilePath();
    if (!filePath.isEmpty()) {
        Executer::executePy(filePath, this);
    } else {
        QMessageBox::warning(this, "Error", "No file to execute!");
    }
}

void App::exitApp() {
    close();
}

void App::closeEvent(QCloseEvent *event) {
    // Check all tabs for unsaved changes
    bool hasUnsavedChanges = false;
    
    for (int i = 0; i < tabWidget->count(); ++i) {
        CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(tabWidget->widget(i));
        if (editor && editor->property("isModified").toBool()) {
            hasUnsavedChanges = true;
            break;
        }
    }
    
    if (hasUnsavedChanges) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Save changes", 
                                    "There are unsaved changes. Do you want to save before exiting?",
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            // save all changed tabs
            for (int i = 0; i < tabWidget->count(); ++i) {
                CustomTextEdit *editor = qobject_cast<CustomTextEdit*>(tabWidget->widget(i));
                if (editor && editor->property("isModified").toBool()) {
                    tabWidget->setCurrentIndex(i);
                    QString filePath = editor->property("filePath").toString();
                    if (filePath.isEmpty()) {
                        // For files without path we give "Save as"
                        QString newFilePath = QFileDialog::getSaveFileName(
                            this,
                            "Save file",
                            QDir::homePath(),
                            "Python files (*.py);;Text files (*.txt);;All files (*)"
                        );
                        if (!newFilePath.isEmpty()) {
                            tabWidget->saveTabContent(editor, newFilePath);
                        }
                    } else {
                        tabWidget->saveTabContent(editor, filePath);
                    }
                }
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    
    event->accept();
}

CustomTextEdit* App::createEditor() {
    return tabWidget->createEditor();
}

CustomTextEdit* App::getCurrentEditor() {
    return tabWidget->getCurrentEditor();
}

QString App::getCurrentFilePath() {
    return tabWidget->getCurrentFilePath();
}