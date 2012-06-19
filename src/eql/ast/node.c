#include <stdlib.h>
#include "../../dbg.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Node Lifecycle
//--------------------------------------

// Recursively frees an AST node.
//
// node - The node to free.
void eql_ast_node_free(eql_ast_node *node)
{
    if(!node) return;
    
    // Recursively free dependent data.
    switch(node->type) {
        case EQL_AST_TYPE_INT_LITERAL: break;
        case EQL_AST_TYPE_FLOAT_LITERAL: break;
        case EQL_AST_TYPE_BINARY_EXPR: {
            eql_ast_binary_expr_free(node);
            break;
        }
        case EQL_AST_TYPE_VAR_REF: {
            eql_ast_var_ref_free(node);
            break;
        }
        case EQL_AST_TYPE_VAR_DECL: {
            eql_ast_var_decl_free(node);
            break;
        }
        case EQL_AST_TYPE_VAR_ASSIGN: {
            eql_ast_var_assign_free(node);
            break;
        }
        case EQL_AST_TYPE_FARG: {
            eql_ast_farg_free(node);
            break;
        }
        case EQL_AST_TYPE_FRETURN: {
            eql_ast_freturn_free(node);
            break;
        }
        case EQL_AST_TYPE_FUNCTION: {
            eql_ast_function_free(node);
            break;
        }
        case EQL_AST_TYPE_FCALL: {
            eql_ast_fcall_free(node);
            break;
        }
        case EQL_AST_TYPE_BLOCK: {
            eql_ast_block_free(node);
            break;
        }
        case EQL_AST_TYPE_METHOD: {
            eql_ast_method_free(node);
            break;
        }
        case EQL_AST_TYPE_PROPERTY: {
            eql_ast_property_free(node);
            break;
        }
        case EQL_AST_TYPE_CLASS: {
            eql_ast_class_free(node);
            break;
        }
        case EQL_AST_TYPE_MODULE: {
            eql_ast_module_free(node);
            break;
        }
        case EQL_AST_TYPE_METADATA: {
            eql_ast_metadata_free(node);
            break;
        }
        case EQL_AST_TYPE_METADATA_ITEM: {
            eql_ast_metadata_item_free(node);
            break;
        }
    }
    node->parent = NULL;
    
    free(node);
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for an AST node.
//
// node    - The node to generate LLVM values for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_node_codegen(eql_ast_node *node, eql_module *module,
                         LLVMValueRef *value)
{
    int rc;
    
    check(node != NULL, "Node is required");
    check(module != NULL, "Module is required");
    check(module->llvm_module != NULL, "LLVM Module is required");
    check(module->compiler != NULL, "Module compiler is required");
    check(module->compiler->llvm_builder != NULL, "LLVM Builder is required");

    // Create an intermediate return value so that callers can optionally
    // pass in NULL for the return value.
    LLVMValueRef ret_value = NULL;

    // Delegate codegen to AST nodes.
    switch(node->type) {
        case EQL_AST_TYPE_INT_LITERAL: {
            rc = eql_ast_int_literal_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen literal integer");
            break;
        }
        case EQL_AST_TYPE_FLOAT_LITERAL: {
            rc = eql_ast_float_literal_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen literal float");
            break;
        }
        case EQL_AST_TYPE_BINARY_EXPR: {
            rc = eql_ast_binary_expr_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen binary expression");
            break;
        }
        case EQL_AST_TYPE_VAR_DECL: {
            rc = eql_ast_var_decl_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen variable declaration");
            break;
        }
        case EQL_AST_TYPE_VAR_REF: {
            rc = eql_ast_var_ref_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen variable reference");
            break;
        }
        case EQL_AST_TYPE_VAR_ASSIGN: {
            rc = eql_ast_var_assign_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen variable assignment");
            break;
        }
        case EQL_AST_TYPE_FRETURN: {
            rc = eql_ast_freturn_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen function return");
            break;
        }
        case EQL_AST_TYPE_FARG: {
            rc = eql_ast_farg_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen function argument");
            break;
        }
        case EQL_AST_TYPE_FUNCTION: {
            rc = eql_ast_function_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen function");
            break;
        }
        case EQL_AST_TYPE_FCALL: {
            rc = eql_ast_fcall_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen function call");
            break;
        }
        case EQL_AST_TYPE_BLOCK: {
            rc = eql_ast_block_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen block");
            break;
        }
        case EQL_AST_TYPE_CLASS: {
            rc = eql_ast_class_codegen(node, module);
            check(rc == 0, "Unable to codegen class");
            break;
        }
        case EQL_AST_TYPE_METHOD: {
            rc = eql_ast_method_codegen(node, module, &ret_value);
            check(rc == 0, "Unable to codegen method");
            break;
        }
    }
    
    // If a return value is requested then set it.
    if(value != NULL) {
        *value = ret_value;
    }
    
    return 0;

error:
    if(value != NULL) *value = NULL;
    return -1;
}



//--------------------------------------
// Types
//--------------------------------------

// Recursively determines the type name of a node.
//
// node - The node to determine the type for.
// type - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_node_get_type(eql_ast_node *node, bstring *type)
{
    check(node != NULL, "Node required");

    // Delegate to each type.
    switch(node->type) {
        case EQL_AST_TYPE_INT_LITERAL: {
            eql_ast_int_literal_get_type(node, type);
            break;
        }
        case EQL_AST_TYPE_FLOAT_LITERAL: {
            eql_ast_float_literal_get_type(node, type);
            break;
        }
        case EQL_AST_TYPE_BINARY_EXPR: {
            eql_ast_binary_expr_get_type(node, type);
            break;
        }
        case EQL_AST_TYPE_VAR_REF: {
            eql_ast_var_ref_get_type(node, type);
            break;
        }
        case EQL_AST_TYPE_FCALL: {
            eql_ast_fcall_get_type(node, type);
            break;
        }
        default:
        {
            sentinel("AST node does not have a type");
            break;
        }
    }
    
    return 0;

error:
    *type = NULL;
    return -1;
}


// Performs a non-recursive search for variable declarations of a given name.
//
// node     - The node to search within.
// name     - The name of the variable to search for.
// var_decl - A pointer to where the variable declaration should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_node_get_var_decl(eql_ast_node *node, bstring name,
							 eql_ast_node **var_decl)
{
    check(node != NULL, "Node required");

    // Delegate to each type.
    switch(node->type) {
        case EQL_AST_TYPE_FUNCTION: {
            eql_ast_function_get_var_decl(node, name, var_decl);
            break;
        }
        case EQL_AST_TYPE_BLOCK: {
            eql_ast_block_get_var_decl(node, name, var_decl);
            break;
        }
        default:
        {
			*var_decl = NULL;
            break;
        }
    }
    
    return 0;

error:
    *var_decl = NULL;
    return -1;
}
