
#include "qgsmapcanvasitem.h"


class QgsGeorefDataPoint : public QgsMapCanvasItem
{
  public:
  
    //! constructor
    QgsGeorefDataPoint(QgsMapCanvas* mapCanvas, int id,
                       const QgsPoint& pixelCoords,
                       const QgsPoint& mapCoords);
  
    //! draws point information
    virtual void paint(QPainter* p);
        
    //! handler for manual updating of position and size
    virtual QRectF boundingRect() const;
    
    //! returns coordinates of the point
    QgsPoint pixelCoords() { return mPixelCoords; }
    QgsPoint mapCoords() { return mMapCoords; }
    
  private:
    int mId;
    QgsPoint mPixelCoords;
    QgsPoint mMapCoords;
    QSizeF mTextBounds;
};

