#ifndef TAB_H
#define TAB_H

#include <QTabWidget>
#include <QAction>
#include <QMenu>
#include "../../parser/parser.h"
#include "../../text/CustomTextEdit.h"

class Tab : public QTabWidget
{
    Q_OBJECT

public:
    explicit Tab(QWidget *parent = nullptr);
    
    void setupWindowMenu(QMenu *windowMenu);
    CustomTextEdit* createEditor();
    CustomTextEdit* getCurrentEditor();
    QString getCurrentFilePath();
    
    void openFileInTab(const QString &filePath);
    void saveTabContent(CustomTextEdit *editor, const QString &filePath);
    void closeCurrentTab();
    void updateTabTitle(int index);

public slots:
    void newTab();
    void nextTab();
    void prevTab();
    void closeTab(int index);

signals:
    void currentTabChanged();
    void requestSaveAs();
    void cursorPositionChanged(); // Новый сигнал

private slots:
    void onTabChanged(int index);
    void onEditorTextChanged(CustomTextEdit *editor, const QString &originalContent);

private:
    void setupTabWidget();
    void setupActions();
    
    QAction *nextTabAction;
    QAction *prevTabAction;
    QAction *newTabAction;
    QAction *closeTabAction;
};

#endif // TAB_H