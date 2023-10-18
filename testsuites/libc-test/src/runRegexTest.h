#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int regex_backref_test();
extern int regex_bracket_icase();
extern int regex_ere_backref();
extern int regex_negated_range();
extern int regexec_nosub();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    regex_backref_test,
    regex_bracket_icase,
    regex_ere_backref,
    regex_negated_range,
    regexec_nosub
};

char run_test_name_1[][50] = {
    "regex_backref_test",
    "regex_bracket_icase",
    "regex_ere_backref",
    "regex_negated_range",
    "regexec_nosub",
};

#endif