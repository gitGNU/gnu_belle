#ifndef ANIMATIONIMAGE_H
#define ANIMATIONIMAGE_H

#include <QObject>
#include <QMovie>
#include <QPixmap>
#include <QDir>

class AnimationImage
{
    QPixmap* mPixmap;
    QMovie* mMovie;
    QStringList mFramesNames;
    QString mFilePath;
    QString mFileName;

public:
    explicit AnimationImage(const QString& path="");
    AnimationImage(QPixmap*);

    bool hasAnimation();
    QMovie* movie();
    QPixmap* pixmap();
    bool contains(QMovie*);
    bool contains(QPixmap*);
    int width();
    int height();
    bool isNull();
    QString path();
    QStringList framesNames() const;
    QRect rect() const;
    void save(const QDir&);
    void setFileName(const QString&);
    QString fileName() const;
    
signals:
    
public slots:
    
private:
   void init();
};

#endif // ANIMATIONIMAGE_H
