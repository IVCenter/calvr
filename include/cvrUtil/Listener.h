/*
 * Listener.h
 *
 *  Created on: Apr 16, 2012
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef LISTENER_H_
#define LISTENER_H_

#include "Shouter.hpp"

template <typename T>
class Listener
{
    friend void
    Shouter<T>::Shout(Listener<T>* ignoreListener);

    friend void
    Shouter<T>::ShoutAt(Listener<T>* target);

protected:
    virtual void
    Hear(T* shouter) = 0;
};

#endif /*LISTENER_H_*/

