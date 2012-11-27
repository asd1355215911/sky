#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <cursor.h>
#include <data_descriptor.h>
#include <sky_string.h>
#include <dbg.h>
#include <mem.h>

#include "../minunit.h"

//==============================================================================
//
// Declarations
//
//==============================================================================

#define ASSERT_OBJ_STATE(OBJ, TIMESTAMP, ACTION_ID, OSTRING, OINT, ODOUBLE, OBOOLEAN, ASTRING, AINT, ADOUBLE, ABOOLEAN) do {\
    mu_assert_int64_equals(OBJ.timestamp, TIMESTAMP); \
    mu_assert_int_equals(OBJ.action_id, ACTION_ID); \
    mu_assert_int_equals(OBJ.object_string.length, (int)strlen(OSTRING)); \
    mu_assert_bool(memcmp(OBJ.object_string.data, OSTRING, strlen(OSTRING)) == 0); \
    mu_assert_int64_equals(OBJ.object_int, OINT); \
    mu_assert_bool(fabs(OBJ.object_double-ODOUBLE) < 0.1); \
    mu_assert_bool(OBJ.object_boolean == OBOOLEAN); \
    mu_assert_int_equals(OBJ.action_string.length, (int)strlen(ASTRING)); \
    mu_assert_bool(memcmp(OBJ.action_string.data, ASTRING, strlen(ASTRING)) == 0); \
    mu_assert_int64_equals(OBJ.action_int, AINT); \
    mu_assert_bool(fabs(OBJ.action_double-ADOUBLE) < 0.1); \
    mu_assert_bool(OBJ.action_boolean == ABOOLEAN); \
} while(0)

typedef struct {
    sky_timestamp_t timestamp;
    sky_action_id_t action_id;
    sky_string action_string;
    int64_t    action_int;
    double     action_double;
    bool       action_boolean;
    sky_string object_string;
    int64_t    object_int;
    double     object_double;
    bool       object_boolean;
} test_t;


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Iteration
//--------------------------------------

int test_sky_cursor_next() {
    char data[1024]; memset(data, 0, 1024);
    FILE *file = fopen("tests/fixtures/cursors/0/data", "r");
    fread(data, 1, 1024, file);
    fclose(file);

    sky_cursor *cursor = sky_cursor_create();

    // Event 1
    int rc = sky_cursor_set_path(cursor, data);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 0);
    mu_assert_long_equals(cursor->ptr-((void*)data), 8L);
    mu_assert_bool(!cursor->eof);
    
    // Event 2
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 1);
    mu_assert_long_equals(cursor->ptr-((void*)data), 19L);
    mu_assert_bool(!cursor->eof);

    // Event 3
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 2);
    mu_assert_long_equals(cursor->ptr-((void*)data), 37L);
    mu_assert_bool(!cursor->eof);
    
    // EOF
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 0);
    mu_assert_bool(cursor->eof);

    sky_cursor_free(cursor);
    return 0;
}


//--------------------------------------
// Set Data
//--------------------------------------

int test_sky_cursor_set_data() {
    importtmp("tests/fixtures/cursors/0/import.json");
    char data[1024]; memset(data, 0, 1024);
    FILE *file = fopen("tmp/0/data", "r");
    fread(data, 1, 1024, file);
    fclose(file);
    
    // Setup data object & data descriptor.
    test_t obj; memset(&obj, 0, sizeof(obj));
    sky_data_descriptor *descriptor = sky_data_descriptor_create();
    descriptor->timestamp_descriptor.offset = offsetof(test_t, timestamp);
    descriptor->action_descriptor.offset = offsetof(test_t, action_id);
    sky_data_descriptor_set_property(descriptor, -4, offsetof(test_t, action_boolean), SKY_DATA_TYPE_BOOLEAN);
    sky_data_descriptor_set_property(descriptor, -3, offsetof(test_t, action_double), SKY_DATA_TYPE_DOUBLE);
    sky_data_descriptor_set_property(descriptor, -2, offsetof(test_t, action_int), SKY_DATA_TYPE_INT);
    sky_data_descriptor_set_property(descriptor, -1, offsetof(test_t, action_string), SKY_DATA_TYPE_STRING);
    sky_data_descriptor_set_property(descriptor, 1, offsetof(test_t, object_string), SKY_DATA_TYPE_STRING);
    sky_data_descriptor_set_property(descriptor, 2, offsetof(test_t, object_int), SKY_DATA_TYPE_INT);
    sky_data_descriptor_set_property(descriptor, 3, offsetof(test_t, object_double), SKY_DATA_TYPE_DOUBLE);
    sky_data_descriptor_set_property(descriptor, 4, offsetof(test_t, object_boolean), SKY_DATA_TYPE_BOOLEAN);

    sky_cursor *cursor = sky_cursor_create();
    cursor->data_descriptor = descriptor;
    cursor->data = &obj;

    // Event 1 (State-Only)
    sky_cursor_set_path(cursor, data);
    ASSERT_OBJ_STATE(obj, 0LL, 0, "john doe", 1000LL, 100.2, true, "", 0LL, 0, false);
    
    // Event 2 (Action + Action Data)
    sky_cursor_next(cursor);
    ASSERT_OBJ_STATE(obj, 1000000LL, 1, "john doe", 1000LL, 100.2, true, "super", 21LL, 2.5, true);
    
    // Event 3 (Action-Only)
    sky_cursor_next(cursor);
    ASSERT_OBJ_STATE(obj, 2000000LL, 2, "john doe", 1000LL, 100.2, true, "", 0LL, 0, false);

    // Event 4 (Data-Only)
    sky_cursor_next(cursor);
    ASSERT_OBJ_STATE(obj, 3000000LL, 0, "frank sinatra", 20LL, 1.5, false, "", 0LL, 0, false);

    sky_cursor_free(cursor);
    sky_data_descriptor_free(descriptor);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_cursor_next);
    mu_run_test(test_sky_cursor_set_data);
    return 0;
}

RUN_TESTS()