#ifndef CHILOG_H_
#define CHILOG_H_


/* Log levels */
typedef enum {
    CRITICAL = 10,
    ERROR    = 20,
    WARNING  = 30,
    INFO     = 40,
    DEBUG    = 50,
    TRACE    = 60
} loglevel_t;


/*
 * chilog_setloglevel - Sets the logging level
 *
 * When a log level is set, all messages at that level or "worse" are
 * printed. e.g., if you set the log level to WARNING, then all
 * WARNING, ERROR, and CRITICAL messages will be printed.
 *
 * level: Logging level
 *
 * Returns: Nothing.
 */
void chilog_setloglevel(loglevel_t level);


/*
 * chilog - Print a log message
 *
 * level: Logging level of the message
 *
 * fmt: printf-style formatting string
 *
 * ...: Extra parameters if needed by fmt
 *
 * Returns: nothing.
 */
#define chilog(level, fmt, ...) __chilog(level, __FILE__,  __LINE__, fmt, ##__VA_ARGS__)
void __chilog(loglevel_t level, char *file, int line, char *fmt, ...);

/*
 * chilog_hex - Print arbitrary data in hexdump style
 *
 * level: Logging level
 *
 * data: Pointer to the data
 *
 * len: Number of bytes to print
 *
 * Returns: nothing.
 */
#define chilog_hex(level, data, len) __chilog_hex(level, __FILE__,  __LINE__, data, len)
void __chilog_hex (loglevel_t level, char *file, int fline, void *data, int len);


#endif /* CHILOG_H_ */
