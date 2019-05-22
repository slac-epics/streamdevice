#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <new>
#include <cstdlib>

#include "messageEngine.h"

StreamErrorEngine* createEngine(pthread_t thread_id)
{
    StreamErrorEngine* newEngine = new StreamErrorEngine;
    pthread_create(&thread_id, NULL, engineJob, 
                   reinterpret_cast<void *>(newEngine));
    return newEngine;
}

void *engineJob(void* engineObj) 
{
    int iii; // loop index
    int elapsedTime;

    StreamErrorEngine* engine = reinterpret_cast<StreamErrorEngine*>(engineObj);
    errorData_t* errData;
    
    while(1) 
    {   
        if (engine->mustStop())
        {
            // Time to finish the thread
            pthread_exit(0);
        }

        // Loop through all error categories
        for(iii=0; iii<TOTAL_CATEGORIES; ++iii) 
        {
            errData = engine->errorDataFrom(static_cast<ErrorCategory>(iii));
 
            if (! errData)
            {
                continue;
            }

            // Lock access to error data
            pthread_mutex_lock(&errData->mutex);
            
            // Timeout?
            elapsedTime = difftime(time(NULL),
                                   errData->lastPrintTime);
            
            if (elapsedTime >= engine->getTimeout()) 
            {
                if (errData->numberOfCalls != 0) 
                {
                    // Print error message
                    fprintf(stderr, "Stream Device: this message was received %d times"
                            " in the last %d seconds:\n", 
                            errData->numberOfCalls, 
                            elapsedTime);
                    fprintf(stderr,
                            errData->lastErrorMessage);
                    // Restart number of calls
                    errData->numberOfCalls = 0;
                    // Reset last message time
                    errData->lastPrintTime = time(NULL);
                } // if # of calls != 0
                else
                {
                    errData->waitingTimeout = false;
                }
            } // if timeout

            // Release lock
            pthread_mutex_unlock(&errData->mutex);

        } // for

        sleep(ENGINE_POLLING_TIME);
    } // while(1)

    return 0;
}

StreamErrorEngine::StreamErrorEngine()
{
    stopThread = false;

    for (int iii=0; iii<TOTAL_CATEGORIES; ++iii)
    {
        categories[iii].waitingTimeout = 0;
        categories[iii].lastPrintTime = time(NULL);
        categories[iii].numberOfCalls = 0;
        strcpy(categories[iii].lastErrorMessage, "\0");
        pthread_mutex_init(&categories[iii].mutex, NULL);
    }
}

StreamErrorEngine::~StreamErrorEngine()
{
    // Order message engine thread to stop
    stopThread = true;

    // Wait for thread to stop
    pthread_join(thread_id, NULL);

    // Cleanup
    for (int iii=0; iii<TOTAL_CATEGORIES; ++iii)
    {
        pthread_mutex_destroy(&categories[iii].mutex);
    }
}

void StreamErrorEngine::callError(ErrorCategory category, char* message)
{
    if (category < 0 || category > TOTAL_CATEGORIES)
    {
        return;
    }
    
    // Using errData variable is easier to read than categories[category] 
    errorData_t* errData = &categories[category];
    
    // Lock access to error data
    pthread_mutex_lock(&errData->mutex);

    // If it is waiting timeout, just increment the number of calls for that
    // error category.
    if (errData->waitingTimeout)
    {
        errData->numberOfCalls++;
    }
    else
    // It is not waiting for timeout, what means that this is the first message
    // since a long time ago.
    {
        // Print message right away
        fprintf(stderr, message);
        // Signal that now we are waiting for a timeout before printing another
        // message
        errData->waitingTimeout = true;
        // Remember the error message to use later
        strcpy(errData->lastErrorMessage, message);
        // Starting time to detect a future timeout
        errData->lastPrintTime = time(NULL);
        // Restart message counter
        errData->numberOfCalls = 0;
    }
    
    // Release lock
    pthread_mutex_unlock(&errData->mutex);
}

void StreamErrorEngine::setTimeout(int tmout)
{
    timeout = tmout;
}

int StreamErrorEngine::getTimeout()
{
    return timeout;
}

errorData_t* StreamErrorEngine::errorDataFrom(ErrorCategory category)
{
    if (category >= 0 && category <= TOTAL_CATEGORIES)
    {
        return &categories[category];
    }
    else
    {
        return NULL;
    }
}

bool StreamErrorEngine::mustStop()
{
    return stopThread;
}
