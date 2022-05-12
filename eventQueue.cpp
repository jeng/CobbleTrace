#include "eventQueue.h"

//TODO this will need to do some locking more than likely
void
AddEvent(eventManager_t *em, event_t event){
    if (em->writeIndex >= em->capacity){
        em->writeIndex = 0;
    }
    em->queue[em->writeIndex] = event;
    em->writeIndex++;
    em->active++;
} 

event_t*
ReadEvent(eventManager_t *em){
    if (em->active > 0){
        if (em->readIndex >= em->capacity){
            em->readIndex = 0;
        }
        em->active--;
        return &em->queue[em->readIndex++];
    } 
    return NULL;
}


event_t*
PeekEvent(eventManager_t *em){
    if (em->active > 0){
        if (em->readIndex >= em->capacity){
            em->readIndex = 0;
        }
        return &em->queue[em->readIndex];
    } 
    return NULL;
}
