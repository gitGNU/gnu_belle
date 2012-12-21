#ifndef ANIMATIONIMAGE_H
#define ANIMATIONIMAGE_H

#include <QObject>
#include <QMovie>
#include <QPixmap>

class AnimationImage : public QPixmap
{
    QPixmap* mPixmap;
    QMovie* mMovie;

public:
    explicit AnimationImage(const QString& path="", QObject *parent = 0);
    AnimationImage(QPixmap*, QObject *parent = 0);

    bool hasAnimation();
    QMovie* movie();
    QPixmap* pixmap();
    bool contains(QMovie*);
    bool contains(QPixmap*);
    int width();
    int height();
    bool isNull();
    
signals:
    
public slots:
    
private:
   void init();
};

#endif // ANIMATIONIMAGE_H
