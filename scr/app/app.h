#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QTabWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include "../text/CustomTextEdit.h"

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
    void executePy();
    void exitApp();
    void openFileFromExplorer(const QString &filePath);
    void refreshFileModel(QFileSystemModel *fileModel, QTreeView *fileTree);
    
    // Новые слоты для управления вкладками
    void newTab();
    void closeTab(int index);
    void closeCurrentTab();
    void nextTab();
    void prevTab();
    void onTabChanged(int index);
    void onEditorTextChanged(CustomTextEdit *editor, const QString &originalName);

private:
    void openFileInTab(const QString &filePath);
    CustomTextEdit* createEditor();
    CustomTextEdit* getCurrentEditor();
    QString getCurrentFilePath();
    void saveTabContent(CustomTextEdit *editor, const QString &filePath);
    void updateTabTitle(int index);
    void updateWindowTitle();

private:
    QTabWidget *tabWidget;
    QString currentFilePath; // Для обратной совместимости
};

#endif // APP_H