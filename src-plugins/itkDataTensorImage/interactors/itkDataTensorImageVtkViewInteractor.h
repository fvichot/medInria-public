/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medAbstractImageViewInteractor.h>

#include <itkDataTensorImagePluginExport.h>

class itkDataTensorImageVtkViewInteractorPrivate;

class medAbstractData;
class medAbstractImageView;

/**
 * @class itkDataTensorImageVtkViewInteractor
 * @brief Extents a view by providing tensor viewing/managing capabilities.
 *
 * An interactor is an extension to a view (v3dView in this case)
 * that provides additional functionality. This class extents the view by
 * adding tensor handling capabilities like visualization and tensor-specific
 * properties.
 */
class ITKDATATENSORIMAGEPLUGIN_EXPORT itkDataTensorImageVtkViewInteractor: public medAbstractImageViewInteractor
{

    Q_OBJECT

public:
    itkDataTensorImageVtkViewInteractor(medAbstractView* parent);
    virtual ~itkDataTensorImageVtkViewInteractor();

    enum GlyphShapeType
    {
        Lines,
        Disks,
        Arrows,
        Cubes,
        Cylinders,
        Ellipsoids,
        Superquadrics
    };

public slots:

    virtual QString description() const;
    virtual QString identifier() const;
    virtual QStringList handled() const;

    static bool registered();

    virtual void setData(medAbstractData * data);

    virtual QWidget* buildLayerWidget();
    virtual QWidget* buildToolBoxWidget();
    virtual QWidget* buildToolBarWidget();
    virtual QList<medAbstractParameter*> linkableParameters();
    virtual QList<medBoolParameter*> mouseInteractionParameters();

    void removeData();

    virtual void updateWidgets();

public slots:
    void setOpacity(double opacity);
    void setVisibility(bool visibility);
    void setWindowLevel(QList<QVariant>);

    /** Change glyph shape */
    void setGlyphShape(QString glyphShape);

    /** Modify sample rate */
    void setSampleRate(int sampleRate);

    /** Flip tensors along the X axis */
    void setFlipX(bool flip);

    /** Flip tensors along the Y axis */
    void setFlipY(bool flip);

    /** Flip tensors along the Z axis */
    void setFlipZ(bool flip);

    /** A new eigenvector for mapping the color mode is set */
    void setEigenVector(QString color);

    /** A new eigenvector for mapping the color mode is set */
    void setEigenVector(int eigenVector);

    /** Glyph resolution changed */
    void setGlyphResolution(int glyphResolution);

    /** Scaling changed */
    void setScale(double scale);

    /** Hide or show axial slice */
    void setShowAxial(bool show);

    /** Hide or show coronal slice */
    void setShowCoronal(bool show);

    /** Hide or show sagittal slice */
    void setShowSagittal(bool show);

    /** Change position of the slices */
    void changePosition(const QVector3D& position);


    void setMajorScaling(int majorScalingExponent);

    void setMinorScaling(int minorScaling);

    void setScale(int minorScale, int majorScaleExponent);

    virtual void setUpViewForThumbnail();

    void moveToSlice(int slice);

protected:
    void update();

private:
    static QStringList dataHandled();

private slots:
    void updateSlicingParam();

private:
    itkDataTensorImageVtkViewInteractorPrivate *d;

};

