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
#include "../parser/parser.h"

App::App(QWidget *parent) : QWidget(parent) {
    QMenuBar *menuBar = new QMenuBar(this);
    
    // Create splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    
    // Создаем левую панель с проводником
    QWidget *explorerPanel = new QWidget(this);
    explorerPanel->setStyleSheet("background-color: #262626; border: 1px solid #212121;");
    
    QVBoxLayout *leftLayout = new QVBoxLayout(explorerPanel);
    
    QLabel *explorerLabel = new QLabel("File Explorer");
    explorerLabel->setStyleSheet("font-weight: bold; padding: 5px; background-color: #161616; color: white;");
    explorerLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(explorerLabel);
    
    // Создаем проводник файлов
    QFileSystemModel *fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::homePath());
    
    QTreeView *fileTree = new QTreeView(this);
    fileTree->setModel(fileModel);
    fileTree->setRootIndex(fileModel->index(QDir::homePath()));
    fileTree->setAnimated(false);
    fileTree->setIndentation(15);
    fileTree->setSortingEnabled(true);
    
    // Настраиваем колонки - ОСТАВЛЯЕМ ТОЛЬКО НАЗВАНИЕ
    fileTree->setHeaderHidden(false);
    fileTree->setColumnHidden(1, true); // Скрываем колонку Размера
    fileTree->setColumnHidden(2, true); // Скрываем колонку Типа
    fileTree->setColumnHidden(3, true); // Скрываем колонку Дата изменения
    
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
    
    // Панель инструментов проводника - ОБНОВЛЕНА
    QWidget *toolbar = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(5, 2, 5, 2);
    
    // НОВАЯ КНОПКА: Открыть папку
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
    
    editor = new CustomTextEdit(this);
    
    splitter->addWidget(explorerPanel);
    splitter->addWidget(editor);     
    
    // Обновляем stretch factors
    splitter->setStretchFactor(0, 3); // explorerPanel 
    splitter->setStretchFactor(1, 7); // editor 
    
    splitter->setChildrenCollapsible(false);
    
    // Menus
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *runMenu = menuBar->addMenu(tr("&Run"));
    QMenu *viewMenu = menuBar->addMenu(tr("&View")); 
    
    // File Menu
    QAction *newAction = fileMenu->addAction(tr("&New"));
    QAction *openAction = fileMenu->addAction(tr("&Open"));
    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));

    newAction->setShortcut(QKeySequence::New);
    openAction->setShortcut(QKeySequence::Open);
    saveAction->setShortcut(QKeySequence::Save);

    // Run Menu 
    QAction *runCurrentFile = runMenu->addAction(tr("&Run current file and save"));
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

    // Connect Actions
    connect(newAction, &QAction::triggered, this, &App::newFile);
    connect(openAction, &QAction::triggered, this, &App::openFile);
    connect(saveAction, &QAction::triggered, this, &App::saveFile);
    connect(exitAction, &QAction::triggered, this, &App::exitApp);
    connect(runCurrentFile, &QAction::triggered, this, &App::executePy);
    
    // Connect View actions 
    connect(toggleSplitView, &QAction::triggered, [splitter]() {
        QWidget *explorerPanel = splitter->widget(0);
        explorerPanel->setVisible(!explorerPanel->isVisible());
    });
    
    connect(editorOnlyView, &QAction::triggered, [splitter]() {
        splitter->widget(0)->setVisible(false); // explorerPanel 
        splitter->widget(1)->setVisible(true);  // editor 
    });
    
    connect(panelOnlyView, &QAction::triggered, [splitter]() {
        splitter->widget(0)->setVisible(true);  // explorerPanel 
        splitter->widget(1)->setVisible(false); // editor 
    });
    
    // Connect проводник действий
    connect(fileTree, &QTreeView::doubleClicked, [this, fileModel](const QModelIndex &index) {
        if (fileModel->isDir(index)) {
            return; // Папки обрабатываются самим QTreeView
        }
        
        QString filePath = fileModel->filePath(index);
        if (filePath.endsWith(".py") || filePath.endsWith(".txt")) {
            this->openFileFromExplorer(filePath);
        }
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
    
    // Исправленные connect с правильным захватом this
    connect(newFileBtn, &QPushButton::clicked, this, [this, fileModel, fileTree]() {
        this->newFile();
        
        // Опционально: создать физический файл в текущей директории проводника
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
                // Автоматическое обновление модели
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
                // Автоматическое обновление модели
                refreshFileModel(fileModel, fileTree);
            }
        }
    });

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

    // Start code in the editor
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
        "\n"
        "# Добавим вызов функции для тестирования\n"
        "hello_world()\n"
    );

    // highlighting 
    Parser *highlighter = new Parser(editor->document());

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
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString fileContent = in.readAll();
        editor->setPlainText(fileContent);
        file.close();
        currentFilePath = filePath;
        setWindowTitle("Malachite IDE - " + filePath);
    } else {
        QMessageBox::warning(this, "Error", "Error in file opening!");
    }
}

void App::newFile() {
    editor->clear();
    currentFilePath.clear();
    setWindowTitle("Malachite IDE - New file");
}

void App::openFile() {
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open file", "", "Python files (*.py);;Text files (*.txt);;All files (*)");

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString fileContent = in.readAll();
            editor->setPlainText(fileContent);
            file.close();
            currentFilePath = filePath;
            setWindowTitle("Malachite IDE - " + filePath);
        } else {
            QMessageBox::warning(this, "Error", "Error in file opening!");
        }
    }
}

void App::saveFile() {
    QString fileName;
    
    if (currentFilePath.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(
            editor,
            "Save file",
            "",
            "Python files (*.py);;Text files (*.txt);;All files (*)"
        );
        if (!fileName.isEmpty()) {
            if (!fileName.endsWith(".py") && !fileName.endsWith(".txt")) {
                fileName += ".py";
            }
            currentFilePath = fileName;
        }
    } else {
        fileName = currentFilePath;
    }

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << editor->toPlainText();
            file.close();
            setWindowTitle("Malachite IDE - " + fileName);
        } else {
            QMessageBox::warning(this, "Error", "Error in file saving!");
        }
    }
}

void App::executePy() {
    saveFile();
    
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please save the file first");
        return;
    }
    
    QWidget *runnerWindow = new QWidget();
    runnerWindow->setWindowTitle("Runner - " + QFileInfo(currentFilePath).fileName());
    runnerWindow->setMinimumSize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(runnerWindow);
    
    QTextEdit *outputWidget = new QTextEdit();
    outputWidget->setReadOnly(true);
    mainLayout->addWidget(outputWidget);
    
    QWidget *inputWidget = new QWidget();
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(5, 5, 5, 5);
    
    QLabel *inputLabel = new QLabel("Input:");
    QLineEdit *inputLineEdit = new QLineEdit();
    QPushButton *sendButton = new QPushButton("Send");
    
    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputLineEdit);
    inputLayout->addWidget(sendButton);
    
    mainLayout->addWidget(inputWidget);
    
    inputWidget->setVisible(false);
    
    QProcess *process = new QProcess(runnerWindow);
    
    connect(process, &QProcess::readyReadStandardOutput, [process, outputWidget, inputWidget]() {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        
        if (output.contains(":") || output.toLower().contains("input") || output.trimmed().endsWith(":")) {
            inputWidget->setVisible(true);
        }
        
        outputWidget->append(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, [process, outputWidget]() {
        QString error = QString::fromLocal8Bit(process->readAllStandardError());
        outputWidget->append("<font color='red'>" + error + "</font>");
    });
    
    connect(sendButton, &QPushButton::clicked, [process, inputLineEdit, outputWidget, inputWidget]() {
        QString inputText = inputLineEdit->text() + "\n";
        process->write(inputText.toLocal8Bit());
        inputLineEdit->clear();
        inputWidget->setVisible(false);
    });
    
    connect(inputLineEdit, &QLineEdit::returnPressed, sendButton, &QPushButton::click);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [outputWidget, process, inputWidget, runnerWindow](int exitCode, QProcess::ExitStatus exitStatus) {
                outputWidget->append("\n----------------------------------------");
                outputWidget->append(QString("Process finished with exit code: %1").arg(exitCode));
                inputWidget->setVisible(false);
                process->deleteLater();
            });
    
    runnerWindow->show();
    outputWidget->append("File: " + currentFilePath);
    outputWidget->append("----------------------------------------\n");
    
    process->setProcessChannelMode(QProcess::MergedChannels);
    
    QStringList pythonCommands = {"python", "python3", "py"};
    bool processStarted = false;
    
    for (const QString &command : pythonCommands) {
        process->start(command, QStringList() << "-i" << currentFilePath);
        
        if (process->waitForStarted(2000)) {
            processStarted = true;
            break;
        }
    }
    
    if (!processStarted) {
        outputWidget->append("\n<font color='red'>ERROR: Could not start Python interpreter!</font>");
        outputWidget->append("Please make sure Python is installed and available in your PATH.");
        outputWidget->append("You can download Python from: https://www.python.org/downloads/");
        
        QMessageBox::critical(this, "Python Not Found", 
            "Could not find Python interpreter on your system.\n\n"
            "Please install Python and make sure it's available in your PATH environment variable.\n"
            "Download from: https://www.python.org/downloads/");
    }
}

void App::exitApp() {
    QApplication::quit();
}