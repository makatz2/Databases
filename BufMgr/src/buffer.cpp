/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include <memory>
#include <iostream>
#include "buffer.h"
#include "exceptions/buffer_exceeded_exception.h"
#include "exceptions/page_not_pinned_exception.h"
#include "exceptions/page_pinned_exception.h"
#include "exceptions/bad_buffer_exception.h"
#include "exceptions/hash_not_found_exception.h"

namespace badgerdb { 

BufMgr::BufMgr(std::uint32_t bufs)
	: numBufs(bufs) {
	bufDescTable = new BufDesc[bufs];

  for (FrameId i = 0; i < bufs; i++) 
  {
  	bufDescTable[i].frameNo = i;
  	bufDescTable[i].valid = false;
  }

  bufPool = new Page[bufs];

	int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
  hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

  clockHand = bufs - 1;
}


BufMgr::~BufMgr() {
}

void BufMgr::advanceClock()
{
	clockHand = clockHand + 1;
	clockHand = clockHand%numBufs;
}

void BufMgr::allocBuf(FrameId & frame) 
{
	uint32_t  numFramesChecked = 0;
	while(numFramesChecked < numBufs){
		numFramesChecked++;
		advanceClock();
		if(bufDescTable[clockHand].valid != false){
			if(bufDescTable[clockHand].refbit == true){
				bufDescTable[clockHand].refbit = false;
				continue;
			}else{
				if(bufDescTable[clockHand].pinCnt == 0){
					if(bufDescTable[clockHand].dirty == true){
						bufDescTable[clockHand].file->writePage(bufPool[clockHand]);
						hashTable->remove(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
						bufDescTable[clockHand].Clear();
					}	
				}else{
					continue;
				}
			}
		}

		bufDescTable->Set(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
		frame = bufDescTable[clockHand].frameNo;	
	}
}

	
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{
}


void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
}

void BufMgr::flushFile(const File* file) 
{
}

void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
	page = &file->allocatePage();
	pageNo = page->page_number();
	FrameId newFrame;
	allocBuf(newFrame);
	hashTable->insert(file, pageNo, newFrame);
	bufDescTable->Set(file, pageNo);
}	

void BufMgr::disposePage(File* file, const PageId PageNo)
{
    
	//int *tempFrameNo = -1;
	//hashTable->lookup(file, pageNp, tempFrameNo);
	//if(tempFrameNo != -1){
	//	bufDescTbl->clear() 
	//}
	//file->deletePage(PageNo);
}

void BufMgr::printSelf(void) 
{
  BufDesc* tmpbuf;
	int validFrames = 0;
  
  for (std::uint32_t i = 0; i < numBufs; i++)
	{
  	tmpbuf = &(bufDescTable[i]);
		std::cout << "FrameNo:" << i << " ";
		tmpbuf->Print();

  	if (tmpbuf->valid == true)
    	validFrames++;
  }

	std::cout << "Total Number of Valid Frames:" << validFrames << "\n";
}

}
