#ifndef showValstreamH
#define showValstreamH

#include <stdio.h>
#include "shader.h"


typedef struct ValBinding {
    /* name of the data source, by which it is identified in the shader */
    char *name;
    /* the type of value supplied to the shader. Currently only numeric values are permitted */
    enum ValBindingType {VBT_UINT, VBT_INT, VBT_FLOAT, VBT_DOUBLE} type;
    /* information about the data source */
    enum ValBindingSource {VBS_FILE, VBS_COMMAND, VBS_DUMMY} source;
    char *command;
    FILE *file;
    union ValBindingValue{
        unsigned int u;
        int i;
        float f;
        double d;
    } data;
} ValBinding;

typedef struct ValBindingRef {
    ValBinding *src;
    /* tied to the shader: reference number for the data source. =0 for the global list */
    int loc;
    /* linked list logic (only need one-way iteration) */
    struct ValBindingRef *next;
} ValBindingRef;

extern ValBindingRef *valstream_globalValBindings;

void valstream_load_bindings();
void valstream_add_element(ValBinding vb);
ValBindingRef* valstream_bind_to_shader(Shader shader);
void valstream_rebind_shader(Shader shader, ValBindingRef *head);
void valstream_update_values();
void valstream_inject_into_shader(Shader shader, ValBindingRef *head);

#endif