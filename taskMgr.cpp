#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdio.h" // For printf

char eTaskStateGetStateChar( eTaskState eState );

void vApplicationIdleHook( void )
{
    // This hook is called when the RTOS is in the idle state.
    // It can be used for low-priority background processing.
}

void vApplicationTickHook( void )
{
    // This hook is called on each RTOS tick.
}

void vPrintTaskStats( void *pvParameters )
{
    (void) pvParameters; // Suppress unused parameter warning

    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxArraySize, x;
    unsigned long ulTotalRunTime;

    // Allocate an array to hold the task status structures.
    // The size is determined by the number of tasks in the system.
    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

    if( pxTaskStatusArray != NULL )
    {
        // Get the system state and individual task information.
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

        // Print header for task information
        printf( "\nTask Name\tState\tPrio\tStack\t    HWM        Time %%\n" );

        // Iterate through the array and print information for each task.
        for( x = 0; x < uxArraySize; x++ )
        {
            printf( "%15s\t  %c\t  %u\t%u\t%9lu\t%lu%%\n",
                    pxTaskStatusArray[ x ].pcTaskName,
                    (char)eTaskStateGetStateChar( pxTaskStatusArray[ x ].eCurrentState ),
                    (unsigned int)pxTaskStatusArray[ x ].uxCurrentPriority,
                    (unsigned int)pxTaskStatusArray[ x ].usStackHighWaterMark,
                    pxTaskStatusArray[ x ].ulRunTimeCounter,
                    ( ulTotalRunTime > 0 ) ? ( pxTaskStatusArray[ x ].ulRunTimeCounter * 100UL ) / ulTotalRunTime 
                    					   : 0
                  );
        }

        // Free the allocated memory.
        vPortFree( pxTaskStatusArray );
    }
}

// Helper function to get character representation of task state
char eTaskStateGetStateChar( eTaskState eState )
{
    switch ( eState )
    {
        case eRunning:      return 'R';
        case eReady:        return 'r';
        case eBlocked:      return 'B';
        case eSuspended:    return 'S';
        case eDeleted:      return 'D';
        default:            return '?';
    }
}

