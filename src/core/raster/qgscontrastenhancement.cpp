/* **************************************************************************
                qgscontrastenhancement.cpp -  description
                       -------------------
begin                : Mon Oct 22 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class contains code that was originally part of the larger QgsRasterLayer 
class originally created circa 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"

#include "qgscontrastenhancement.h"
#include "qgscontrastenhancementfunction.h"
#include "qgslinearminmaxenhancement.h"
#include "qgslinearminmaxenhancementwithclip.h"
#include "qgscliptominmaxenhancement.h"

QgsContrastEnhancement::QgsContrastEnhancement(QgsRasterDataType theDataType)
{
  #ifdef QGISDEBUG
    QgsDebugMsg("QgsContrastEnhancement::QgsContrastEnhancement() called");
  #endif
  mLookupTable = 0; 
  mContrastEnhancementFunction = 0;
  mEnhancementDirty = false;
  mContrastEnhancementAlgorithm = NO_STRETCH;
  mRasterDataType = theDataType;
  
  mMinimumValue = getMinimumPossibleValue(mRasterDataType);
  mMaximumValue = getMaximumPossibleValue(mRasterDataType);
  mRasterDataTypeRange = mMaximumValue - mMinimumValue;
  
  mLookupTableOffset = mMinimumValue * -1;
  
  mContrastEnhancementFunction = new QgsContrastEnhancementFunction(mRasterDataType, mMinimumValue, mMaximumValue);
  
  //If the data type is larger than 16-bit do not generate a lookup table
  if(mRasterDataTypeRange <= 65535.0)
  {
     mLookupTable = new int[static_cast <int>(mRasterDataTypeRange+1)];
  }
  
}
 
QgsContrastEnhancement::~QgsContrastEnhancement()
{
}
/*
 *
 * Static methods
 *
 */

/** 
    Simple function to compute the maximum possible value for a data types. 
*/
double QgsContrastEnhancement::getMaximumPossibleValue(QgsRasterDataType theDataType) 
{
  switch(theDataType)
  {
    case QGS_Byte:
      return std::numeric_limits<unsigned char>::max();
      break;
    case QGS_UInt16:
      return std::numeric_limits<unsigned short>::max();
      break;
    case QGS_Int16:
      return std::numeric_limits<short>::max();
      break;
    case QGS_UInt32:
      return std::numeric_limits<unsigned int>::max();
      break;
    case QGS_Int32:
      return std::numeric_limits<int>::max();
      break;
    case QGS_Float32:
      return std::numeric_limits<float>::max();
      break;
    case QGS_Float64:
    return std::numeric_limits<double>::max();
      break;
    case QGS_CInt16:
    return std::numeric_limits<short>::max();
      break;
    case QGS_CInt32:
    return std::numeric_limits<int>::max();
      break;
    case QGS_CFloat32:
    return std::numeric_limits<float>::max();
      break;
    case QGS_CFloat64:
    return std::numeric_limits<double>::max();
      break;
  }

  return std::numeric_limits<double>::max();
}
/** 
    Simple function to compute the minimum possible value for a data type. 
*/
double QgsContrastEnhancement::getMinimumPossibleValue(QgsRasterDataType theDataType) 
{
  switch(theDataType)
  {
    case QGS_Byte:
      return std::numeric_limits<unsigned char>::min();
      break;
    case QGS_UInt16:
      return std::numeric_limits<unsigned short>::min();
      break;
    case QGS_Int16:
      return std::numeric_limits<short>::min();
      break;
    case QGS_UInt32:
      return std::numeric_limits<unsigned int>::min();
      break;
    case QGS_Int32:
      return std::numeric_limits<int>::min();
      break;
    case QGS_Float32:
      return std::numeric_limits<float>::max() * -1.0;
      break;
    case QGS_Float64:
    return std::numeric_limits<double>::max() * -1.0;
      break;
    case QGS_CInt16:
    return std::numeric_limits<short>::min();
      break;
    case QGS_CInt32:
    return std::numeric_limits<int>::min();
      break;
    case QGS_CFloat32:
    return std::numeric_limits<float>::max() * -1.0;
      break;
    case QGS_CFloat64:
    return std::numeric_limits<double>::max() * -1.0;
      break;
  }

  return std::numeric_limits<double>::max() * -1.0;
}

/*
 *
 * Non-Static methods
 *
 */
 

/**
    Generate a new lookup table
*/
bool QgsContrastEnhancement::generateLookupTable()
{
  mEnhancementDirty = false;

  if(0 == mContrastEnhancementFunction) { return false; }
  if(NO_STRETCH == mContrastEnhancementAlgorithm) { return false; }
  if(QGS_Byte != mRasterDataType && QGS_UInt16 != mRasterDataType && QGS_Int16 != mRasterDataType) { return false; }
  if(!mLookupTable) { return false; }

  #ifdef QGISDEBUG
    QgsDebugMsg("QgsContrastEnhancement::generateLookupTable() - building lookup table");
    QgsDebugMsg("***MinimumValue : "+ QString::number(mMinimumValue));
    QgsDebugMsg("***MaximumValue : "+ QString::number(mMaximumValue));
    QgsDebugMsg("***mLookupTableOffset : "+ QString::number(mLookupTableOffset));
    QgsDebugMsg("***mRasterDataTypeRange : "+ QString::number(mRasterDataTypeRange));
  #endif
  for(int myIterator = 0; myIterator <= mRasterDataTypeRange; myIterator++)
  {
    mLookupTable[myIterator] = mContrastEnhancementFunction->enhanceValue((double)myIterator - mLookupTableOffset);
  }

  return true;
}

/**
    Determine if a pixel is within in the displayable range.
    
    @param theValue The pixel value to examine
*/
bool QgsContrastEnhancement::isValueInDisplayableRange(double theValue)
{

  if(0 != mContrastEnhancementFunction)
  {
    return mContrastEnhancementFunction->isValueInDisplayableRange(theValue);
  }

  return false;
}

/**
    Set the contrast enhancement algorithm. The second parameter is optional and is for performace improvements. If you know you are immediately going to set the Minimum or Maximum value, you can elect to not generate the lookup tale. By default it will be generated.
    
    @param theAlgorithm The new contrast enhancement algorithm
    @param generateTable Flag to overide automatic look up table generation
*/
void QgsContrastEnhancement::setContrastEnhancementAlgorithm(CONTRAST_ENHANCEMENT_ALGORITHM theAlgorithm, bool generateTable)
{
  #ifdef QGISDEBUG
    QgsDebugMsg("QgsContrastEnhancement::setContrastEnhancementAlgorithm() called algorithm: "+ QString::number((int)theAlgorithm) +" generate lookup table: "+ QString::number((int)generateTable));
  #endif
  if(theAlgorithm != mContrastEnhancementAlgorithm )
  {
    switch(theAlgorithm)
    {
      case STRETCH_TO_MINMAX :
        mContrastEnhancementFunction = new QgsLinearMinMaxEnhancement(mRasterDataType, mMinimumValue, mMaximumValue);
        break;
      case STRETCH_AND_CLIP_TO_MINMAX :
        mContrastEnhancementFunction = new QgsLinearMinMaxEnhancementWithClip(mRasterDataType, mMinimumValue, mMaximumValue);
        break;
      case CLIP_TO_MINMAX :
        mContrastEnhancementFunction = new QgsClipToMinMaxEnhancement(mRasterDataType, mMinimumValue, mMaximumValue);
        break;
      case USER_DEFINED :
        //Do nothing
        break;
      default:
        mContrastEnhancementFunction = new QgsContrastEnhancementFunction(mRasterDataType, mMinimumValue, mMaximumValue);
        break;
    }
    
    mEnhancementDirty = true;
    mContrastEnhancementAlgorithm = theAlgorithm;
    
    if(generateTable)
    {
      generateLookupTable();
    }
  }
}

/**
    A public function that allows the user to set their own custom contrast enhancement function. 
    
    @param theFunction The new contrast enhancement function
*/
void QgsContrastEnhancement::setContrastEnhancementFunction(QgsContrastEnhancementFunction* theFunction)
{
  #ifdef QGISDEBUG
    QgsDebugMsg("QgsContrastEnhancement::setContrastEnhancementFunction() called");
  #endif
  if(0 != theFunction)
  {
    mContrastEnhancementFunction = theFunction;
    mContrastEnhancementAlgorithm = USER_DEFINED;
    generateLookupTable();
  }
}

/**
    Set the maximum value for the contrast enhancement. The second parameter is option an is for performace improvements. If you know you are immediately going to set the Minimum value or the contrast enhancement algorithm, you can elect to not generate the lookup tale. By default it will be generated.
    
    @param theValue The new maximum value for the band
    @param generateTable Flag to overide automatic look up table generation
*/
void QgsContrastEnhancement::setMaximumValue(double theValue, bool generateTable)
{
  #ifdef QGISDEBUG
    QgsDebugMsg("QgsContrastEnhancement::setMaximumValue() called value: "+ QString::number(theValue) +" generate lookup table: "+ QString::number((int)generateTable));
  #endif
  if(theValue > getMaximumPossibleValue(mRasterDataType))
  {
    mMaximumValue = getMaximumPossibleValue(mRasterDataType);
  }
  else
  {
    mMaximumValue = theValue; 
  }

  if(0 != mContrastEnhancementFunction)
  {
    mContrastEnhancementFunction->setMaximumValue(theValue);
  }

  mEnhancementDirty = true;

  if(generateTable)
  {
    generateLookupTable();
  }
}

/**
    Set the maximum value for the contrast enhancement. The second parameter is option an is for performace improvements. If you know you are immediately going to set the Maximum value or the contrast enhancement algorithm, you can elect to not generate the lookup tale. By default it will be generated.
    
    @param theValue The new minimum value for the band
    @param generateTable Flag to overide automatic look up table generation
*/
void QgsContrastEnhancement::setMinimumValue(double theValue, bool generateTable)
{
  #ifdef QGISDEBUG
    QgsDebugMsg("QgsContrastEnhancement::setMinimumValue() called value: "+ QString::number(theValue) +" generate lookup table: "+ QString::number((int)generateTable));
  #endif
  if(theValue < getMinimumPossibleValue(mRasterDataType))
  {
    mMinimumValue = getMinimumPossibleValue(mRasterDataType);
  }
  else
  {
    mMinimumValue = theValue; 
  }

  if(0 != mContrastEnhancementFunction)
  {
    mContrastEnhancementFunction->setMinimumValue(theValue);
  }

  mEnhancementDirty = true;

  if(generateTable)
  {
    generateLookupTable();
  }
}

/**
    Public function to generate the enhanced for stretched value for a given input.
    
    @param theValue The pixel value to enhance
*/
int QgsContrastEnhancement::stretch(double theValue)
{
  if(mEnhancementDirty)
  {
    generateLookupTable();
  }

  if(mLookupTable && NO_STRETCH != mContrastEnhancementAlgorithm)
  {
    return mLookupTable[static_cast <int>(theValue + mLookupTableOffset)];
  }
  else
  {
    // Even if the contrast enhancement algorithms is set to NO_STRETCH
    // The input values will still have to be scaled for all data types
    // greater than 1 byte.
    return mContrastEnhancementFunction->enhanceValue(theValue);
  }
}
