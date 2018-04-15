#ifndef TIMER_H
#define TIMER_H

typedef struct Timer Timer;
/**
 * Creates a new timer
 *
 * @param ticks Current number of ticks
 * @return The timer on success, NULL on error
 */
Timer *timerNew(unsigned int ticks);

/**
 * Destroys the timer
 *
 * @param t The timer to destroy
 */
void timerDelete(Timer *t);

/**
 * Updates the timer
 *
 * @param t The timer to update
 * @param ticks The current number of ticks
 * @return ticks difference since last update or -1 on error
 */
int timerUpdate(Timer *t, unsigned int ticks);

#endif // TIMER_H

