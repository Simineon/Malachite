#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QMenuBar>
#include <QSplitter>
#include <QFileSystemModel>
#include <QTreeView>
#include <QModelIndex>
#include "tab/tab.h"

class App : public QWidget
{
    Q_OBJECT

public:
    explicit App(QWidget *parent = nullptr);
    CustomTextEdit* createEditor();
    CustomTextEdit* getCurrentEditor();
    QString getCurrentFilePath();

private slots:
    void newFile();
    void openFile();
    void openFileInTab(const QString &filePath);
    void saveFile();
    void saveAsFile();
    void executePy();
    void exitApp();
    void updateWindowTitle();
    
    // File Explorer slots
    void onFileDoubleClicked(const QModelIndex &index);
    void openFolder();
    void createNewFileInExplorer();
    void createNewFolderInExplorer();
    void refreshFileExplorer();
    
    // View menu slots
    void toggleSplitView();
    void showEditorOnly();
    void showPanelOnly();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupMenuBar();
    void setupFileExplorer();
    void setupConnections();

    QMenuBar *menuBar;
    QSplitter *splitter;
    Tab *tabWidget;
    QFileSystemModel *fileModel;
    QTreeView *fileTree;
    QWidget *explorerPanel;
};

#endif // APP_H