#ifndef IIDXHOOK_LOG_SERVER_H
#define IIDXHOOK_LOG_SERVER_H

/**
 * Initialize the log server which creates a dedicated thread and a log message queue.
 * 
 * Log messages are posted to the queue by the application and the background thread writes them to the target output.
 * Required for newer iidx and pop'n games using the avs log api (e.g. iidxhook4 to 7) due to a quirk of the ezusb
 * driver. For a full breakdown about the logging on avs (and non avs) based konami games, 
 * see doc/logging-breadkdown-avs.md.
 */
void log_server_init(void);

/**
 * Shut down the log server.
 */
void log_server_fini(void);

#endif
