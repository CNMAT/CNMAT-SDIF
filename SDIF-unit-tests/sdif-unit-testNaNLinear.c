#ifndef SDIF_BOOLEANtypedef unsigned char Boolean;#undef TRUE#define TRUE 1#undef FALSE#define FALSE 0#define SDIF_BOOLEAN#endif#include <stdio.h>#include <stdlib.h>#include <stdarg.h>#include <math.h>#include "sdif.h"#include "sdif-mem.h"#include "sdif-buf.h"#include "sdif-interp.h"#include "sdif-util.h"#include "sdif-unit.h"#include "sdif-unit-utils.h"//  parameters for running this teststatic const int NUM_FRAMES = 20;static const int NUM_COLUMNS = 2;static const int NUM_ROWS = 6;static const int FUN_DOMAIN = 100;static const int NUM_RANDOM_TIMES = 40;//  prototypes for entrypointsUnitTestFn unit_randomNaNLinear;UnitTestFn unit_randomNaNLagrange;UnitTestFn unit_randomNaNNeighbors;//  prototypes for local functionsstatic SDIFresult doTest(SDIFinterp_InterpolatorFn interpFn);static SDIFresult doOneTest(SDIFinterp_InterpolatorFn interpFn);static SDIFresult doLagrangeTest(void);static SDIFresult doNeighborTest(void);static SDIFresult doOneNeighborTest(void);static SDIFresult doOneLagrangeTest(void);static SDIFresult doInterpolatorTestGuts(SDIFbuf_Buffer buf, SDIFinterp_Interpolator it);static SDIFresult doLagrangeTestGuts(SDIFbuf_Buffer buf);static SDIFresult doNeighborTestGuts(SDIFbuf_Buffer buf, sdif_int32 count);static SDIFbuf_Buffer createRandomFunctionBuffer();static int createRandomFunction(SDIFbuf_Buffer b,                                sdif_int32 numFrames,                                const char *frameType,                                const char *matrixType,                                sdif_int32 columns,                                sdif_int32 rows,                                sdif_float64 functionDomain                                );static SDIFresult addJunkToBuffer(SDIFbuf_Buffer buf, int junkCount);static void randomTimeInit(void);static sdif_float64 randomTime(void);static Boolean matrixAlmostEqual(SDIFmem_Matrix m1, SDIFmem_Matrix m2);static sdif_float64 interpolateOneCell(SDIFbuf_Buffer buf,                                const char *matrixType,                                sdif_int32 column,                                sdif_int32 row,                                sdif_float64 time                                );static SDIFmem_Matrix doInterpolate(SDIFinterp_Interpolator it,                                     SDIFbuf_Buffer b,                                    const char *matrixType,                                    sdif_float64 time                                    );static SDIFinterp_Interpolator createInterpolator(SDIFinterp_InterpolatorFn interpFn);static SDIFinterp_InterpolatorFn unit_LinearInterpolator;static SDIFinterp_InterpolatorFn unit_LagrangeInterpolator;//  globalsstatic sdif_float64 randomTimeList[NUM_RANDOM_TIMES];//  implementation of unit test functionsvoid *unit_randomNaNLinear(SDIFunitCmd cmd, ...){  void *result = NULL;    va_list va;  va_start(va, cmd);  switch(cmd)  {    case ESDIF_UNIT_CMD_NAME:      result = "random NaN function linear interpolation test";      break;    case ESDIF_UNIT_CMD_RUN:      result = (void *)doTest(unit_LinearInterpolator);      break;  }    va_end(va);    return result;}void *unit_randomNaNLagrange(SDIFunitCmd cmd, ...){  void *result = NULL;    va_list va;  va_start(va, cmd);  switch(cmd)  {    case ESDIF_UNIT_CMD_NAME:      result = "random NaN function Lagrange interpolation test";      break;    case ESDIF_UNIT_CMD_RUN:      result = (void *)doLagrangeTest();      break;  }    va_end(va);    return result;}void *unit_randomNaNNeighbors(SDIFunitCmd cmd, ...){  void *result = NULL;    va_list va;  va_start(va, cmd);  switch(cmd)  {    case ESDIF_UNIT_CMD_NAME:      result = "random NaN function neighbor value test";      break;    case ESDIF_UNIT_CMD_RUN:      result = (void *)doNeighborTest();      break;  }    va_end(va);    return result;}static SDIFresult doTest(SDIFinterp_InterpolatorFn interpFn){  SDIFresult r;  int i;    //  run the test repeatedly  //  (random parameters should exercise a variety of unusual cases)  for(i = 0; i < 100; i++)  {    unit_log(LOG_RESULTS, "starting run %d\n", i);    if(ESDIF_SUCCESS != (r = doOneTest(interpFn)))    {      unit_log(LOG_ERROR, "failed on run %d\n", i);      break;    }  }    return r;}static SDIFresult doOneTest(SDIFinterp_InterpolatorFn interpFn){  SDIFresult r;  SDIFbuf_Buffer buf;  SDIFinterp_Interpolator it;  //  initialize collection of random times  //  (use limited set of times so we exercise a variety of exact time match/collision cases)  randomTimeInit();    //  create a buffer  //  add a bunch of random values at random times  if(!(buf = createRandomFunctionBuffer()))    return ESDIF_OUT_OF_MEMORY;    //  add a bunch of empty frames at other random times  if(ESDIF_SUCCESS != (r = addJunkToBuffer(buf, NUM_FRAMES)))    return r;    //  create an interpolator  if(!(it = createInterpolator(interpFn)))  {    SDIFbuf_Free(buf);    return ESDIF_OUT_OF_MEMORY;  }  //  interpolate the random function buffer 2 different ways, compare results  r = doInterpolatorTestGuts(buf, it);  //  release the buffer + interpolator  SDIFbuf_Free(buf);  SDIFinterp_Free(it);    return r;}static SDIFresult doLagrangeTest(){  SDIFresult r;  int i;    //  run the test repeatedly  //  (random parameters should exercise a variety of unusual cases)  for(i = 0; i < 100; i++)  {    unit_log(LOG_RESULTS, "starting run %d\n", i);    if(ESDIF_SUCCESS != (r = doOneLagrangeTest()))    {      unit_log(LOG_ERROR, "failed on run %d\n", i);      break;    }  }    return r;}static SDIFresult doOneLagrangeTest(){  SDIFresult r;  SDIFbuf_Buffer buf;  //  initialize collection of random times  //  (use limited set of times so we exercise a variety of exact time match/collision cases)  randomTimeInit();    //  create a buffer  //  add a bunch of random values at random times  if(!(buf = createRandomFunctionBuffer()))    return ESDIF_OUT_OF_MEMORY;    //  add a bunch of empty frames at other random times  if(ESDIF_SUCCESS != (r = addJunkToBuffer(buf, NUM_FRAMES)))    return r;    //  interpolate the random function buffer 2 different ways, compare results  r = doLagrangeTestGuts(buf);  //  release the buffer + interpolator  SDIFbuf_Free(buf);    return r;}static SDIFresult doNeighborTest(){  SDIFresult r;  int i;    //  run the test repeatedly  //  (random parameters should exercise a variety of unusual cases)  for(i = 0; i < 100; i++)  {    unit_log(LOG_RESULTS, "starting run %d\n", i);    if(ESDIF_SUCCESS != (r = doOneNeighborTest()))    {      unit_log(LOG_ERROR, "failed on run %d\n", i);      break;    }  }    return r;}static SDIFresult doOneNeighborTest(){  SDIFresult r;  SDIFbuf_Buffer buf;  //  initialize collection of random times  //  (use limited set of times so we exercise a variety of exact time match/collision cases)  randomTimeInit();    //  create a buffer  //  add a bunch of random values at random times  if(!(buf = createRandomFunctionBuffer()))    return ESDIF_OUT_OF_MEMORY;    //  add a bunch of empty frames at other random times  if(ESDIF_SUCCESS != (r = addJunkToBuffer(buf, NUM_FRAMES)))    return r;    //  test neighbor search (test odd + even number)  r = doNeighborTestGuts(buf, 5);  r |= doNeighborTestGuts(buf, 6);  //  release the buffer + interpolator  SDIFbuf_Free(buf);    return r;}static SDIFresult doInterpolatorTestGuts(SDIFbuf_Buffer buf, SDIFinterp_Interpolator it){  int i, j;  sdif_float64 t, prevTime;    //  compute interpolated samples at regular intervals  //  1. do it the long way  //  2. do it through SDIFinterp_Interpolator instance  //  3. compare results  for(t = 0; t <= FUN_DOMAIN; t += 10)  {    SDIFmem_Frame f;    SDIFmem_Matrix m, mCheck, mInterp;    sdif_float64 t1, t2, v1, v2, vt, tt;        //  do linear interpolation per cell without using an SDIFinter_Interpolator instance    if(!(mCheck = SDIFutil_CreateMatrix(NUM_COLUMNS, NUM_ROWS, SDIF_FLOAT64, "----")))      return ESDIF_OUT_OF_MEMORY;        unit_log(LOG_INFO2, "***time=%f\n\n", t);        for(i = 0; i < NUM_ROWS; i++)      for(j = 0; j < NUM_COLUMNS; j++)      {        //  loop over entire buffer to find left + right values for this cell        t1 = t2 = vt = NAN;        for(f = SDIFbuf_GetFirstFrame(buf); f; f = SDIFbuf_GetNextFrame(f))        {          m = SDIFbuf_GetMatrixInFrame(f, "----");          if(m)          {            tt = f->header.time;            vt = SDIFutil_GetMatrixCell(m, j, i);            if(!isnan(vt))            {              if(tt <= t)              {                t1 = tt;                v1 = vt;              }              else              {                t2 = tt;                v2 = vt;                break;              }            }          }        }                if(t == t1)          //  exact match          vt = v1;                else if (!isnan(t1) && !isnan(t2))          //  we have left + right values, so interpolate          vt = v1 + ((t - t1) * (v2 - v1) / (t2 - t1));                else          //  failed to find neighboring values          vt = NAN;                //  put the value in results matrix (NaN if not found)        SDIFutil_SetMatrixCell(mCheck, j, i, vt);      }        //  now do the same thing using SDIFinterp_Interpolator    mInterp = doInterpolate(it, buf, "----", t);        //  compare results    unit_assert(mInterp != NULL, "@failed existence check\n");    if(mInterp)    {      unit_assert(SDIFutil_MatrixEqual(mInterp, mCheck), "@failed matrix match check\n");      unit_PrintMatrix(LOG_INFO2, mCheck);      unit_PrintMatrix(LOG_INFO2, mInterp);    }        //  clean up    SDIFmem_FreeMatrix(mCheck);    if(mInterp)      SDIFmem_FreeMatrix(mInterp);  }    return ESDIF_SUCCESS;}static SDIFresult doLagrangeTestGuts(SDIFbuf_Buffer buf){  int i, j;  sdif_float64 t, prevTime;  SDIFinterp_Interpolator it1, it2;    it1 = createInterpolator(unit_LinearInterpolator);  it2 = createInterpolator(unit_LagrangeInterpolator);    //  compute interpolated samples at regular intervals  //  1. do linear interpolation  //  2. do Lagrange interpolation (degree 1, should be same as linear)  //  3. compare results (not equal due to float64 computation error, but should be very close)  for(t = 0; t <= FUN_DOMAIN; t += 10)  {    SDIFmem_Frame f;    SDIFmem_Matrix m, mCheck, mInterp;    sdif_float64 t1, t2, v1, v2, vt, tt;        mCheck = doInterpolate(it1, buf, "----", t);    mInterp = doInterpolate(it2, buf, "----", t);        //  compare results    unit_assert(mCheck != NULL, "@failed existence check 1\n");    unit_assert(mInterp != NULL, "@failed existence check 2\n");    if(mCheck && mInterp)    {      unit_assert(matrixAlmostEqual(mInterp, mCheck), "@failed matrix comparison check\n");      unit_PrintMatrix(LOG_INFO2, mCheck);      unit_PrintMatrix(LOG_INFO2, mInterp);    }        //  clean up    if(mCheck)      SDIFmem_FreeMatrix(mCheck);    if(mInterp)      SDIFmem_FreeMatrix(mInterp);  }      SDIFinterp_Free(it1);  SDIFinterp_Free(it2);    return ESDIF_SUCCESS;}static SDIFresult doNeighborTestGuts(SDIFbuf_Buffer buf, sdif_int32 count){  sdif_float64 t[100], v[100];  sdif_int32 countOut;  SDIFresult r;  sdif_float64 time;  int i, j;    for(time = 0; time <= FUN_DOMAIN; time += 10)    for(i = 0; i < NUM_ROWS; i++)      for(j = 0; j < NUM_COLUMNS; j++)      {        int k;        SDIFmem_Frame f;                if(!(f = SDIFbuf_GetFrame(buf, time, ESDIF_SEARCH_BACKWARDS)))          if(!(f = SDIFbuf_GetFrame(buf, time, ESDIF_SEARCH_FORWARDS)))            break;                r = SDIFbuf_GetNeighborValues(f,                                      "----",                                      j,                                      i,                                      time,                                      count,                                      ESDIF_NAN_ACTION_KEEP_LOOKING,                                      t,                                      v,                                      &countOut                                      );        //  check number of neighbors found (we expect to find as many as were requested)        unit_assert(countOut == count, "@didn't find enough neighbors");        unit_log(LOG_INFO2, "got %d at time %f (col=%d, row=%d)\n", countOut, time, j, i);                //  check if all time values are in domain        for(k = 0; k < countOut; k++)        {          unit_assert(t[k] >= 0 && t[k] <= FUN_DOMAIN,                      "@bad time value (%d, %d, %d, %f)\n",                      j,                      i,                      k,                      t[k]);          unit_log(LOG_INFO2, "  t=%f, v=%f\n", t[k], v[k]);        }      }  return r;}static SDIFbuf_Buffer createRandomFunctionBuffer(){  SDIFbuf_Buffer buf;  SDIFmem_Frame f;  int i;  sdif_float64 prevTime;  int numUniqueFrames;    if(!(buf = SDIFbuf_Create()))    return NULL;    //  make a random function (i.e. choose some random values at random times)  numUniqueFrames = createRandomFunction(buf,                                          NUM_FRAMES,                                          "----",                                          "----",                                          NUM_COLUMNS,                                          NUM_ROWS,                                          FUN_DOMAIN                                         );  if(numUniqueFrames == 0)    //  something bad happened    return NULL;  unit_log(LOG_INFO,           "generated random matrix values at these times (count = %d):\n",           numUniqueFrames           );  //  check buffer for correct number of frames and correct (increasing) time order  for(f = SDIFbuf_GetFirstFrame(buf), i = 0, prevTime = -1; f; f = SDIFbuf_GetNextFrame(f), i++)  {    unit_log(LOG_INFO, "%f\n", f->header.time);    unit_assert(f->header.time > prevTime, "@SDIFbuf_Buffer: inserted frames out of sequence\n");    prevTime = f->header.time;  }  unit_log(LOG_INFO, "\n");  unit_assert(i == numUniqueFrames, "@SDIFbuf_Buffer: inserted frames missing\n");    return buf;}static int createRandomFunction(SDIFbuf_Buffer b,                                sdif_int32 numFrames,                                const char *frameType,                                const char *matrixType,                                sdif_int32 columns,                                sdif_int32 rows,                                sdif_float64 functionDomain                                ){  SDIFresult r;  int i, j, k;  SDIFmem_Frame f;  SDIFmem_Matrix m;  int uniqueFramesAdded;    for(i = 0, uniqueFramesAdded = 0; i < numFrames; i++)  {    //  create a frame at a random time on functionDomain    if(!(f = SDIFmem_CreateEmptyFrame()))      break;    SDIF_Copy4Bytes(f->header.frameType, frameType);    f->header.time = randomTime();    unit_log(LOG_INFO2, "%f\n", f->header.time);        //  create a matrix, add to frame    if(!(m = SDIFutil_CreateMatrix(columns, rows, SDIF_FLOAT64, matrixType)))      break;    if(ESDIF_SUCCESS != (r = SDIFmem_AddMatrix(f, m)))      break;        //  fill in the matrix with random data    for(j = 0; j < rows; j++)      for(k = 0; k < columns; k++)      {        if(rand() > 4000)          SDIFutil_SetMatrixCell(m, k, j, rand() / (sdif_float64)RAND_MAX);        else          //  put some NaN's in at random locations          SDIFutil_SetMatrixCell(m, k, j, NAN);      }        //  add frame to buffer    r = SDIFbuf_InsertFrame(b, f, ESDIF_INSERT_DONT_REPLACE);    if(r == ESDIF_SUCCESS)      uniqueFramesAdded++;      continue;    if(r == ESDIF_FRAME_ALREADY_EXISTS)      continue;    else      break;  }  if(i != numFrames)    //  something bad happened    return 0;    return uniqueFramesAdded;}static SDIFresult addJunkToBuffer(SDIFbuf_Buffer buf, int junkCount){  int i;  SDIFresult r;    for(i = 0, r = ESDIF_SUCCESS; i < junkCount; i++)  {    SDIFmem_Frame f = SDIFmem_CreateEmptyFrame();    if(!f)      return ESDIF_OUT_OF_MEMORY;    f->header.time = randomTime();    SDIF_Copy4Bytes(f->header.frameType, "----");        r = SDIFbuf_InsertFrame(buf, f, ESDIF_INSERT_DONT_REPLACE);    if((r != ESDIF_SUCCESS) && (r != ESDIF_FRAME_ALREADY_EXISTS))      return r;  }    return ESDIF_SUCCESS;}static void randomTimeInit(void){  int i;    for(i = 0; i < NUM_RANDOM_TIMES; i++)    randomTimeList[i] = rand() % FUN_DOMAIN;}static sdif_float64 randomTime(void){  return randomTimeList[(NUM_RANDOM_TIMES * rand()) / (RAND_MAX + 1)];}static Boolean matrixAlmostEqual(SDIFmem_Matrix m1, SDIFmem_Matrix m2){  int i, j;    if(!m1 && !m2)    return TRUE;  if(!m1 || !m2)    return FALSE;    for(i = 0; i < m1->header.rowCount; i++)    for(j = 0; j < m1->header.columnCount; j++)    {      sdif_float64 v1 = SDIFutil_GetMatrixCell(m1, j, i);      sdif_float64 v2 = SDIFutil_GetMatrixCell(m2, j, i);      if(isnan(v1))      {        if(!isnan(v2))          return FALSE;      }      else if(fabs(v1 - v2) > 0.000001)        return FALSE;    }  return TRUE;}////  do linear interpolation without using sdif-interp.c//static sdif_float64 interpolateOneCell(SDIFbuf_Buffer buf,                                const char *matrixType,                                sdif_int32 column,                                sdif_int32 row,                                sdif_float64 time                                ){  SDIFmem_Frame f;  SDIFmem_Matrix m1, m2;  sdif_float64 t1, t2, f1, f2;    if(!(f = SDIFbuf_GetFirstFrame(buf)))    return -1;  //  in a real app, would need to signal errors some other way!                //  (but caller already checked to make sure we have previous + next data)  if(!(m1 = SDIFbuf_GetMatrixNearby(f, matrixType, time, ESDIF_SEARCH_BACKWARDS, &t1)))    return -1;    if(!(m2 = SDIFbuf_GetMatrixNearby(f, matrixType, time, ESDIF_SEARCH_FORWARDS, &t2)))    return -1;  f1 = SDIFutil_GetMatrixCell(m1, column, row);  f2 = SDIFutil_GetMatrixCell(m2, column, row);  return ((time - t1) * (f2 - f1) / (t2 - t1)) + f1;}////  do linear interpolation using sdif-interp.c//static SDIFmem_Matrix doInterpolate(SDIFinterp_Interpolator it,                                     SDIFbuf_Buffer b,                                    const char *matrixType,                                    sdif_float64 time                                    ){  SDIFmem_Matrix matrixOut;  SDIFresult r;  va_list va;  if(!(matrixOut = SDIFutil_CreateMatrix(NUM_COLUMNS, NUM_ROWS, SDIF_FLOAT64, "----")))    return NULL;      r = SDIFinterp_GetMatrix(it,                            b,                            matrixType,                            time,                            ESDIF_NAN_ACTION_KEEP_LOOKING,                           matrixOut,                           1                           );  if((r != ESDIF_SUCCESS) && (r != ESDIF_NOT_AVAILABLE))  {    SDIFmem_FreeMatrix(matrixOut);    return NULL;  }    return matrixOut;}////  prepare interpolator instance//static SDIFinterp_Interpolator createInterpolator(SDIFinterp_InterpolatorFn interpFn){  SDIFinterp_Interpolator it;    if(!(it = SDIFinterp_Create(NUM_COLUMNS)))    return NULL;    if(ESDIF_SUCCESS != SDIFinterp_SetAllInterpolatorFn(it, interpFn))    return NULL;    return it;}////  linear interpolator (includes NaN handling)//static SDIFresult unit_LinearInterpolator(SDIFmem_Frame nearbyFrame,                                          const char *matrixType,                                          sdif_int32 column,                                          sdif_float64 time,                                          SDIFactionOnNaN actionOnNaN,                                          SDIFmem_Matrix matrixOut,                                          va_list args                                          ){  SDIFresult r;  int rows = matrixOut->header.rowCount;  int i;    //  loop to compute value for this column in each row  for(i = 0; i < matrixOut->header.rowCount; i++)  {    sdif_float64 t1, t2, v1, v2;    //  first attempt to find value at requested time    r = SDIFbuf_GetValueNearby(nearbyFrame,                               matrixType,                               column,                               i,                               time,                               ESDIF_SEARCH_BACKWARDS,                               actionOnNaN,                               &v1,                               &t1                               );    if(r == ESDIF_NOT_AVAILABLE)    {      //  earlier neighbor value not found -- write NaN into output matrix      SDIFutil_SetMatrixCell(matrixOut, column, i, NAN);      continue;    }    if(r != ESDIF_SUCCESS)      //  failed: couldn't find value at or before requested time      return r;        if(time == t1)      //  exact time match found, no need to interpolate      SDIFutil_SetMatrixCell(matrixOut, column, i, v1);        else    {      //  need to interpolate, so we need a second value      r = SDIFbuf_GetValueNearby(nearbyFrame,                                 matrixType,                                 column,                                 i,                                 time,                                 ESDIF_SEARCH_FORWARDS,                                 actionOnNaN,                                 &v2,                                 &t2                                 );      if(r == ESDIF_NOT_AVAILABLE)      {        //  later neighbor value not found -- write NaN into output matrix        SDIFutil_SetMatrixCell(matrixOut, column, i, NAN);        continue;      }      if(r != ESDIF_SUCCESS)        //  failed: couldn't find value after requested time        return r;      //  success: do linear interpolation      SDIFutil_SetMatrixCell(matrixOut, column, i, v1 + ((time - t1) * (v2 - v1) / (t2 - t1)));    }  }    return ESDIF_SUCCESS;}////  Lagrange/Waring interpolator (includes NaN handling)//  var arg #1 = requested polynomial degree//static SDIFresult unit_LagrangeInterpolator(SDIFmem_Frame nearbyFrame,                                            const char *matrixType,                                            sdif_int32 column,                                            sdif_float64 time,                                            SDIFactionOnNaN actionOnNaN,                                            SDIFmem_Matrix matrixOut,                                            va_list args                                            ){#define MAX_DEGREE 100  SDIFresult r;  int i;  sdif_int32 degree;  //  read requested degree arg; check against MAX_DEGREE  degree = va_arg(args, sdif_int32);  if(degree < 1)    return ESDIF_BAD_PARAM;  if(degree > MAX_DEGREE)    return ESDIF_BAD_PARAM;    //  loop to compute value for this column in each row  for(i = 0; i < matrixOut->header.rowCount; i++)  {    sdif_float64 t1, val;    sdif_float64 t[MAX_DEGREE + 1], v[MAX_DEGREE + 1];    //  write NaN into output matrix if we can't find at least    //  one earlier + one later neighbor value        r = SDIFbuf_GetValueNearby(nearbyFrame,                               matrixType,                               column,                               i,                               time,                               ESDIF_SEARCH_BACKWARDS,                               actionOnNaN,                               &val,                               &t1                               );    r |= SDIFbuf_GetValueNearby(nearbyFrame,                                matrixType,                                column,                                i,                                time,                                ESDIF_SEARCH_FORWARDS,                                actionOnNaN,                                &val,                                &t1                                );    if(r == ESDIF_NOT_AVAILABLE)    {      SDIFutil_SetMatrixCell(matrixOut, column, i, NAN);      continue;    }    //  if no exact time match, interpolate from a full set of neighboring values    if(t1 != time)    {      int j, k;      sdif_int32 n;      //  get the neighboring values      r = SDIFbuf_GetNeighborValues(nearbyFrame,                                    matrixType,                                    column,                                    i,                                    time,                                    degree + 1,                                    actionOnNaN,                                    t,                                    v,                                    &n);            if(n < 2)        //  fail: didn't even find 2 neighboring values        return ESDIF_NOT_AVAILABLE;            //  do the interpolation (Lagrange/Waring method)      val = 0;      for(j = 0; j < n; j++)      {        sdif_float64 vp = 1.0;        for(k = 0; k < n; k++)        {          if(k == j)            continue;          vp *= (time - t[k]) / (t[j] - t[k]);        }        val += v[j] * vp;      }    }    //  store the result    SDIFutil_SetMatrixCell(matrixOut, column, i, val);  }    return ESDIF_SUCCESS;}