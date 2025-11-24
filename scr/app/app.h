#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QTabWidget>
#include "tab/tab.h"

class CustomTextEdit;
class QMenuBar;
class QSplitter;
class QFileSystemModel;
class QTreeView;

class App : public QWidget
{
    Q_OBJECT

public:
    App(QWidget *parent = nullptr);

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void exitApp();
    void executePy();
    void updateWindowTitle();

private:
    void setupUI();
    void setupMenuBar();
    void setupFileExplorer();
    void setupConnections();
    
    CustomTextEdit* createEditor();
    CustomTextEdit* getCurrentEditor();
    QString getCurrentFilePath();
    void openFileInTab(const QString &filePath);
    void refreshFileModel(QFileSystemModel *fileModel, QTreeView *fileTree);

    // UI Components
    QMenuBar *menuBar;
    QSplitter *splitter;
    Tab *tabWidget;
    QFileSystemModel *fileModel;
    QTreeView *fileTree;
    QWidget *explorerPanel;
};

#endif // APP_H