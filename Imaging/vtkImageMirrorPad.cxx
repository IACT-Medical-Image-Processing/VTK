/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageMirrorPad.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageMirrorPad.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkImageMirrorPad, "1.31.10.2");
vtkStandardNewMacro(vtkImageMirrorPad);

//----------------------------------------------------------------------------
// Just clip the request.
void vtkImageMirrorPad::ComputeInputUpdateExtent(int inExt[6], 
                                                 int outExt[6])
{
  int *wExtent = this->GetInput()->GetWholeExtent();
  int idx;
  
  // initialize inExt
  memcpy(inExt,wExtent,6*sizeof(int));

  // a simple approximation to the required extent
  // basically get the whole extent for an axis unless a fully
  // contained subset is being requested. If so then use that.
  for (idx = 0; idx < 3; idx++)
    {
    if (outExt[idx*2] >= wExtent[idx*2] &&
        outExt[idx*2+1] <= wExtent[idx*2+1])
      {
      inExt[idx*2] = outExt[idx*2];
      inExt[idx*2+1] = outExt[idx*2+1];
      }
    }
}


//----------------------------------------------------------------------------
template <class T>
void vtkImageMirrorPadExecute(vtkImageMirrorPad *self,
                              vtkImageData *inData,
                              vtkImageData *outData, T *outPtr,
                              int outExt[6], int id)
{
  int idxC, idxX, idxY, idxZ;
  int maxX, maxY, maxZ;
  int inInc[3];
  int inIncStart[3];
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  unsigned long count = 0;
  unsigned long target;
  int *wExtent = self->GetInput()->GetWholeExtent();
  int idx;
  int inIdxStart[3];
  int inIdx[3];
  T *inPtr, *inPtrX, *inPtrY, *inPtrZ;
  int maxC, inMaxC;
  
  // find the region to loop over
  inMaxC = inData->GetNumberOfScalarComponents();
  maxC = outData->GetNumberOfScalarComponents();
  maxX = outExt[1] - outExt[0]; 
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;
  
  // Get increments to march through data 
  inData->GetIncrements(inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // find the starting point
  for (idx = 0; idx < 3; idx++)
    {
    inIdxStart[idx] = outExt[idx*2];
    inIncStart[idx] = 1;
    while (inIdxStart[idx] < wExtent[idx*2])
      {
      inIncStart[idx] = -inIncStart[idx];
      inIdxStart[idx] = inIdxStart[idx] + (wExtent[idx*2+1] - wExtent[idx*2] + 1);
      }
    while (inIdxStart[idx] > wExtent[idx*2+1])
      {
      inIncStart[idx] = -inIncStart[idx];
      inIdxStart[idx] = inIdxStart[idx] - (wExtent[idx*2+1] - wExtent[idx*2] + 1);
      }
    // if we are heading negative then we need to mirror the offset
    if (inIncStart[idx] < 0)
      {
      inIdxStart[idx] = wExtent[idx*2+1] - inIdxStart[idx] + wExtent[idx*2];
      }
    }
  inPtr = (T *)inData->GetScalarPointer(inIdxStart[0], inIdxStart[1], inIdxStart[2]);
  
  // Loop through ouput pixels
  inPtrZ = inPtr;
  inIdx[2] = inIdxStart[2];
  inInc[2] = inIncStart[2];
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    inPtrY = inPtrZ;
    inIdx[1] = inIdxStart[1];
    inInc[1] = inIncStart[1];
    for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++)
      {
      inPtrX = inPtrY;
      inIdx[0] = inIdxStart[0];
      inInc[0] = inIncStart[0];
      if (!id) 
        {
        if (!(count%target))
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }

      // if components are same much faster
      if ((maxC == inMaxC) && (maxC == 1))
        {
        for (idxX = 0; idxX <= maxX; idxX++)
          {
          // Pixel operation
          *outPtr = *inPtrX;
          outPtr++;
          inIdx[0] += inInc[0];
          inPtrX = inPtrX + inInc[0]*inIncX;
          if (inIdx[0] < wExtent[0] || inIdx[0] > wExtent[1])
            {
            inInc[0] *= -1;
            inIdx[0] += inInc[0];
            inPtrX = inPtrX + inInc[0]*inIncX;
            }
          }
        }
      else // components are not the same
        {
        for (idxX = 0; idxX <= maxX; idxX++)
          {
          for (idxC = 0; idxC < maxC; idxC++)
            {
            // Pixel operation
            if (idxC < inMaxC)
              {
              *outPtr = *(inPtrX + idxC);
              }
            else
              {
              *outPtr = *(inPtrX + idxC%inMaxC);
              }
            outPtr++;
            }
          inIdx[0] += inInc[0];
          inPtrX = inPtrX + inInc[0]*inIncX;
          if (inIdx[0] < wExtent[0] || inIdx[0] > wExtent[1])
            {
            inInc[0] *= -1;
            inIdx[0] += inInc[0];
            inPtrX = inPtrX + inInc[0]*inIncX;
            }
          }
        }
      
      outPtr += outIncY;
      inIdx[1] += inInc[1];
      inPtrY = inPtrY + inInc[1]*inIncY;
      if (inIdx[1] < wExtent[2] || inIdx[1] > wExtent[3])
        {
        inInc[1] *= -1;
        inIdx[1] += inInc[1];
        inPtrY = inPtrY + inInc[1]*inIncY;
        }
      }
    outPtr += outIncZ;
    inIdx[2] += inInc[2];
    inPtrZ = inPtrZ + inInc[2]*inIncZ;
    if (inIdx[2] < wExtent[4] || inIdx[2] > wExtent[5])
      {
      inInc[2] *= -1;
      inIdx[2] += inInc[2];
      inPtrZ = inPtrZ + inInc[2]*inIncZ;
      }
    }
}



//----------------------------------------------------------------------------
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageMirrorPad::ThreadedExecute(vtkImageData *inData, 
                                        vtkImageData *outData,
                                        int outExt[6], int id)
{
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
                << ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
                  << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  switch (inData->GetScalarType())
    {
    vtkTemplateMacro6(vtkImageMirrorPadExecute, this, inData, outData, 
                      (VTK_TT *)(outPtr), outExt, id);
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}














