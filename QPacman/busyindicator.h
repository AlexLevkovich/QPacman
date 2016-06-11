/********************************************************************************
** Initial source is from somewhere from Internet
** Modified by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef BUSYINDICATOR_H
#define BUSYINDICATOR_H
 
#include <QDeclarativeItem>
 
class BusyIndicator : public QDeclarativeItem {
    Q_OBJECT
    Q_PROPERTY( double innerRadius READ innerRadius WRITE setInnerRadius NOTIFY innerRadiusChanged )
    Q_PROPERTY( double outerRadius READ outerRadius WRITE setOuterRadius NOTIFY outerRadiusChanged )
    Q_PROPERTY( QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged )
    Q_PROPERTY( QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged )
    Q_PROPERTY( double actualInnerRadius READ actualInnerRadius NOTIFY actualInnerRadiusChanged )
    Q_PROPERTY( double actualOuterRadius READ actualOuterRadius NOTIFY actualOuterRadiusChanged )
 
public:
    explicit BusyIndicator( QDeclarativeItem* parent = 0 );
 
    void setInnerRadius( const double& innerRadius );
    double innerRadius() const;
 
    void setOuterRadius( const double& outerRadius );
    double outerRadius() const;
 
    void setBackgroundColor( const QColor& color );
    QColor backgroundColor() const;
 
    void setForegroundColor( const QColor& color );
    QColor foregroundColor() const;
 
    double actualInnerRadius() const;
    double actualOuterRadius() const;
 
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );
 
signals:
    void innerRadiusChanged();
    void outerRadiusChanged();
    void backgroundColorChanged();
    void foregroundColorChanged();
    void actualInnerRadiusChanged();
    void actualOuterRadiusChanged();
 
protected slots:
    virtual void updateSpinner();
 
private:
    // User settable properties
    double m_innerRadius; // In range (0, m_outerRadius]
    double m_outerRadius; // (m_innerRadius, 1]
    QColor m_backgroundColor;
    QColor m_foregroundColor;
 
    // The calculated size, inner and outer radii
    double m_size;
    double m_actualInnerRadius;
    double m_actualOuterRadius;
 
    QString m_cacheKey;
};
 
#endif // BUSYINDICATOR_H
