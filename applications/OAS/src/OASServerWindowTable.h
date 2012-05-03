/**
 * @file OASServerWindowTable
 * @author Shreenidhi Chowkwale
 *
 */

#ifndef _OAS_SERVER_WINDOW_TABLE_H_
#define _OAS_SERVER_WINDOW_TABLE_H_

#include <FL/Fl.H>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>
#include <pthread.h>
#include <map>
#include <queue>
#include "OASAudioUnit.h"

namespace oas
{
// Types for managing the state of audio units

typedef std::map<unsigned int, const AudioUnit*>    AudioUnitMap;
typedef AudioUnitMap::iterator                      AudioUnitMapIterator;
typedef AudioUnitMap::const_iterator                AudioUnitMapConstIterator;
typedef std::pair<unsigned int, const AudioUnit*>   AudioUnitPair;

class ServerWindowTable : public Fl_Table 
{
public:

    /**
     * Constructor for table creation.
     * @param X The initial X position of the table
     * @param Y The initial Y position of the table
     * @param W The width of the table
     * @param H The height of the table
     * @param L A string for the label
     */
    ServerWindowTable(int X, int Y, int W, int H, const char *L = NULL, bool tableIsForSoundSources = true);

    /**
     * Notifies the ServerWindowTable that this particular audio unit has been modified.
     * It will then be queued up for processing, by a different thread.
     */
    void audioUnitWasModified(const AudioUnit* audioUnit);

    /**
     * Notifies the ServerWindowTable that these audio units have been modified.
     * They will be then queued up for processing by a different thread.
     */
    void audioUnitsWereModified(std::queue<const AudioUnit*> &audioUnits);

    /**
     * Clears the audio units that are in the processing queue, by making the appropriate
     * adjustments to the internal map of audio units.
     */
    void update();

    /**
     * Resets the table for a new session
     */
    void reset();

private:

    /**
     * @brief Takes the audio units in the queue and updates the entries in the map accordingly
     * @param queue Queue containing audio units that were modified
     */
    void _updateAudioUnitMap(std::queue<const AudioUnit*> &queue);

    /**
     * @brief Handles drawing each individual cell in the table, overriding Fl_Table's method.
     * Fl_Table calls this function to draw each visible cell in the table.
     * We use FLTK's drawing functions to draw the cells the way we want.
     */
    void draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H);

    /**
     * @brief Draws a table header cell
     */
    void _drawHeader(const char *s, int X, int Y, int W, int H);

    /**
     * @brief Draws a data cell
     */
    void _drawData(const char *s, int X, int Y, int W, int H);

    const char* _getColumnHeaderForAudioUnit(int column);
    const char* _getRowHeaderForAudioUnit(int row, const AudioUnit*);
    void _writeCellContentsForAudioUnit(const AudioUnit *audioUnit, int column, char *buffer);

    /*
     * Audio Units are:
     *   added to this queue via "audioUnitWasModified()", on thread A
     *   removed from this queue via "processModifiedAudioUnits()", on thread B
     */
    std::queue<const AudioUnit*> _audioUnitProcessingQueue;
    pthread_mutex_t _queueMutex;
    pthread_cond_t _queueCondition;

    /*
     * Audio Units are:
     *   * added, removed, and replaced in the map by processModifiedAudioUnits(), on thread B
     *   * read from the map by the _drawData() method, on thread C (owned by fltk)
     */
    AudioUnitMap _audioUnitMap;
    bool _audioUnitMapModified;
    pthread_mutex_t _mapMutex;
};

}

#endif
