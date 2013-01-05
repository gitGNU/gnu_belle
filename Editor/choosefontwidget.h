#ifndef CHOOSEFONTWIDGET_H
#define CHOOSEFONTWIDGET_H

#include <QComboBox>
#include <QFontDatabase>
#include <QStyledItemDelegate>

class ChooseFontWidget : public QComboBox
{
    Q_OBJECT

    QFontDatabase mFontDatabase;
    QList<int> mCustomFontsIds;
    int mPreviousIndex;

public:
    explicit ChooseFontWidget(QWidget *parent = 0);
    void loadFonts();
    void addCustomFontItem();
    void setCurrentFontFamily(const QString&);
    
signals:
    void fontChosen(const QString&);
    
public slots:
private slots:
    void onItemActivated(int);
    
};

#endif // CHOOSEFONTWIDGET_H
