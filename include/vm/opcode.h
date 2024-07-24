#ifndef _OPCODE_H_
#define _OPCODE_H_

typedef enum _opcode_
{
    NIL_OPC,
    BCONST_OPC,
    ICONST_OPC,
    SCONST_OPC,
    OARR_OPC, // creates an object array

    ARR_LEN_OPC,  // gets the length of an array
    ARR_ITM_OPC,  // gets a item from a array
    ARR_SITM_OPC, // sets a value to a array item

    LREAD_OPC, // get a local variable (push to stack)
    LSET_OPC,  // set a value to a local variable

    GWRITE_OPC, // creates a global variable
    GREAD_OPC,  // gets a global variable

    LOAD_OPC,

    // arithmetic
    ADD_OPC,
    SUB_OPC,
    MUL_OPC,
    DIV_OPC,
    MOD_OPC,

    // comparison
    LT_OPC, // less
    GT_OPC, // greater
    LE_OPC, // less equals
    GE_OPC, // greater equals
    EQ_OPC, // equals
    NE_OPC, // not equals

    // control flow
    JMP_OPC, // unconditional jump
    JIT_OPC, // jump if true
    JIF_OPC, // jump if false

    // logical
    OR_OPC,
    AND_OPC,
    NOT_OPC,
    NNOT_OPC,

    CONCAT_OPC,  // join two strings
    STR_LEN_OPC, // length of a string
    STR_ITM_OPC, // gets string char

    CLASS_OPC,
    THIS_OPC,
    SET_PROPERTY_OPC,
    GET_PROPERTY_OPC,

    IS_OPC,
    FROM_OPC,

    // others
    PRT_OPC,  // prints a value from the stack
    POP_OPC,  // pops a value from the stack
    CALL_OPC, // calls a function
    GBG_OPC,  // Asks he vm to garbage objects
    RET_OPC,
    HLT_OPC
} Opcode;

#endif