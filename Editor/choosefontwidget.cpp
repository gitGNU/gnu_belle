#include "choosefontwidget.h"

#include <QFontDatabase>
#include <QStandardItemModel>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>

static QStringList mFontFamilies;

ChooseFontWidget::ChooseFontWidget(QWidget *parent) :
    QComboBox(parent)
{
    mPreviousIndex = 0;
    loadFonts();
    connect(this, SIGNAL(activated(int)), this, SLOT(onItemActivated(int)));
}

void ChooseFontWidget::setCurrentFontFamily(const QString& family)
{
    if (mFontFamilies.contains(family)) {
        setCurrentIndex(mFontFamilies.indexOf(family));
        mPreviousIndex = currentIndex();
    }
}

void ChooseFontWidget::loadFonts()
{
    if (mFontFamilies.isEmpty()) {
        QList<QFontDatabase::WritingSystem> writingSystems = mFontDatabase.writingSystems();
        QStringList families;
        for(int i=0; i < writingSystems.size(); i++) {
            families = mFontDatabase.families(writingSystems[i]);
            for(int j=0; j < families.size(); j++) {
                if (! mFontFamilies.contains(families[j]))
                    mFontFamilies.append(families[j]);
            }
        }

        mFontFamilies.sort();
    }

    addItems(mFontFamilies);
    addCustomFontItem();
}

void ChooseFontWidget::addCustomFontItem()
{
    addItem(QIcon(":/media/document_open.png"), tr("Custom Font"), "CustomFont");
}

void ChooseFontWidget::onItemActivated(int index)
{
    if (index == count()-1) {
        QString filePath = QFileDialog::getOpenFileName(0, tr("Choose a custom font"), QString(), QString("Fonts (*.ttf *.otf)"));
        QStringList families;

        if (! filePath.isEmpty()) {
            int id = QFontDatabase::addApplicationFont(filePath);
            if (id != -1) {
                mCustomFontsIds.append(id);
                families = QFontDatabase::applicationFontFamilies(id);

                for(int i=0; i < families.size(); i++)
                    if (! mFontFamilies.contains(families[i]))
                        mFontFamilies.append(families[i]);
                mFontFamilies.sort();

                this->blockSignals(true);
                clear();
                addItems(mFontFamilies);
                addCustomFontItem();
                this->blockSignals(false);
            }
        }

        if (families.isEmpty())
            setCurrentIndex(mPreviousIndex);
        else {
            setCurrentFontFamily(families.first());
            emit fontChosen(mFontFamilies[currentIndex()]);
        }
    }
    else {
        mPreviousIndex = index;
        emit fontChosen(mFontFamilies[currentIndex()]);
    }
}
