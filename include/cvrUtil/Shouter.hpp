/*
 * Shouter.h
 *
 *  Created on: Apr 16, 2012
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef SHOUTER_H_
#define SHOUTER_H_

#include <assert.h>
#include <set>

template <typename T>
class Listener;

template <typename T>
class Shouter
{
public:
    typedef std::set< Listener<T>* > Listeners;

    void AddListener(Listener<T>* listener);
    void RemoveListener(Listener<T>* listener);
    void Shout(Listener<T>* ignoreListener = NULL);
    void ShoutAt(Listener<T>* target);

protected:
    virtual ~Shouter() {}

private:
    Listeners mToAdd;
    Listeners mToRemove;
    Listeners mListeners;
};

#include "Listener.h"

template <typename T>
void
Shouter<T>::AddListener(Listener<T>* listener)
{
    assert(listener != NULL);
    mToRemove.erase(listener);
    mToAdd.insert(listener);
}

template <typename T>
void
Shouter<T>::RemoveListener(Listener<T>* listener)
{
    assert(listener != NULL);
    mToAdd.erase(listener);
    mToRemove.insert(listener);
}

template <typename T>
void
Shouter<T>::Shout(Listener<T>* ignoreListener /*= NULL*/)
{
    typename Listeners::iterator current;

    // Update mListeners
    // Note: Should be less overhead to iterate over mToRemove instead of using set_difference, assuming removals are rare
    for (current = mToRemove.begin(); current != mToRemove.end(); ++current)
        mListeners.erase( *current );
    mToRemove.clear();

    mListeners.insert(mToAdd.begin(), mToAdd.end());
    mToAdd.clear();

    // Iterate over and ShoutAt mListeners
    for (current = mListeners.begin(); current != mListeners.end(); ++current)
        if (*current != ignoreListener)
            ShoutAt( *current );
}

template <typename T>
void
Shouter<T>::ShoutAt(Listener<T>* target)
{
    assert(target != NULL);
    target->Hear(static_cast<T*>(this));
}

#endif /*SHOUTER_H_*/

