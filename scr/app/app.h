#ifndef APP_H
#define APP_H

#include <QWidget>
#include "../text/CustomTextEdit.h"

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
    void openFileFromExplorer(const QString &filePath);
    void executePy();
    void exitApp();

private:
    void refreshFileModel(QFileSystemModel *fileModel, QTreeView *fileTree);
    
    CustomTextEdit *editor;
    QString currentFilePath;
};

#endif // APP_H