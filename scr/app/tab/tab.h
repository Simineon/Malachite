#ifndef TAB_H
#define TAB_H

#include <QTabWidget>
#include <QAction>
#include <QMenu>
#include "../../text/CustomTextEdit.h"
#include "../../parser/parser.h"

class Tab : public QTabWidget
{
    Q_OBJECT

public:
    explicit Tab(QWidget *parent = nullptr);
    
    // File operations
    void newTab();
    void openFileInTab(const QString &filePath);
    void saveTabContent(CustomTextEdit *editor, const QString &filePath);
    void closeTab(int index);
    
    // Tab navigation
    void nextTab();
    void prevTab();
    
    // Editor management
    CustomTextEdit* createEditor();
    CustomTextEdit* getCurrentEditor();
    QString getCurrentFilePath();
    
    // Menu setup
    void setupWindowMenu(QMenu *windowMenu);

public slots:
    void closeCurrentTab();
    void onTabChanged(int index);
    void onEditorTextChanged(CustomTextEdit *editor, const QString &originalContent);
    void updateTabTitle(int index);

signals:
    void requestSaveAs(); // Добавляем этот сигнал
    void currentTabChanged(); // Добавляем этот сигнал

private:
    void setupTabWidget();
    void setupActions();
    
    // Actions
    QAction *nextTabAction;
    QAction *prevTabAction;
    QAction *newTabAction;
    QAction *closeTabAction;
};

#endif // TAB_H