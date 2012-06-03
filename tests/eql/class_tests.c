#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>
#include <eql/parser.h>

#include "../minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring Foo = bsStatic("Foo");
struct tagbstring Bar = bsStatic("Bar");

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");
struct tagbstring bat = bsStatic("bat");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_class_create() {
    eql_ast_node *methods[1];
    eql_ast_node *properties[2];
    eql_ast_node *node, *method1, *property1, *property2;
    eql_ast_method_create(EQL_ACCESS_PUBLIC, NULL, &method1);
    methods[0] = method1;
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property1);
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property2);
    properties[0] = property1;
    properties[1] = property2;
    eql_ast_class_create(&Foo, methods, 1, properties, 2, &node);
    
    mu_assert(node->type == EQL_AST_TYPE_CLASS, "");
    mu_assert(biseqcstr(node->class.name, "Foo"), "");
    mu_assert(node->class.method_count == 1, "");
    mu_assert(node->class.methods[0] == method1, "");
    mu_assert(node->class.property_count == 2, "");
    mu_assert(node->class.properties[0] == property1, "");
    mu_assert(node->class.properties[1] == property2, "");
    eql_ast_node_free(node);
    return 0;
}

int test_eql_ast_class_add_property() {
    eql_ast_node *node, *property1, *property2;
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property1);
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property2);
    eql_ast_class_create(&Foo, NULL, 0, NULL, 0, &node);
    eql_ast_class_add_property(node, property1);
    eql_ast_class_add_property(node, property2);
    mu_assert(node->class.property_count == 2, "");
    mu_assert(node->class.properties[0] == property1, "");
    mu_assert(node->class.properties[1] == property2, "");
    eql_ast_node_free(node);
    return 0;
}

int test_eql_ast_class_add_method() {
    eql_ast_node *node, *method1, *method2;
    eql_ast_method_create(EQL_ACCESS_PUBLIC, NULL, &method1);
    eql_ast_method_create(EQL_ACCESS_PUBLIC, NULL, &method2);
    eql_ast_class_create(&Foo, NULL, 0, NULL, 0, &node);
    eql_ast_class_add_method(node, method1);
    eql_ast_class_add_method(node, method2);
    mu_assert(node->class.method_count == 2, "");
    mu_assert(node->class.methods[0] == method1, "");
    mu_assert(node->class.methods[1] == method2, "");
    eql_ast_node_free(node);
    return 0;
}

int test_eql_ast_class_add_members() {
    eql_ast_node *members[3];
    eql_ast_node *node, *method1, *property1, *property2;
    eql_ast_method_create(EQL_ACCESS_PUBLIC, NULL, &method1);
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property1);
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property2);
    members[0] = method1;
    members[1] = property1;
    members[2] = property2;
    eql_ast_class_create(&Foo, NULL, 0, NULL, 0, &node);
    eql_ast_class_add_members(node, members, 3);
    mu_assert(node->class.method_count == 1, "");
    mu_assert(node->class.methods[0] == method1, "");
    mu_assert(node->class.property_count == 2, "");
    mu_assert(node->class.properties[0] == property1, "");
    mu_assert(node->class.properties[1] == property2, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_class() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("class Foo { }");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.classes[0];
    mu_assert(node->type == EQL_AST_TYPE_CLASS, "");
    mu_assert(biseqcstr(node->class.name, "Foo"), "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_class_create);
    mu_run_test(test_eql_ast_class_add_property);
    mu_run_test(test_eql_ast_class_add_method);
    mu_run_test(test_eql_ast_class_add_members);
    mu_run_test(test_eql_parse_class);
    return 0;
}

RUN_TESTS()