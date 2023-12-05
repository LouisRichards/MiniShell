/* this file was taken from https://jera.com/techinfo/jtns/jtn002 */

/* You may use the code in this tech note for any purpose, 
 with the understanding that it comes with NO WARRANTY. */

 #define mu_assert(message, test) do { if (!(test)) return message; } while (0)
 #define mu_run_test(test) do { printf("### Running test %d... ###\n", tests_run); char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
 extern int tests_run;
