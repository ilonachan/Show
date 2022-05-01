#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>

#include "valstream.h"
#include "shader.h"

ValBindingRef *valstream_globalValBindings = NULL;
ValBindingRef *valstream_globalValBindings_tail = NULL;

void valstream_load_bindings() {
    // TODO: load the list of bindings from config instead
	ValBinding new_binding = {.name="test", .type=VBT_FLOAT, .source=VBS_COMMAND, .command="1234"};
	ValBinding new_binding2 = {.name="test2", .type=VBT_FLOAT, .source=VBS_COMMAND, .command="617"};
	valstream_add_element(new_binding);
	valstream_add_element(new_binding2);
}

void valstream_add_element(ValBinding vb) {
    ValBindingRef *newNode = malloc(sizeof(ValBindingRef));
    newNode->src = malloc(sizeof(ValBinding));
    memcpy(newNode->src, &vb, sizeof(ValBinding));
    if(valstream_globalValBindings_tail == NULL) {
        valstream_globalValBindings = newNode;
    } else {
        valstream_globalValBindings_tail->next = newNode;
    }
    valstream_globalValBindings_tail = newNode;
}

/* copy the global list of value bindings, getting the  */
ValBindingRef* valstream_bind_to_shader(Shader shader) {
    ValBindingRef *head = valstream_globalValBindings;
    ValBindingRef *ret = NULL;
    // I hate having to do special handling for the beginning of a list, so I'll use double pointer magic instead
    // **counter  is the new list node (malloc'd)
    //  *counter  is a reference to the new list node
    //   counter  points to where that reference will be stored (initially the head pointer, later a next pointer)
    ValBindingRef **cursor = &ret;
    while(head != NULL) {
        *cursor = (ValBindingRef*) malloc(sizeof(struct ValBindingRef));
        (*cursor)->src = head->src;
        (*cursor)->loc = glGetUniformLocation(shader, head->src->name);
        (*cursor)->next = NULL;
        cursor = &((*cursor)->next);
        head = head->next;
    }
    // because this is my first time writing C code, I am scared of passing malloc'd data out of a function. Hopefully nobody forgets to delete this lmao.
    return ret;
}

void valstream_rebind_shader(Shader shader, ValBindingRef *head) {
    while(head != NULL) {
        head->loc = glGetUniformLocation(shader, head->src->name);
        head = head->next;
    }
}


union ValBindingValue valstream_str2value(char* str, enum ValBindingType type) {
    union ValBindingValue val;
    switch (type) {
    case VBT_FLOAT:
        val.f = atof(str); break;
    case VBT_DOUBLE:
        val.d = atof(str); break;
    case VBT_INT:
        val.i = atoi(str); break;
    case VBT_UINT:
        val.u = atoi(str); break;
    default:
        printf("undefined value type %d", type); break;
    }
    return val;
}

union ValBindingValue valstream_call(char* command, enum ValBindingType type) {
    // TODO: actually call the program
    char* str = command;

    return valstream_str2value(str, type);
}


/*  call this every frame or something ig, queries the specified channels to obtain current values
    TODO: large parts of this should probably be done asynchronously, so the drawing does not need to block for delayed data */
void valstream_update_values() {
    ValBindingRef *head = valstream_globalValBindings;
    while(head != NULL) {
        union ValBindingValue value;
        switch (head->src->source) {
        case VBS_COMMAND:
            value = valstream_call(head->src->command, head->src->type);
            break;
        case VBS_FILE: // TODO: read latest value from a file stream
            // these two operations are actually very much related, since the result of a command execution
            // is also usually accessible through stdout.
            break;
        case VBS_DUMMY: // for testing purposes
            value = head->src->data;
            break;
        default:
            break;
        }
        head->src->data = value;
        head = head->next;
    }
}

/* With the specified shader program active, injects the previously obtained values into this frame's shader execution */
void valstream_inject_into_shader(Shader shader, ValBindingRef *head) {
    while(head != NULL) {
        switch (head->src->type) {
        case VBT_FLOAT:
            glUniform1f(head->loc, head->src->data.f); break;
        case VBT_DOUBLE:
            glUniform1d(head->loc, head->src->data.d); break;
        case VBT_INT:
            glUniform1i(head->loc, head->src->data.i); break;
        case VBT_UINT:
            glUniform1ui(head->loc, head->src->data.u); break;
        default:
            printf("undefined value type %d", head->src->type);
            break;
        }
        head = head->next;
    }
}
