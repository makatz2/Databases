/**
 * Name: Michael Katz, ID: 9070102042
 * Name: Emmet Ryan, ID: 9069927185
 * Name: 
 *
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
	uint32_t  pinnedFrames = 0;
	while(pinnedFrames <  numBufs){
		advanceClock();
		if(bufDescTable[clockHand].valid){
			if(bufDescTable[clockHand].refbit){
				bufDescTable[clockHand].refbit = false;
				continue;
			}else{
				if(bufDescTable[clockHand].pinCnt == 0){
					if(bufDescTable[clockHand].dirty == true){
						(bufDescTable[clockHand].file)->writePage(bufPool[clockHand]);
					}	
					hashTable->remove(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
					bufDescTable[clockHand].Clear();
					frame = clockHand;
					break;	
				}else{
					pinnedFrames++;
					continue;
				}
			}
		}else{
			frame = clockHand;
			break;	
		}

	}
	if(pinnedFrames == numBufs){
		throw BufferExceededException();
	}
}

	
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{
	FrameId fid = -1;
	try{
		hashTable->lookup(file, pageNo, fid);
		// case2-page is in the bufPool
		bufDescTable[fid].pinCnt++;
		bufDescTable[fid].refbit = true;
		page = &bufPool[fid];
		
	}catch(HashNotFoundException e){	
		//case1-page is not in the bufPool
		allocBuf(fid);
		bufPool[fid]  = file->readPage(pageNo);
		hashTable->insert(file, pageNo, fid);
		bufDescTable[fid].Set(file, pageNo);
		page = &bufPool[fid];;
	}
}


void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
    FrameId fid = -1;
    // Check if the frame is in the hashtable
    try{
        hashTable->lookup(file, pageNo, fid);
        // Frame found. Check pin count to check for 0.
	if(bufDescTable[fid].pinCnt == 0){
            // Pin count is already 0. Throw PAGENOTPINNED exception.
	    throw PageNotPinnedException(file->filename(), pageNo, fid);
	}else{
		//Pin value is not 0. Decrement by 1.
		bufDescTable[fid].pinCnt--;
		// Check dirty boolean and set if true
		if(dirty){
            		bufDescTable[fid].dirty = true;
		}
	}
    }
    catch(HashNotFoundException e){
        // Do nothing if hash is not found
    }
}

void BufMgr::flushFile(const File* file) 
{
	for(uint32_t i = 0; i < numBufs; i++){
		if(bufDescTable[i].file == file){
			if(bufDescTable[i].pinCnt != 0){
				throw PagePinnedException(file->filename(),bufDescTable[i].pageNo, i);
			}
			if(bufDescTable[i].valid == false){
				throw BadBufferException(i, bufDescTable[i].dirty, bufDescTable[i].valid, bufDescTable[i].refbit);
			}
			if(bufDescTable[i].dirty){
				(bufDescTable[i].file)->writePage(bufPool[i]);
				bufDescTable[i].dirty = false;
			}
			hashTable->remove(file, bufDescTable[i].pageNo);
			bufDescTable[i].Clear();
		}
	}

}

void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
	Page tempPage = file->allocatePage();
	pageNo = tempPage.page_number();
	FrameId fid;
	allocBuf(fid);
	bufPool[fid] = tempPage;
	bufDescTable[fid].Set(file, pageNo);
	hashTable->insert(file, pageNo, fid);
	page = &bufPool[fid];
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
