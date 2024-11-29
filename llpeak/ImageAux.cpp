
#include "ImageAux.hpp"

#ifdef USE_THREAD
#include "ImageUtilF.hpp"
#include "FPrint.hpp"

RingBuffer<ThreadJob*, 5> saveQueue;

//-------------------------------------------------------------------------------------------------
void ThreadJob::saveImageThreadFnc() {
   // FPrint::printInfo(img, name);
   if (ImageUtilF::saveTo(img, name)) {
       std::cout << "Thread - saved " << name << std::endl;
   } else {
       std::cerr << "Thread - save FAILED " << name << std::endl;
   }
   img.Close();
}

//-------------------------------------------------------------------------------------------------
bool ThreadJob::StartThread( FImage& img, const char* toName, ImageAux* aux) {
    ThreadJob* saveAuxPtr;
    if (saveQueue.Full()) {
        saveQueue.Get(saveAuxPtr);
        // std::cerr << "Save Queue Full - join thread " << saveAuxPtr->name << std::endl;
        saveAuxPtr->thread1.join();
        delete saveAuxPtr;
    }
    
    saveAuxPtr = new ThreadJob(img, toName, (aux!=nullptr) ? aux->verbose : false);
    return saveQueue.Put(saveAuxPtr);
}

//-------------------------------------------------------------------------------------------------
void ThreadJob::EndThreads() {
    ThreadJob* saveAuxPtr;
    while (!saveQueue.Empty()) {
        saveQueue.Get(saveAuxPtr);
        saveAuxPtr->thread1.join();
        delete saveAuxPtr;
    }
}
#endif

