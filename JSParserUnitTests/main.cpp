#include <stdio.h>
#include "Tester.h"
#include "JSLookUpTable.h"
#include "JSSourceFile.h"
#include <wx/init.h>
#include <wx/module.h>
#include <wx/filename.h>
#include "fileutils.h"
#include "JSExpressionParser.h"
#include "JSObjectParser.h"
#include "json_node.h"
#include <wx/tokenzr.h>

// Parse class with 2 properties defined using
// this.<name> = ...
TEST_FUNC(test_parse_simple_class)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/simple_class.js"));
    source.Parse();

    JSObject::Ptr_t Foo = lookup->FindClass("Foo");
    CHECK_BOOL(Foo);
    CHECK_SIZE(Foo->GetProperties().size(), 2);
#if 0    
    Foo->Print(0);
#endif
    return true;
}

// Parse class with 3 properties defined outside of the class
TEST_FUNC(class_with_static_method)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/class_with_static_method.js"));
    source.Parse();

    JSObject::Ptr_t Foo = lookup->FindClass("Foo");
    CHECK_BOOL(Foo);
    CHECK_SIZE(Foo->GetProperties().size(), 3);
    return true;
}

// Parse class with 3 properties defined outside of the class
TEST_FUNC(parse_class_properties_assigned_by_object)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/parse_class_properties_assigned_by_object.js"));
    source.Parse();

    JSObject::Ptr_t Foo = lookup->FindClass("Foo");
    CHECK_BOOL(Foo);
    CHECK_SIZE(Foo->GetProperties().size(), 3);
    CHECK_SIZE(Foo->GetProperties().count("name"), 1);
    CHECK_SIZE(Foo->GetProperties().count("foo"), 1);
    CHECK_SIZE(Foo->GetProperties().count("anotherFunc"), 1);
    return true;
}

// Parse global function and check that it exists + check its signature
TEST_FUNC(parse_global_function)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/parse_global_function.js"));
    source.Parse();

    JSObject::Ptr_t Foo = lookup->FindClass("Foo");
    CHECK_BOOL(Foo);
    CHECK_SIZE(Foo->GetProperties().size(), 0);
    CHECK_SIZE(Foo->GetVariables().size(), 2);
    return true;
}

// Parse global function and check that it exists + check its signature
TEST_FUNC(parse_local_variables_in_functions)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/parse_local_variables_in_functions.js"));
    source.Parse();

    // Check that both classes exist
    JSObject::Ptr_t Array = lookup->FindClass("Array");
    CHECK_BOOL(Array);

    JSObject::Ptr_t Foo = lookup->FindClass("Foo");
    CHECK_BOOL(Foo);

    // Check that Foo has 3 local variables
    CHECK_SIZE(Foo->GetVariables().size(), 3);

    // Check the variables names
    const JSObject::Map_t& vars = Foo->GetVariables();
    CHECK_SIZE(vars.count("a"), 1);
    CHECK_SIZE(vars.count("b"), 1);
    CHECK_SIZE(vars.count("c"), 1);

    // Check the variables types
    CHECK_STRING(vars.find("a")->second->GetType(), "Array");
    CHECK_STRING(vars.find("b")->second->GetType(), "Foo");
    CHECK_STRING(vars.find("c")->second->GetType(), "");

    // Check that the variables are not visible in the global scope
    JSObject::Map_t variables = lookup->GetVisibleVariables();
    CHECK_SIZE(variables.size(), 5);

    return true;
}

// Parse local variables declare inside a callback function
TEST_FUNC(parse_locals_in_callback_function)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/parse_locals_in_callback_function.js"));
    source.Parse();

    JSObject::Map_t vars = lookup->GetVisibleVariables();
    CHECK_SIZE(vars.size(), 3);

    CHECK_SIZE(vars.count("event"), 1);
    CHECK_SIZE(vars.count("local_var"), 1);
    CHECK_SIZE(vars.count("element"), 1);
    return true;
}

// Parse local variables declare inside a callback function
TEST_FUNC(parse_locals_self_executed_function)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/parse_local_variables_2.js"));
    source.Parse();

    JSObject::Map_t vars = lookup->GetVisibleVariables();
    // 3 local variables: innerObj, retObj and 'e' (the function argument)
    CHECK_SIZE(vars.size(), 3);
    CHECK_SIZE(vars.count("retObj"), 1);
    CHECK_SIZE(vars.count("innerObj"), 1);
    CHECK_SIZE(vars.count("e"), 1);

    JSObject::Ptr_t retObj = vars.find("retObj")->second;
    JSObject::Ptr_t proto = lookup->FindClass(retObj->GetType());
    CHECK_BOOL(proto);
    CHECK_SIZE(proto->GetProperties().size(), 2);
    return true;
}

// Test parsing function and attaching the return value from the doc comment
TEST_FUNC(parse_function_doc_return_value)
{
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile source(lookup, wxFileName("../TestFiles/parse_function_doc_return_value.js"));
    source.Parse();

    JSObject::Ptr_t Foo = lookup->FindClass("Foo");
    const JSObject::Map_t& variables = Foo->GetVariables();
    CHECK_BOOL(Foo);
    CHECK_SIZE(Foo->GetProperties().size(), 2);
    CHECK_SIZE(variables.size(), 2);
    CHECK_STRING(Foo->GetType(), "Array");
    CHECK_SIZE(variables.count("name"), 1);
    CHECK_SIZE(variables.count("arrOfNames"), 1);

    // Check the function arguments
    CHECK_STRING(variables.find("arrOfNames")->second->GetType(), "Array");
    CHECK_STRING(variables.find("name")->second->GetType(), "string");
    return true;
}

// Test the expression parser
TEST_FUNC(parse_expression)
{
    wxString fileContent;
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    FileUtils::ReadFileContent(wxFileName("../TestFiles/parse_expression.js"), fileContent);
    JSExpressionParser expr(fileContent);
    JSObject::Ptr_t resolved = expr.Resolve(lookup, "../TestFiles/parse_expression.js");
    CHECK_BOOL(resolved);
    // resolved->Print(0);
    CHECK_SIZE(resolved->GetProperties().size(), 3);
    CHECK_STRING(resolved->GetName(), "Array");
    return true;
}

TEST_FUNC(parse_json_object)
{
    wxString fileContent;
    FileUtils::ReadFileContent(wxFileName("../TestFiles/parse_json_object.js"), fileContent);

    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile sourceFile(lookup, fileContent, wxFileName("../TestFiles/parse_json_object.js"));
    JSObjectParser objectParser(sourceFile, lookup);
    CHECK_BOOL(objectParser.Parse(NULL));

    JSObject::Ptr_t o1 = lookup->FindClass("__object1");
    JSObject::Ptr_t o2 = lookup->FindClass("__object2");

    CHECK_BOOL(o1);
    CHECK_BOOL(o2);

    // __object1 should have the following properties:
    // width, height, doc and position
    CHECK_SIZE(o1->GetProperties().count("width"), 1);
    CHECK_SIZE(o1->GetProperties().count("height"), 1);
    CHECK_SIZE(o1->GetProperties().count("doc"), 1);
    CHECK_SIZE(o1->GetProperties().count("position"), 1);

    // __object2: x (Number), y (Number), dims (Array)
    CHECK_SIZE(o2->GetProperties().count("x"), 1);
    CHECK_SIZE(o2->GetProperties().count("y"), 1);
    CHECK_SIZE(o2->GetProperties().count("dims"), 1);

    return true;
}

TEST_FUNC(parse_json_object_2)
{
    wxString fileContent;
    FileUtils::ReadFileContent(wxFileName("../TestFiles/parse_json_object_2.js"), fileContent);

    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSSourceFile sourceFile(lookup, fileContent, wxFileName("../TestFiles/parse_json_object_2.js"));
    JSObjectParser objectParser(sourceFile, lookup);
    CHECK_BOOL(objectParser.Parse(NULL));

    JSObject::Ptr_t obj = objectParser.GetResult();
    CHECK_SIZE(obj->GetProperties().count("open"), 1);
    CHECK_SIZE(obj->GetProperties().count("close"), 1);

    // obj->Print(0);
    return true;
}

TEST_FUNC(parse_func_with_multiple_return_values)
{
    wxString fileContent;
    FileUtils::ReadFileContent(wxFileName("../TestFiles/parse_func_with_multiple_return_values.js"), fileContent);

    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSExpressionParser expr(fileContent);
    JSObject::Ptr_t resolved = expr.Resolve(lookup, "../TestFiles/parse_func_with_multiple_return_values.js");
    CHECK_BOOL(resolved);

#if 0
    resolved->Print(0);
#endif

    CHECK_STRING(resolved->GetType(), "Boolean|Number|String");
    return true;
}

TEST_FUNC(parse_var_assigment_of_function)
{
    wxString fileContent;
    FileUtils::ReadFileContent(wxFileName("../TestFiles/parse_var_assigment_of_function.js"), fileContent);

    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSExpressionParser expr(fileContent);
    JSObject::Ptr_t resolved = expr.Resolve(lookup, "../TestFiles/parse_var_assigment_of_function.js");
    CHECK_BOOL(resolved);
    CHECK_STRING(resolved->GetType(), "Boolean|String");
    return true;
}

TEST_FUNC(parse_this_expression)
{
    wxString fileContent;
    FileUtils::ReadFileContent(wxFileName("../TestFiles/parse_this_expression.js"), fileContent);

    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSExpressionParser expr(fileContent);
    JSObject::Ptr_t resolved = expr.Resolve(lookup, "../TestFiles/parse_this_expression.js");
    CHECK_BOOL(resolved);
    CHECK_STRING(resolved->GetType(), "String");
    return true;
}

TEST_FUNC(parse_this_of_prototype)
{
    wxString fileContent;
    FileUtils::ReadFileContent(wxFileName("../TestFiles/parse_this_of_prototype.js"), fileContent);

    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    JSExpressionParser expr(fileContent);
    JSObject::Ptr_t resolved = expr.Resolve(lookup, "../TestFiles/parse_this_of_prototype.js");
    CHECK_BOOL(resolved);
    return true;
}

void ParseJSONFile(JSLookUpTable::Ptr_t lookup);
int main(int argc, char** argv)
{
    ::wxInitialize(argc, argv);
    
#if 1  
    Tester::Instance()->RunTests();
#else
    JSLookUpTable::Ptr_t lookup(new JSLookUpTable());
    ParseJSONFile(lookup);
#endif    
    ::wxUninitialize();
    return 0;
}

bool ProcessSignature(const wxString& type, wxString& sig, wxString& returnValue) {}

wxString PrepareDoc(const wxString& doc, const wxString& url)
{
    wxString d = doc;
    d.Replace("\\n", " ");

    wxArrayString tokens = ::wxStringTokenize(d, " ", wxTOKEN_STRTOK);
    wxString content;
    wxString curline;
    for(size_t i = 0; i < tokens.size(); ++i) {
        curline << tokens.Item(i) << " ";
        if(curline.length() > 80) {
            content << curline << "\n";
            curline.clear();
        }
    }

    if(!curline.IsEmpty()) {
        content << curline << "\n";
    }
    content << "@link " << url;
    return content;
}

JSObject::Ptr_t ProcessEntry(JSONElement entry, JSLookUpTable::Ptr_t lookup)
{
    wxString name = entry.getName();
    if(name.IsEmpty()) return NULL;

    wxString type = entry.namedObject("!type").toString();
    type.Trim().Trim(false);
    if(type.IsEmpty()) return NULL;

    wxString url = entry.namedObject("!url").toString();
    wxString doc = entry.namedObject("!doc").toString();
    doc = PrepareDoc(doc, url);
    
    JSObject::Ptr_t prop = lookup->NewObject();
    prop->SetName(name);
    prop->SetComment(doc);

    if(type.StartsWith("fn(")) {
        doc = PrepareDoc(doc, url);
        prop->SetFunction();
        type = type.Mid(2);
        wxString sig, returnValue;
        if(!ProcessSignature(type, sig, returnValue)) return NULL;
        
    } else if(type == "number") {
        prop->SetType("Number");
    
    } else if(type == "string") {
        prop->SetType("String");
    
    } else if(type == "bool") {
        prop->SetType("Boolean");

    }
    
    
    {
        JSONElement child = entry.firstChild();
        while(child.isOk()) {
            if(!child.getName().StartsWith("!")) {
                // an item with properties -> a class
                prop->SetClass();
                break;
            }
            child = entry.nextChild();
        }
    }
    
    {
        if(prop->IsClass()) {
            JSONElement child = entry.firstChild();
            while(child.isOk()) {
                if(!child.getName().StartsWith("!") && child.getName() != "prototype") {
                    // an item with properties -> a class
                    JSObject::Ptr_t o = ProcessEntry(child, lookup);
                    if(o) {
                        prop->AddProperty(o);
                    }
                }
                child = entry.nextChild();
            }
        }
    }
    // register this class / function / constant
    lookup->AddObject(prop);
    return prop;
}

void ProcessJSONTopLevel(JSONElement element, JSLookUpTable::Ptr_t lookup)
{
    if(!element.isOk()) return;

    // Scan the global functions / classes
    JSONElement child = element.firstChild();
    while(child.isOk()) {
        const wxString& name = child.getName();
        if(name.StartsWith("!")) {
            // skip any property that starts with "!"
            child = element.nextChild();
            continue;
        }
        
        ProcessEntry(child, lookup);
        child = element.nextChild();
    }
}

void ParseJSONFile(JSLookUpTable::Ptr_t lookup)
{
    JSONRoot root(wxFileName("/home/eran/devl/codelite/WebTools/json/ecma5.json"));
    JSONElement element = root.toElement();
    if(!element.hasNamedObject("!name")) return;
    wxString libraryName = element.namedObject("!name").toString();
    ProcessJSONTopLevel(element, lookup);
}