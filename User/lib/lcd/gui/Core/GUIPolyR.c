/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              �C/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUIPolyR.c
Purpose     : Polygon rotation
----------------------------------------------------------------------
*/

#include "arm_math.h"
#include "GUI.h"

/*********************************************************************
 *
 *       Public code
 *
 **********************************************************************
 */
/*********************************************************************
 *
 *       GUI_RotatePolygon
 */
void GUI_RotatePolygon(GUI_POINT *pDest, const GUI_POINT *pSrc, int NumPoints, float Angle)
{
  int j;
  float fcos = arm_cos_f32(Angle);
  float fsin = arm_sin_f32(Angle);
  for (j = 0; j < NumPoints; j++)
  {
    int x = (pSrc + j)->x;
    int y = (pSrc + j)->y;
    (pDest + j)->x = x * fcos + y * fsin;
    (pDest + j)->y = -x * fsin + y * fcos;
  }
}

/*************************** End of file ****************************/
