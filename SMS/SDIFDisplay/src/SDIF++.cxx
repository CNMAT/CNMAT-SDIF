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

#include "SDIF++.H"

SDIFMatrixHeader::SDIFMatrixHeader(
		char* type,sdif_int32 matrixDataType,int columnCount,int rowCount)
{
	memcpy(mHeaderStruct.matrixType,type,4);
	mHeaderStruct.matrixDataType = matrixDataType;
	mHeaderStruct.rowCount = rowCount;
	mHeaderStruct.columnCount = columnCount;
}

SDIFMatrix::SDIFMatrix(
		char* type,sdif_int32 matrixDataType,int columnCount,int rowCount)
:mHeader(type,matrixDataType,columnCount,rowCount)
{
	mData = 0;
	mNext = 0;
	
	int sz = GetDataSize();

	if (sz == 0) {
	  mData = 0;
	} else {
		mData = new unsigned char[sz];
#ifdef __DEBUG__
		printf("  allocating %d : %x in matrix\n",sz,mData,this);		fflush(stdout);
#endif
		if (mData == 0) {
		}
	}

}

SDIFFrame::SDIFFrame(char* type,sdif_float64 time,sdif_int32 id)
:mHeader(type,time,id)
{
	mFirst = 0;
	mPrev = mNext = mNextInStream = mPrevInStream = 0;	
}

SDIFFrame::~SDIFFrame()
{
	SDIFMatrix* matrix = mFirst;
	while (matrix) {
		SDIFMatrix* next = matrix->mNext;
#ifdef __DEBUG__
		printf("deleting SDIFMatrix %lx\n",matrix); fflush(stdout);fflush(stdout);
#endif
		delete matrix;
		matrix=next;
	}
}


SDIFFrameHeader::SDIFFrameHeader(char* type,sdif_float64 time,sdif_int32 id)
{
	memcpy(mHeaderStruct.frameType,type,4);
	mHeaderStruct.time = time;
	mHeaderStruct.streamID = id;
	mHeaderStruct.matrixCount = 0;
	mHeaderStruct.size = sizeof(mHeaderStruct);
}

SDIFResult SDIFFile::SkipMatrix(const SDIFMatrixHeader& header)
{
	mResult = SDIF_SkipMatrix(&header.mHeaderStruct,mFile);
	return mResult;
}

SDIFFile::SDIFFile(const char* filename,SDIFFileMode mode)
{
	mMode = mode;
	mFile = 0;
	mFirst = 0;
	switch (mMode) {
	case read:
		mResult = SDIF_OpenRead(filename,&mFile);
		break;
	case write:
		mResult = SDIF_OpenWrite(filename,&mFile);
		break;
	}
}

SDIFFile::~SDIFFile()
{
	SDIFFrame* frame = mFirst;
	while (frame) {
		SDIFFrame* next=frame->mNext;
		delete frame;
		frame=next;
	}
	if (mFile) {
		switch (mMode) {
		case read:
			mResult = SDIF_CloseRead(mFile);
			break;
		case write:
			mResult = SDIF_CloseWrite(mFile);
			break;
		}	
	}
}

SDIFResult SDIFFile::Read(void)
{
	if (mFile==0) return ESDIF_READ_FAILED;
	
	SDIFFrame* frame = new SDIFFrame;
	SDIFFrame* prev=0;
	
	while (Read(*frame)==ESDIF_SUCCESS)
	{
		if (prev) {
			prev->mNext = frame;
			frame->mPrev = prev;
		}else{
			mFirst = frame;
		}
		prev=frame;
		frame = new SDIFFrame;
	}
	delete frame;
	
	// Reading the frame will eventually give an error
	// when all frames are read; 
	// 
	if (feof(mFile)) {
		mResult = ESDIF_SUCCESS; 
	}
	return mResult;
}

SDIFResult SDIFFile::Write(const SDIFFrameHeader& header)
{
	mResult = SDIF_WriteFrameHeader(&header.mHeaderStruct,mFile);
	return mResult;
}

SDIFResult SDIFFile::Read(SDIFFrameHeader& header)
{
	mResult = SDIF_ReadFrameHeader(&header.mHeaderStruct,mFile);
	return mResult;
}

SDIFResult SDIFFile::Write(const SDIFMatrixHeader& header)
{
	mResult = SDIF_WriteMatrixHeader(&header.mHeaderStruct,mFile);
	return mResult;
}

SDIFResult SDIFFile::Read(SDIFMatrixHeader& header)
{
	mResult = SDIF_ReadMatrixHeader(&header.mHeaderStruct,mFile);
	return mResult;
}


SDIFresult SDIFFile::ReadMatrixData(
	void *putItHere,const SDIFMatrixHeader& header)
{
	mResult = SDIF_ReadMatrixData(putItHere,mFile,&header.mHeaderStruct);
	return mResult;
}

SDIFResult SDIFFile::Read(SDIFMatrix& matrix)
{
	mResult = Read(matrix.mHeader);
	if (mResult) return mResult;

	int sz = matrix.GetDataSize();

	if (sz == 0) {
	  matrix.mData = 0;
	} else {
		matrix.mData = new unsigned char[sz];

#ifdef __DEBUG__
		printf("  allocating %d : %x in matrix\n",sz,matrix.mData,&matrix);		fflush(stdout);
#endif

		if (matrix.mData == 0) {
			mResult = ESDIF_OUT_OF_MEMORY;
			return mResult;
		}
	}
  mResult = ReadMatrixData(matrix.mData,matrix.mHeader);

	{
		int paddingNeeded = matrix.PaddingRequired();
		if (paddingNeeded) {
			char pad[8] = "\0\0\0\0\0\0\0";
			mResult = SDIF_Read1(pad, paddingNeeded, mFile);
      if (mResult) return mResult;
		}
	}


	return mResult;
}

SDIFMatrix::~SDIFMatrix() {
	if (mData != 0) {
#ifdef __DEBUG__
		printf("  deleting data %lx from matrix %lx\n",mData,this);		fflush(stdout);
#endif
		delete mData;
	}
}

void SDIFFrame::Repaire(void) {
  sdif_int32 numBytes;
  sdif_int32 numMatrices;

  SDIFMatrix *m;

	/* The rest of the frame header: */
	numBytes = sizeof(SDIF_FrameHeader) - 8;
	numMatrices = 0;

  for (m = mFirst; m != 0; m=m->mNext) {
		++numMatrices;
		numBytes += sizeof(SDIF_MatrixHeader);
		numBytes += m->GetDataSize();
	}

  mHeader.mHeaderStruct.size = numBytes;
  mHeader.mHeaderStruct.matrixCount = numMatrices;
}
    

SDIFResult SDIFFile::ReadContents(SDIFFrame& frame) {
 /* The user has just read the header for this frame; now we have to read
    all the frame's matrices, put them in an SDIFmem_Matrix linked list,
    and then stuff everything into an SDIFmem_Frame. */
  SDIFMatrix* matrix,*prev=0;
  int i;

  frame.mPrev = frame.mNext = 0;
	frame.mFirst = 0;
	
  for (i = 0; i < frame.MatrixCount(); ++i) {
		matrix = new SDIFMatrix;

#ifdef __DEBUG__
		printf("allocating new SDIFMatrix %lx\n",matrix);		fflush(stdout);
#endif

		if (matrix == 0) {
			mResult = ESDIF_OUT_OF_MEMORY;
			return mResult;
		}
		if (prev) {
			prev->mNext = matrix;
		}else{
			frame.mFirst = matrix;
		}
		prev=matrix;

		mResult = Read(*matrix);

		if (mResult) return mResult;
  }
	
  return mResult;
}


SDIFResult SDIFFile::Read(SDIFFrame& frame) {
	mResult = Read(frame.mHeader);
	if (mResult) return mResult;
	
  return ReadContents(frame);
}

SDIFResult SDIFFrame::Add(SDIFMatrix *m) {
  int sz;
  SDIFMatrix *p, *last;

  m->mNext = NULL;

  p = mFirst;
  if (p == NULL) {
      mFirst = m;
  } else {
      while (p != NULL) {
				if (SDIF_Char4Eq(
					p->Type(), 
					m->Type())) {
				return ESDIF_DUPLICATE_MATRIX_TYPE_IN_FRAME;
		  }
		  last = p;
		  p = p->mNext;
	  }
		last->mNext = m;
  }

  sz = m->GetDataSize();

  mHeader.mHeaderStruct.size += sz + sizeof(SDIF_MatrixHeader);
  mHeader.mHeaderStruct.matrixCount++;
  return ESDIF_SUCCESS;
}

SDIFResult SDIFFile::Write(const SDIFMatrix& m) {
  sdif_int32 sz, numElements;
  int paddingNeeded;

	mResult = Write(m.mHeader);
	if (mResult) return mResult;
	
  sz = m.GetDataTypeSize();
  numElements = (m.RowCount() * m.ColumnCount());

  switch (sz) {
	case 1:
	  mResult = SDIF_Write1(m.mData, numElements, mFile);
	  break;
	case 2:
	  mResult = SDIF_Write2(m.mData, numElements, mFile);
	  break;
	case 4:
	  mResult = SDIF_Write4(m.mData, numElements, mFile);
	  break;
	case 8:
	  mResult = SDIF_Write8(m.mData, numElements, mFile);
	  break;
	default:
	  mResult = ESDIF_BAD_MATRIX_DATA_TYPE;
  }

  if (mResult) return mResult;
  paddingNeeded = m.PaddingRequired();
  if (paddingNeeded) {
		char pad[8] = "\0\0\0\0\0\0\0";
		mResult = SDIF_Write1(pad, paddingNeeded, mFile);
  }
  return mResult;
}

SDIFResult SDIFFile::Write(const SDIFFrame& f) {
  SDIFMatrix* p;

	mResult = Write(f.mHeader);
  if (mResult) return mResult;

  for (p = f.mFirst; p != NULL; p = p->mNext) {
		mResult = Write(*p);
	  if (mResult) return mResult;
  }

  return mResult;
}

SDIFStream::SDIFStream(SDIFFrame* f){
	mFirst = mCurrent = 0;
	Init(f);
}

SDIFStream::SDIFStream(SDIFFrame* f,int id)
{
	mFirst = mCurrent = 0;
	while (f && f->StreamID()!=id) {
			f=f->mNext;		
	}
	Init(f);
}

SDIFStream::SDIFStream(SDIFFrame* f,char* type)
{
	mFirst = mCurrent = 0;
	while (f && !f->CheckType(type)) {
			f=f->mNext;		
	}
	Init(f);	
}

void SDIFStream::Init(SDIFFrame* f) {
	mFirst = mCurrent = f;
	SDIFFrame* prev = 0;
	while (f && f->mNext) {
		prev=f;
		f=f->mNext;
		while (f && f->StreamID()!=prev->StreamID()) {
			f=f->mNext;
		}
		if (f) {
			prev->mNextInStream = f;
			f->mPrevInStream = prev;
		}
	}
	mLast=prev;
}
