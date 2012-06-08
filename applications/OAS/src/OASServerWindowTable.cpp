#include "OASServerWindowTable.h"
#include "OASAudioSource.h"
#include "OASLogger.h"
#include <iostream>

using namespace oas;

ServerWindowTable::ServerWindowTable(int X, int Y, int W, int H, const char *L,
                                     int numColumns)
: Fl_Table(X, Y, W, H, L)
{
    // Set up Rows
    rows(0); // Set the initial number of rows (empty)
    row_header(1); // Enable row headers (along the left)
    row_height_all(20); // Default height of rows
    row_resize(0); // Disable row resizing

    // Set up Columns
    cols(numColumns);

    col_header(1); // Enable column headers (along the top)
    col_width_all(80); // Default width of columns
    col_resize(1); // Enable column resizing

    end(); // End the Fl_Table group

    // Initialize mutexes
    pthread_mutex_init(&_queueMutex, NULL);
    pthread_mutex_init(&_mapMutex, NULL);

    // Initialize condition variable
    pthread_cond_init(&_queueCondition, NULL);

    _audioUnitMapModified = false;
}

void ServerWindowTable::reset()
{
    // Clear out the processing queue first
    pthread_mutex_lock(&_queueMutex);
    while (!_audioUnitProcessingQueue.empty())
        _audioUnitProcessingQueue.pop();
    pthread_mutex_unlock(&_queueMutex);

    // Then clear out the map state
    Fl::lock();

    for (AudioUnitMapConstIterator iterator = _audioUnitMap.begin();
            iterator != _audioUnitMap.end(); iterator++)
    {
        delete iterator->second;
    }

    _audioUnitMap.clear();
    Fl_Table::rows(0);

    Fl::unlock();
}

void ServerWindowTable::audioUnitWasModified(const AudioUnit* audioUnit)
{
    if (!audioUnit)
        return;

    // Lock mutex
    pthread_mutex_lock(&_queueMutex);
    // Push the audio unit onto the queue
    _audioUnitProcessingQueue.push(audioUnit);
    // Use the condition variable to signal that the queue is not empty
    pthread_cond_signal(&_queueCondition);
    // Unlock mutex
    pthread_mutex_unlock(&_queueMutex);
}

void ServerWindowTable::audioUnitsWereModified(std::queue<const AudioUnit*> &audioUnits)
{
    // If the queue is empty, there's nothing to do
    if (audioUnits.empty())
    {
        return;
    }

    // Lock mutex
    pthread_mutex_lock(&_queueMutex);

    // Transfer the queue contents
    while (!audioUnits.empty())
    {
        _audioUnitProcessingQueue.push(audioUnits.front());
        audioUnits.pop();
    }

    // Signal the condition
    pthread_cond_signal(&_queueCondition);
    // Unlock mutex
    pthread_mutex_unlock(&_queueMutex);

}

void ServerWindowTable::update()
{
    /*
     * Strategy:
     * Create a copy of the _audioUnitProcessingQueue, empty this queue, and then perform all
     * update operations on the duplicate that was created. This will minimize the time that
     * the mutex is locked for the processing queue, and implicitly give priority to threads
     * that are actually adding audio units to the processing queue.
     */

    // If the queue at this point is empty, there is nothing to process
    // Obtain a lock on the queue
    pthread_mutex_lock(&_queueMutex);

    // Compute timeout
    struct timeval currTime;
    struct timespec timeout;
    gettimeofday(&currTime, NULL);

    timeout.tv_sec = currTime.tv_sec + ((currTime.tv_usec + 1000) / 1000000);
    timeout.tv_nsec = ((currTime.tv_usec + 1000) % 1000000) * 1000;

    // wait (block) on the condition variable with timeout
    // this effectively waits for the queue to have some content, without spinlocking
    while (_audioUnitProcessingQueue.empty())
    {
        int error = pthread_cond_timedwait(&_queueCondition, &_queueMutex, &timeout);

        // If some error occured (or the wait timed out), return
        if (0 != error)
        {
            pthread_mutex_unlock(&_queueMutex);
            return;
        }
    }

    // Create a temporary queue that is a duplicate of the processing queue
    // The original processing queue is simultaneously emptied out
    std::queue<const AudioUnit*> tempQueue;

    while (!_audioUnitProcessingQueue.empty())
    {
        tempQueue.push(_audioUnitProcessingQueue.front());
        _audioUnitProcessingQueue.pop();
    }

    // Unlock the mutex so other threads can use it
    pthread_mutex_unlock(&_queueMutex);

    // Call _updateAudioUnitMap(), which will handle the actual updating of the internal map state
    _updateAudioUnitMap(tempQueue);
}

void ServerWindowTable::_updateAudioUnitMap(std::queue<const AudioUnit*> &queue)
{
    Fl::lock();

    while (!queue.empty())
    {
        // Grab the first element from the queue, and then pop it off
        const AudioUnit *unit = queue.front();
        queue.pop();

        if (!unit)
            continue;

        // Try to find an audio unit with the same handle in the map
        AudioUnitMapIterator iterator = _audioUnitMap.find(unit->getHandle());

        // If we did not find one, then our job is easy - we insert into the map
        if (iterator == _audioUnitMap.end())
        {
            _audioUnitMap.insert(AudioUnitPair(unit->getHandle(), unit));
            rows(_audioUnitMap.size());
        }
        else
        {
            // Else, we are either replacing or removing the audio unit
            // In either case, we have to delete the old entry first
            delete iterator->second;

            // If the audio unit is a sound source and is marked for deletion,
            // remove the entry from the map altogether
            if (unit->isSoundSource()
                    && (AudioSource::ST_DELETED
                            == (static_cast<const AudioSource *>(unit))->getState()))
            {
                _audioUnitMap.erase(iterator);
                rows(_audioUnitMap.size());
            }
            else
            {
                // assign the value to the new unit
                iterator->second = unit;
            }
        }
    }

    // Turn on the modified indicator
    _audioUnitMapModified = true;

    redraw();
    Fl::unlock();
}

void ServerWindowTable::_drawHeader(const char *s, int X, int Y, int W, int H)
{
    fl_push_clip(X, Y, W, H);
    fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
    fl_pop_clip();
}

void ServerWindowTable::_drawData(const char *s, int X, int Y, int W, int H)
{
    fl_push_clip(X, Y, W, H);
    // Draw cell bg
    fl_color(FL_WHITE);
    fl_rectf(X, Y, W, H);
    // Draw cell data
    fl_color(FL_GRAY0);
    fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color());
    fl_rect(X, Y, W, H);
    fl_pop_clip();
}

void ServerWindowTable::draw_cell(TableContext context, int ROW = 0, int COL = 0, int X = 0,
                                  int Y = 0, int W = 0, int H = 0)
{
#define BUFFER_SIZE 40

    // This static character array is used to store text that fltk will draw in cells
    static char buffer[BUFFER_SIZE];

    // Clear the buffer
    bzero(buffer, BUFFER_SIZE);

    Fl::lock();

    // First, check if the map has contents
    if (_audioUnitMap.empty())
    {
        // If the map is empty, draw nothing
        Fl::unlock();
        return;
    }

    // Second, check if the map has been updated between now and the previous call to draw_cell()
    if (_audioUnitMapModified)
    {
        // Turn off the modified indicator
        _audioUnitMapModified = false;

        // Rebuild the vector
        _audioUnitVector.clear();
        for (AudioUnitMapConstIterator iter = _audioUnitMap.begin();
                iter != _audioUnitMap.end(); iter++)
        {
            _audioUnitVector.push_back(iter->second);
        }
    }

    switch (context)
    {
        case CONTEXT_STARTPAGE: // before page is drawn..
            fl_font(FL_HELVETICA, 16); // set the font for our drawing operations
            break;
        case CONTEXT_COL_HEADER: // Draw column headers
            sprintf(buffer, "%s", _getColumnHeaderForAudioUnit(COL)); // "A", "B", "C", etc.
            this->_drawHeader(buffer, X, Y, W, H);
            break;
        case CONTEXT_ROW_HEADER: // Draw row headers
            sprintf(buffer, "%03d:", _audioUnitVector[ROW]->getHandle()); // "001:", "002:", etc
            this->_drawHeader(buffer, X, Y, W, H);
            break;
        case CONTEXT_CELL: // Draw data in cells
            _writeCellContentsForAudioUnit(_audioUnitVector[ROW], COL, buffer);
            this->_drawData(buffer, X, Y, W, H);
            break;
        default:
            break;
    }

    Fl::unlock();
}

const char* ServerWindowTable::_getColumnHeaderForAudioUnit(int column)
{
    return _audioUnitMap.begin()->second ? _audioUnitMap.begin()->second->getLabelForIndex(column) : "";
}

void ServerWindowTable::_writeCellContentsForAudioUnit(const AudioUnit *audioUnit, int column, char *buffer)
{
    if (!audioUnit || !buffer)
        return;

    sprintf(buffer, "%s", audioUnit->getStringForIndex(column).c_str());
}
