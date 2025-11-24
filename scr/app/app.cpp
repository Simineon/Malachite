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
#include "execute/executer.h"
#include "../text/CustomTextEdit.h"

App::App(QWidget *parent) : QWidget(parent) 
    , menuBar(nullptr)
    , splitter(nullptr)
    , tabWidget(nullptr)
    , fileModel(nullptr)
    , fileTree(nullptr)
    , explorerPanel(nullptr)
{
    setupUI();
    setupMenuBar();
    setupFileExplorer();
    setupConnections();

    // Создаем начальную вкладку с примером кода
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
        
        // highlighting для первой вкладки
        new Parser(firstEditor->document());
    }

    // Window settings
    setWindowTitle("Malachite IDE");
    setMinimumSize(1000, 800);
    setMaximumSize(1600, 1000);
}

void App::setupUI() {
    menuBar = new QMenuBar(this);
    
    // splitter
    splitter = new QSplitter(Qt::Horizontal, this);
    
    // Создаем Tab widget
    tabWidget = new Tab(this);
    splitter->addWidget(tabWidget);
    
    // Основной layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(menuBar);
    layout->addWidget(splitter);
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
    exitAction = runMenu->addAction(tr("E&xit"));
    
    // View Menu
    QAction *toggleSplitView = viewMenu->addAction(tr("&Toggle Split View"));
    QAction *editorOnlyView = viewMenu->addAction(tr("&Editor Only"));
    QAction *panelOnlyView = viewMenu->addAction(tr("&Panel Only"));
    
    toggleSplitView->setShortcut(QKeySequence("Ctrl+\\"));
    editorOnlyView->setShortcut(QKeySequence("Ctrl+1"));
    panelOnlyView->setShortcut(QKeySequence("Ctrl+2"));

    // Window Menu - настраиваем через Tab класс
    tabWidget->setupWindowMenu(windowMenu);
    
    // Connect Actions
    connect(newAction, &QAction::triggered, this, &App::newFile);
    connect(openAction, &QAction::triggered, this, &App::openFile);
    connect(saveAction, &QAction::triggered, this, &App::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &App::saveAsFile);
    connect(closeTabAction, &QAction::triggered, tabWidget, &Tab::closeCurrentTab);
    connect(exitAction, &QAction::triggered, this, &App::exitApp);
    connect(runCurrentFile, &QAction::triggered, this, &App::executePy);
    
    // Connect для обновления заголовка окна при смене вкладки - ИСПРАВЛЕНО
    connect(tabWidget, &Tab::currentChanged, this, &App::updateWindowTitle);
}

void App::setupFileExplorer() {
    // left panel - explorer
    explorerPanel = new QWidget(this); // Теперь это член класса
    explorerPanel->setStyleSheet("background-color: #262626; border: 1px solid #212121;");
    
    QVBoxLayout *leftLayout = new QVBoxLayout(explorerPanel);
    
    QLabel *explorerLabel = new QLabel("File Explorer");
    explorerLabel->setStyleSheet("font-weight: bold; padding: 5px; background-color: #161616; color: white;");
    explorerLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(explorerLabel);
    
    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::homePath());
    
    fileTree = new QTreeView(this);
    fileTree->setModel(fileModel);
    fileTree->setRootIndex(fileModel->index(QDir::homePath()));
    fileTree->setAnimated(false);
    fileTree->setIndentation(15);
    fileTree->setSortingEnabled(true);
    
    // column settings
    fileTree->setHeaderHidden(false);
    fileTree->setColumnHidden(1, true); // Unshow Size column
    fileTree->setColumnHidden(2, true); // Unshow Type column
    fileTree->setColumnHidden(3, true); // Unshow Date Modified column
    
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
    
    QWidget *toolbar = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(5, 2, 5, 2);
    
    QPushButton *openFolderBtn = new QPushButton("Open Folder");
    QPushButton *newFileBtn = new QPushButton("New File");
    QPushButton *newFolderBtn = new QPushButton("New Folder");
    
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
    
    splitter->insertWidget(0, explorerPanel);
    
    // Обновляем stretch factors
    splitter->setStretchFactor(0, 3); // explorerPanel 
    splitter->setStretchFactor(1, 7); // tabWidget 
    
    splitter->setChildrenCollapsible(false);
}

void App::setupConnections() {
    connect(fileTree, &QTreeView::doubleClicked, [this](const QModelIndex &index) {
        if (fileModel->isDir(index)) {
            return; 
        }
        
        QString filePath = fileModel->filePath(index);
        this->openFileInTab(filePath);
    });
    
    // Ищем кнопки в toolbar - ИСПРАВЛЕНО
    QWidget *toolbar = explorerPanel->findChild<QWidget*>();
    if (toolbar) {
        QPushButton *openFolderBtn = toolbar->findChild<QPushButton*>();
        if (openFolderBtn) {
            connect(openFolderBtn, &QPushButton::clicked, [this]() {
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
        }
    }
}

void App::newFile() {
    tabWidget->newTab();
}

void App::openFile() {
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open file", "", "Python files (*.py);;Text files (*.txt);;All files (*)");

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
    
    // Проверяем, есть ли несохраненные изменения
    if (!editor->property("isModified").toBool()) {
        return; // Файл не изменялся, не нужно сохранять
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
        
        // Обновляем свойства вкладки
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
        setWindowTitle("Malachite IDE - " + filePath);
    }
}

void App::executePy() {
    CustomTextEdit *editor = tabWidget->getCurrentEditor();
    if (!editor) return;
    
    // Сохраняем файл только если он был изменен
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

CustomTextEdit* App::createEditor() {
    return tabWidget->createEditor();
}

CustomTextEdit* App::getCurrentEditor() {
    return tabWidget->getCurrentEditor();
}

QString App::getCurrentFilePath() {
    return tabWidget->getCurrentFilePath();
}