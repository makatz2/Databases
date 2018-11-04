#include <iostrean>
#include bufMgr.h

class BufMgr{
	BufMgr(std :: uint32_t bufs){

	}
	~BufMgr(){

	} 
	void allocBuf(FrameId & frame){

	}
	void advanceClock() {
	
	} 
	void readPage(File *file, const PageId PageNo, Page*& page){

	}
	void unPinPage (File *file, const PageId PageNo, const bool dirty){

	}
	void allocPage (File *file, PageId& PageNo, Page*& page){

	}
	void disposePage (File *file, const PageId pageNo){

	}
	void flushFile(constFile *file){

	}
};
