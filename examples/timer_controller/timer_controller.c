#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include "logging.h"
#include "timer_controller.h"

static void generalTimerCallback( TimerHandle_t xTimer )
{
    TimerHandler_t *pTimerHandler = (TimerHandler_t *) pvTimerGetTimerID( xTimer );

    if( pTimerHandler == NULL )
    {
        LogWarn( ( "Unexpected behavior, timer expires with NULL data pointer" ) );
    }
    else
    {
        pTimerHandler->onTimerExpire( pTimerHandler->pUserContext );
    }
}

TimerControllerResult_t TimerController_Create( TimerHandler_t *pTimerHandler, const char *pTimerName, uint32_t initialTimeMs, uint32_t repeatTimeMs, TimerControllerTimerExpireCallback onTimerExpire, void *pUserContext )
{
    TimerControllerResult_t ret = TIMER_CONTROLLER_RESULT_OK;
    UBaseType_t uxAutoReload = repeatTimeMs == 0? pdFALSE : pdTRUE;

    if( pTimerHandler == NULL || onTimerExpire == NULL || pTimerName == NULL )
    {
        LogError( ("Invalid input parameters, pTimerHandler=%p, onTimerExpire=%p, pTimerName=%p", pTimerHandler, onTimerExpire, pTimerName) );
        ret = TIMER_CONTROLLER_RESULT_BAD_PARAMETER;
    }

    if( ret == TIMER_CONTROLLER_RESULT_OK )
    {
        // Set timer handler
        pTimerHandler->onTimerExpire = onTimerExpire;
        pTimerHandler->pUserContext = pUserContext;

        pTimerHandler->timer = xTimerCreate( pTimerName,
                                             pdMS_TO_TICKS( initialTimeMs ),
                                             uxAutoReload,
                                             ( void * ) pTimerHandler,
                                             generalTimerCallback );

        // Create the timer
        if( pTimerHandler->timer == NULL )
        {
            LogError( ( "Fail to create timer %s", pTimerName ) );
            ret = TIMER_CONTROLLER_RESULT_FAIL_TIMER_CREATE;
        }
    }

    return ret;
}

TimerControllerResult_t TimerController_SetTimer( TimerHandler_t *pTimerHandler, uint32_t initialTimeMs, uint32_t repeatTimeMs )
{
    TimerControllerResult_t ret = TIMER_CONTROLLER_RESULT_OK;

    (void) initialTimeMs;
    (void) repeatTimeMs;

    if( pTimerHandler == NULL )
    {
        ret = TIMER_CONTROLLER_RESULT_BAD_PARAMETER;
    }

    if( ret == TIMER_CONTROLLER_RESULT_OK )
    {
        if( xTimerStart( pTimerHandler->timer, 0 ) != pdPASS )
        {
            LogError( ( "Fail to set timer" ) );
            ret = TIMER_CONTROLLER_RESULT_FAIL_TIMER_SET;
        }
    }

    return ret;
}

void TimerController_ResetTimer( TimerHandler_t *pTimerHandler )
{
    if( pTimerHandler != NULL )
    {
        // Cancel the timer
        if( TimerController_SetTimer( pTimerHandler, 0U, 0U ) != TIMER_CONTROLLER_RESULT_OK )
        {
            LogError( ("Fail to reset time, errno: %s", strerror( errno ) ) );
        }
    }
}

TimerControllerResult_t TimerController_IsTimerSet( TimerHandler_t *pTimerHandler )
{
    TimerControllerResult_t ret = TIMER_CONTROLLER_RESULT_OK;
    BaseType_t retTimer;

    if( pTimerHandler == NULL )
    {
        ret = TIMER_CONTROLLER_RESULT_BAD_PARAMETER;
    }

    if( ret == TIMER_CONTROLLER_RESULT_OK )
    {
        retTimer = xTimerIsTimerActive( pTimerHandler->timer );
        if( retTimer != pdFALSE )
        {
            ret = TIMER_CONTROLLER_RESULT_SET;
        }
        else
        {
            ret = TIMER_CONTROLLER_RESULT_NOT_SET;
        }
    }

    return ret;
}
