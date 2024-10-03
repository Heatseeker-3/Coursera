/* Routines for timing functions */

/*  minimum resolution of timer (secs) */
void start_counter();

/* Timer: measures in seconds */

/* Start the timer */
double get_counter();

/* Get # seconds since timer started.  Returns 1e20 if detect timing anomaly */
double ovhd();

/* Determine clock rate of processor (using a default sleeptime) */
double mhz(int verbose);

/* Determine clock rate of processor (using a default sleeptime) */
double mhz_full(int verbose, int sleeptime);

/* Get # cycles since counter started.  Returns 1e20 if detect timing anomaly */

void start_comp_counter();
double get_comp_counter();

