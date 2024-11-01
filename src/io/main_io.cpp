#include "main_io.h"

#include "lib/logger/logger.h"

static const char OWL_TEXT[] = R"""(You let the owls out!
    A_,,,_A        A_,,,_A        A_,,,_A    
   ((O)V(O))      ((O)V(O))      ((O)V(O))   
  (w\W|W|W/w)    (w\W|W|W/w)    (w\W|W|W/w)  
   \v|W|W|v/      \v|W|W|v/      \v|W|W|v/   
     "|v|"          "|v|"          "|v|"     
      ^ ^            ^ ^            ^ ^      )""";

error_t parse(int key, char* arg, struct argp_state* state) {
    Options* options = (Options*)state->input;

    switch (key) {
        case OPT_OWL:
            puts(OWL_TEXT);
            break;
        case 's':
            options->enable_server();
            break;
        case ARGP_KEY_ARG:
        default:
            break;
    }

    return 0;
}
