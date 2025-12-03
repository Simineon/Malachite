#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QMenuBar>
#include <QSplitter>
#include <QFileSystemModel>
#include <QTreeView>
#include <QModelIndex>
#include <QStatusBar>
#include <QLabel>
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
    void updateCursorInfo();
    
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
    void setupStatusBar();

    QMenuBar *menuBar;
    QSplitter *splitter;
    Tab *tabWidget;
    QFileSystemModel *fileModel;
    QTreeView *fileTree;
    QWidget *explorerPanel;
    QStatusBar *statusBar;
    QLabel *lineLabel;
    QLabel *indentLabel;
    QMetaObject::Connection currentEditorCursorConnection;
};

#endif // APP_H
