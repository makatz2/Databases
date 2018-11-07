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
	clockHand = clokcHand++;
	clockHand = clockHand%numBufs;
}

void BufMgr::allocBuf(FrameId & frame) 
{
	int numFramesChecked = 0;
	while(numFramesChecked < numBufs){
		numFramesChecked++;
		advanceClock();
		if(bufDescTable[clockHand].valid != false){
			if(bufDescTable[clockHand].refbit == true){
				refbit = false;
				continue;
			}else{
				if(bufDescTable[clockHand].pinCount == 0){
					if(bufDescTable[clockHand].dirty == true){
						bufDescTable[clockHand].file->writePage(bufDescTable[clockHand].pageNo);
						hashTable.remove(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
						bufDescTable[clockHand].clear();
					}	
				}else{
					continue;
				}
			}
		}
		set(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
		frame = bufDescTable[clockHand].frameNo;	
	}
}

	
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{
}


void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
    FrameId fid = -1;
    // Check if the frame is in the hashtable
    try{
        hashTable->lookup(file, pageNo, fid);
        // Frame found. Check pin count to check for 0.
        int pins = bufDescTable[fid].pinCnt;
	if(pins == 0){
            // Pin count is already 0. Throw PAGENOTPINNED exception.
	    throw PageNotPinnedException(file->filename(), pageNo, fid);
            break;
	}
	//Pin value is not 0. Decrement by 1.
	bufDescTable[fid].pinCnt--;
	// Check dirty boolean and set if true
	if(dirty){
            bufDescTable[fid].dirty = true;
	}
    }
    catch(HashNotFoundException e){
        // Do nothing if hash is not found
	return;
    }
}

void BufMgr::flushFile(const File* file) 
{
}

void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
	Page newPage = file->allocatePage();
	FrameID *newFrame;
	allocBuf(newFrame);
	hashTable.insert(file, pageNo, newFrame);
	page
}	

void BufMgr::disposePage(File* file, const PageId PageNo)
{
	FrameId fid = -1;
	// First make sure page is allocated a frame in the buffer pool
	try{
		hashTable->lookup(file, pageNo, fid);
		// If true, free the frame in bufDescTable
		bufDescTable[fid].Clear();
		// Remove the entry from the hash table
		hashTable->remove(file, PageNo);
		// Finally, delete the page from the file itself
		file->deletePage(PageNo);	
	}
	catch(HashNotFoundException e){
		return;
	}
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
