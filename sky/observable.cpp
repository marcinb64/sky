#include "observable.h"

class NullOwner : public Owner
{
    virtual void stopObserving(ObserverBase &) {}
};

static NullOwner __n;

Owner &nullOwner = __n;





