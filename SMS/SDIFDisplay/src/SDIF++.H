/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************
 *   Maarten de Boer <maarten.deboer@iua.upf.es>, 1999                     *
 *   Music Technology Group                                                *
 *   Institut de l'Audiovisual, Universitat Pompeu Fabra, Barcelona, Spain *
 *   http://www.iua.upf.es/mtg/                                            *
 ***************************************************************************/

#ifndef __SDIF_CPP__
#define __SDIF_CPP__

//D/ These classes are C++ wrappers of the CNMAT sdif library
 
extern "C" {
	#include <stdio.h>
	#include "sdif.h"
	#include "sdif-mem.h"
}

//#define __DEBUG__

enum SDIFFileMode{
	read=0,
	write=1
};

typedef SDIFresult SDIFResult;

class SDIFFrameHeader
{
friend class SDIFFile;
friend class SDIFFrame;
protected:
	SDIF_FrameHeader mHeaderStruct;
public:
	SDIFFrameHeader(char* type="\0\0\0\0",sdif_float64 time=0.,sdif_int32 id=0);
	int MatrixCount(void) { return mHeaderStruct.matrixCount; }
	const char* Type() const { return mHeaderStruct.frameType; }
	int CheckType(char* type) {
		return !memcmp(type,Type(),4);
	}
	double Time(void) {
		return mHeaderStruct.time;
	}
	int StreamID(void) {
		return mHeaderStruct.streamID;	
	}
};

class SDIFMatrixHeader
{
friend class SDIFFile;
friend class SDIFMatrix;
protected:
	SDIF_MatrixHeader mHeaderStruct;
public:
	SDIFMatrixHeader(
			char* type="\0\0\0\0",
			sdif_int32 matrixDataType=0,
			int columnCount=0,int rowCount=0);

	int GetDataSize() const;
	int GetDataTypeSize() const;
	int PaddingRequired() const;
	const char* Type() const { return mHeaderStruct.matrixType; }
	int RowCount() const { return mHeaderStruct.rowCount; }
	int ColumnCount() const { return mHeaderStruct.columnCount; }
	int CheckType(char* type) {
		return !memcmp(type,Type(),4);
	}
};

class SDIFMatrix
{
friend class SDIFFrame;
friend class SDIFFile;
protected:
	SDIFMatrixHeader mHeader;
	void* mData;
	SDIFMatrix* mNext;
public:
	SDIFMatrix(
			char* type="\0\0\0\0",
			sdif_int32 matrixDataType=0,
			int columnCount=0,int rowCount=0);

	~SDIFMatrix();
	int GetDataSize() const { return mHeader.GetDataSize(); }
	int GetDataTypeSize() const { return mHeader.GetDataTypeSize(); }
	int PaddingRequired() const { return mHeader.PaddingRequired(); }
	const char* Type() const { return mHeader.Type(); }
	int RowCount() const { return mHeader.RowCount(); }
	int ColumnCount() const { return mHeader.ColumnCount(); }
	void Set(int col,int row,sdif_int32 value);
	void Set(int col,int row,sdif_float32 value);
	void Get(int col,int row,sdif_int32 &value);
	void Get(int col,int row,sdif_float32 &value);
	int CheckType(char* type) { return mHeader.CheckType(type);}
};

inline void SDIFMatrix::Set(int col,int row,sdif_int32 value)
{
	switch (mHeader.mHeaderStruct.matrixDataType) {
	case SDIF_FLOAT32: 
		((sdif_float32*)mData)[col+row*ColumnCount()]=(sdif_float32)value; break;
	case SDIF_FLOAT64: 
		((sdif_float64*)mData)[col+row*ColumnCount()]=(sdif_float64)value; break;
	case SDIF_INT32:  
		((sdif_int32*  )mData)[col+row*ColumnCount()]=(sdif_int32)value; break;
	case SDIF_UINT32:
		((sdif_uint32* )mData)[col+row*ColumnCount()]=(sdif_uint32)value; break;
	}
} 

inline void SDIFMatrix::Set(int col,int row,sdif_float32 value)
{
	switch (mHeader.mHeaderStruct.matrixDataType) {
	case SDIF_FLOAT32: 
		((sdif_float32*)mData)[col+row*ColumnCount()]=(sdif_float32)value; break;
	case SDIF_FLOAT64: 
		((sdif_float64*)mData)[col+row*ColumnCount()]=(sdif_float64)value; break;
	case SDIF_INT32:  
		((sdif_int32*  )mData)[col+row*ColumnCount()]=(sdif_int32)value; break;
	case SDIF_UINT32:
		((sdif_uint32* )mData)[col+row*ColumnCount()]=(sdif_uint32)value; break;
	}
}

inline void SDIFMatrix::Get(int col,int row,sdif_int32 &value)
{
	switch (mHeader.mHeaderStruct.matrixDataType) {
	case SDIF_FLOAT32: 
		value=(sdif_int32)((sdif_float32*)mData)[col+row*ColumnCount()]; break;
	case SDIF_FLOAT64: 
		value=(sdif_int32)((sdif_float64*)mData)[col+row*ColumnCount()]; break;
	case SDIF_INT32:   
		value=(sdif_int32)((sdif_int32*  )mData)[col+row*ColumnCount()]; break;
	case SDIF_UINT32:  
		value=(sdif_int32)((sdif_uint32* )mData)[col+row*ColumnCount()]; break;
	}
}

inline void SDIFMatrix::Get(int col,int row,sdif_float32 &value)
{
	switch (mHeader.mHeaderStruct.matrixDataType) {
	case SDIF_FLOAT32: 
		value=(sdif_float32)((sdif_float32*)mData)[col+row*ColumnCount()]; break;
	case SDIF_FLOAT64: 
		value=(sdif_float32)((sdif_float64*)mData)[col+row*ColumnCount()]; break;
	case SDIF_INT32:   
		value=(sdif_float32)((sdif_int32*  )mData)[col+row*ColumnCount()]; break;
	case SDIF_UINT32:  
		value=(sdif_float32)((sdif_uint32* )mData)[col+row*ColumnCount()]; break;
	}
}

class SDIFFrame {
friend class SDIFFile;
friend class SDIFStream;
protected:
	SDIFFrameHeader mHeader;
	SDIFMatrix* mFirst;
	SDIFFrame *mPrev;
	SDIFFrame *mNext;
	SDIFFrame *mNextInStream;
	SDIFFrame *mPrevInStream;
public:
	SDIFFrame(char* type="\0\0\0\0",sdif_float64 time=0.,sdif_int32 id=0);
	~SDIFFrame();

	SDIFFrame* Next(void){ return mNext; }
	SDIFFrame* NextInStream(void){ return mNextInStream; }

	SDIFResult Add(SDIFMatrix* matrix);
	SDIFMatrix* Matrix(int index);

	void Repaire(void);
	int MatrixCount(void) { return mHeader.MatrixCount(); }
	const char* Type() const { return mHeader.Type(); }
	int CheckType(char* type) { return mHeader.CheckType(type);}

	double Time(void) {
		return mHeader.Time();
	}
	int StreamID(void) {
		return mHeader.StreamID();	
	}
};

inline SDIFMatrix* SDIFFrame::Matrix(int index)
{
	SDIFMatrix* m=0;
	m=mFirst;
	
	while (m && index) {
		m=m->mNext;
		index--;
	}
	return m;
}

inline int SDIFMatrixHeader::GetDataSize() const
{
	return SDIF_GetMatrixDataSize(&mHeaderStruct);
}
inline int SDIFMatrixHeader::GetDataTypeSize() const
{
	return SDIF_GetMatrixDataTypeSize(mHeaderStruct.matrixDataType);
}
inline int SDIFMatrixHeader::PaddingRequired() const
{
	return SDIF_PaddingRequired(&mHeaderStruct);
}

class SDIFFile
{
protected:
	FILE*	mFile;
	SDIFResult mResult;
	SDIFFileMode mMode;
	SDIFFrame* mFirst;
public:
	SDIFFile(const char* filename,SDIFFileMode mode);
	~SDIFFile();

	SDIFFrame* First(void) { return mFirst; }

	bool Ok(void) {
		return mResult == ESDIF_SUCCESS;
	}

	SDIFResult Read(void);

	SDIFResult Write(const SDIFFrameHeader& header);
	SDIFResult Read(SDIFFrameHeader& header);
	SDIFResult Write(const SDIFMatrixHeader& header);
	SDIFResult Read(SDIFMatrixHeader& header);

	SDIFResult Read(SDIFFrame& frame);
	SDIFResult ReadContents(SDIFFrame& frame);
	SDIFResult Write(const SDIFFrame& frame);

	SDIFResult Read(SDIFMatrix& matrix);
	SDIFResult Write(const SDIFMatrix& matrix);

	SDIFResult SkipMatrix(const SDIFMatrixHeader& header);
	SDIFresult ReadMatrixData(void *putItHere,const SDIFMatrixHeader& header);
};

class SDIFStream
{
protected:
	SDIFFrame* mFirst;
	SDIFFrame* mLast;
	SDIFFrame* mCurrent;
public:
	SDIFStream(SDIFFrame* f);
	SDIFStream(SDIFFrame* f,int id);
	SDIFStream(SDIFFrame* f,char* type);
	
	void Init(SDIFFrame* f);

	SDIFFrame* First(void) { return mFirst; }
	SDIFFrame* Last(void) { return mLast; }

	SDIFFrame* Start(SDIFFrame* f = 0);
	SDIFFrame* Current();
	SDIFFrame* Forward();
};
		
inline SDIFFrame* SDIFStream::Start(SDIFFrame* f) {
	if (f) {
		mCurrent = f;
	}else{
		mCurrent = mFirst;
	}
	return mCurrent;
}

inline SDIFFrame* SDIFStream::Current() {
	return mCurrent;
}

inline SDIFFrame* SDIFStream::Forward() {
	mCurrent = mCurrent->mNextInStream;
	return mCurrent;
}

#endif
