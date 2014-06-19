/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "medSegmentationAbstractToolBox.h"

#include "msegPluginExport.h"

#include "medProcessPaintSegm.h"

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medDataIndex.h>
#include <medViewEventFilter.h>
#include <medImageMaskAnnotationData.h>
#include <itkApproximateSignedDistanceMapImageFilter.h>
#include <itkReinitializeLevelSetImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkExtractImageFilter.h>

#include <QVector3D>
#include <QTextEdit>

#include <vector>

class medAbstractData;
class medAbstractView;
class medAnnotationData;

class dtkAbstractProcessFactory;
class medSeedPointAnnotationData;

namespace mseg {
    class ClickAndMoveEventFilter;

struct PaintState {
    enum E{ None, Wand, Stroke, DeleteStroke };
};

typedef itk::Image <unsigned char, 3>              MaskType;
typedef itk::Image <float, 3>                      MaskFloatType;
typedef itk::ImageRegionIterator <MaskType>        MaskIterator;
typedef itk::ImageRegionIterator <MaskFloatType>   MaskFloatIterator;

typedef itk::Image <unsigned char, 2>              Mask2dType;
typedef itk::ImageRegionIterator<Mask2dType>       Mask2dIterator;
typedef itk::Image <float, 2>                      Mask2dFloatType;
typedef itk::ImageRegionIterator <Mask2dFloatType> Mask2dFloatIterator;

typedef itk::BinaryThresholdImageFilter              < MaskType, MaskType >          BinaryThresholdImageFilterType;
typedef itk::ExtractImageFilter                      < MaskType, Mask2dType >        Extract2DType;
typedef itk::ApproximateSignedDistanceMapImageFilter < Mask2dType, Mask2dFloatType > DistanceMap2DType;
typedef itk::ReinitializeLevelSetImageFilter         < Mask2dType >                  LevelSetFilterType;

//! Segmentation toolbox to allow manual painting of pixels
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT AlgorithmPaintToolbox : public medSegmentationAbstractToolBox
{
    Q_OBJECT;
public:

    typedef QPair<Mask2dType::Pointer,unsigned int> SlicePair;
    typedef QPair<QList<SlicePair>,unsigned char> PairListSlicePlaneId;

    AlgorithmPaintToolbox( QWidget *parent );
    ~AlgorithmPaintToolbox();

     //! Override dtkAbstractObject
    QString description() const { return s_description(); }
    QString identifier() const { return s_identifier(); }

    static medSegmentationAbstractToolBox * createInstance( QWidget *parent );

    static QString s_description();

    /** Get name to use for this when registering with a factory.*/
    static QString s_identifier();

    //! Get a human readable name for this widget.
    /** \param trObj : Provide an object for the tr() function. If NULL qApp will be used. */
    static QString s_name(const QObject * trObj =  NULL);

    inline void setPaintState(PaintState::E value){m_paintState = value;}
    inline PaintState::E paintState(){return m_paintState;}
    void setCurrentView(medAbstractView * view);

    bool getSeedPlanted();
    void setSeedPlanted(bool,MaskType::IndexType,unsigned int,double);
    void setSeed(QVector3D);

    inline bool getCursorOn(){return cursorOn;};
    void setCursorOn(bool value);
    inline void setCurrentIdSlice(unsigned int id){currentIdSlice = id;};
    inline unsigned int getCurrentIdSlice(){return currentIdSlice;};
    inline void setCurrentPlaneIndex(unsigned int index){currentPlaneIndex = index;};
    inline unsigned int getCurrentPlaneIndex(){return currentPlaneIndex;};
    void setParameter(int channel, int value);

    bool isData(Mask2dType::Pointer input,unsigned char label);
    Mask2dType::Pointer extract2DImageSlice(MaskType::Pointer input,int plane,int slice,MaskType::SizeType size,MaskType::IndexType start);
    Mask2dFloatType::Pointer computeDistanceMap(Mask2dType::Pointer img);
    void computeIntermediateSlice(Mask2dFloatType::Pointer distanceMapImg0,Mask2dFloatType::Pointer distanceMapImg1,int nbinterslice,
                                                              int slice0,
                                                              int slice1,int j,MaskFloatIterator ito,MaskIterator itMask);
                                                

public slots:
    void onStrokeToggled(bool);
    void onMagicWandToggled(bool);
  /*  void onStrokePressed();
    void onMagicWandPressed();
*/
    void onApplyButtonClicked();
    void onClearMaskClicked();

    void onLabelChanged(int newVal);
    void onSelectLabelColor();

    void synchronizeWandSpinBoxesAndSliders(void);
    
    void updateStroke(ClickAndMoveEventFilter * filter, medAbstractView * view);
    void updateWandRegion(medAbstractView * view, QVector3D &vec);
    void updateMouseInteraction();

    void onUndo();
    void onRedo();
    void addSliceToStack(medAbstractView * view,const unsigned char planeIndex,QList<int> listIdSlice);
    //void saveCurrentStateForCursor(medAbstractView * view,const unsigned char planeIndex,unsigned int idSlice);
    void onViewClosed();

    void onNewSeed();
    void onRemoveSeed();

    void copySliceMask();
    void pasteSliceMask();

    void onAddBrushSize();
    void onReduceBrushSize();

    void onInterpolate();

protected:
    friend class ClickAndMoveEventFilter;
    
    void addStroke( medAbstractView *view, const QVector3D &vec );
    void setData( dtkAbstractData *data );

    // update with seed point data.
    void updateTableRow(int row);

    void initializeMaskData( medAbstractData * imageData, medAbstractData * maskData );

    void updateFromGuiItems();

    void showButtons( bool value);

    void generateLabelColorMap(unsigned int numLabels);

    void updateButtons();
    void addBrushSize(int size);

    char computePlaneIndex(const QVector3D &,MaskType::IndexType & ,bool& isInside);

    void copySliceFromMask3D(itk::Image<unsigned char,2>::Pointer copy,const char planeIndex,const char * direction,const unsigned int slice);
    void pasteSliceToMask3D(itk::Image<unsigned char,2>::Pointer image2D,const char planeIndex,const char * direction,const unsigned int slice);

    void removeCursorDisplay();

private:
    typedef dtkSmartPointer<medSeedPointAnnotationData> SeedPoint;

    QPushButton *m_strokeButton;
    QPushButton *m_labelColorWidget;
    QSpinBox *m_strokeLabelSpinBox;
    QPushButton * m_newSeedButton;
    QPushButton * m_removeSeedButton;
    QShortcut *undo_shortcut, *redo_shortcut, *copy_shortcut, *paste_shortcut, *newSeed_shortcut, *removeSeed_shortcut, *addBrushSize_shortcut, *reduceBrushSize_shortcut;
    
    QLabel *m_colorLabel;
    QLabel * m_wandInfo;

    QSlider *m_brushSizeSlider;
    QSpinBox *m_brushSizeSpinBox;
    QLabel *m_brushRadiusLabel;

    QFormLayout * magicWandLayout;
    QPushButton *m_magicWandButton;
    QSlider *m_wandUpperThresholdSlider, *m_wandLowerThresholdSlider;
    QDoubleSpinBox *m_wandUpperThresholdSpinBox , * m_wandLowerThresholdSpinBox;
    QCheckBox *m_wand3DCheckbox;
    QTime wandTimer;

    bool seedPlanted;
    QVector3D m_seed;

    double m_MinValueImage;
    double m_MaxValueImage;

    QPushButton *m_applyButton;
    QPushButton *m_interpolateButton;

    QPushButton *m_clearMaskButton;

    dtkSmartPointer< ClickAndMoveEventFilter > m_viewFilter;

    dtkSmartPointer<medImageMaskAnnotationData> m_maskAnnotationData;

    dtkSmartPointer<medAbstractData> m_maskData;
    dtkSmartPointer<medAbstractData> m_imageData;
    
    medImageMaskAnnotationData::ColorMapType m_labelColorMap;
    
    MaskType::Pointer m_itkMask;
    QPair<Mask2dType::Pointer,char> m_copy;
    
    // undo_redo_feature's attributes
    QHash<medAbstractView*,QStack<PairListSlicePlaneId>*> * m_undoStacks,*m_redoStacks;
    medAbstractView * currentView;
    medAbstractView * viewCopied;

    template <typename IMAGE> void RunConnectedFilter (MaskType::IndexType &index, unsigned int planeIndex);
    template <typename IMAGE> void GenerateMinMaxValuesFromImage ();

    QVector3D m_lastVup;
    QVector3D m_lastVpn;
    double m_sampleSpacing[2];

    double m_wandRadius, m_wandUpperThreshold, m_wandLowerThreshold;
    double m_strokeRadius;
    unsigned int m_strokeLabel;

    PaintState::E m_paintState;
    bool cursorOn;
    QList<QPair<MaskType::IndexType,unsigned char> > * cursorPixels;
    unsigned int currentPlaneIndex; //plane Index of the current/last operation
    unsigned int currentIdSlice; // current slice;
    bool undoRedoCopyPasteModeOn;
    bool cursorJustReactivated;
};

} // namespace mseg


